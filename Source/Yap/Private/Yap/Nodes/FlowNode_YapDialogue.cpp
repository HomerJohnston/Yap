// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
#include "GameplayTagsModule.h"
#include "Yap/YapBit.h"
#include "Yap/YapCondition.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"

#define LOCTEXT_NAMESPACE "Yap"

FName UFlowNode_YapDialogue::OutputPinName = FName("Out");
FName UFlowNode_YapDialogue::BypassPinName = FName("Bypass");

// ------------------------------------------------------------------------------------------------

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Yap");

	NodeStyle = EFlowNodeStyle::Custom;
#endif

	DialogueNodeType = EYapDialogueNodeType::Talk;
	
	NodeActivationLimit = 0;
	
	TalkSequencing = EYapDialogueTalkSequencing::RunAll;

	// Always have at least one fragment.
	Fragments.Add(FYapFragment());

	// The node will only have certain context-outputs which depend on the node type. 
	OutputPins = {};
	
#if WITH_EDITOR
	// TODO use the subsystem to manage crap like this
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTag), EYap_TagFilter::Prompts);
	
	if (IsTemplate())
	{
		UGameplayTagsManager::Get().OnFilterGameplayTagChildren.AddUObject(this, &ThisClass::OnFilterGameplayTagChildren);
	}
#endif
}

// ------------------------------------------------------------------------------------------------

int16 UFlowNode_YapDialogue::FindFragmentIndex(const FGuid& InFragmentGuid) const
{
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		if (Fragments[i].GetGuid() == InFragmentGuid)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

// ------------------------------------------------------------------------------------------------

FYapFragment* UFlowNode_YapDialogue::FindTaggedFragment(const FGameplayTag& Tag)
{
	for (FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag() == Tag)
		{
			return &Fragment;
		}
	}

	return nullptr;
}

void UFlowNode_YapDialogue::OnSkipAction(UObject* Instigator, FYapSpeechHandle Handle)
{
	if (!CanSkip(Handle))
	{
		return;
	}

	int32 SkippedRunningFragmentIndex = RunningFragmentIndex;
	
	for (uint8 i = 0; i <= SkippedRunningFragmentIndex; ++i)
	{
		FYapFragment& Fragment = Fragments[i];
		
		if (Fragment.SpeechTimerHandle.IsValid())
		{
			OnSpeechComplete(i);
		}

		if (Fragment.ProgressionTimerHandle.IsValid())
		{
			OnProgressionComplete(i);
		}

		if (Fragment.GetIsAwaitingManualAdvance())
		{
			AdvanceFromFragment(i);
		}
	}
	
	return;
}

bool UFlowNode_YapDialogue::CanSkip(FYapSpeechHandle Handle) const
{
	if (!RunningSpeechHandle.IsValid())
	{
		return false;
	}

	if (RunningSpeechHandle != Handle)
	{
		return false;
	}

	const FYapFragment& Fragment = Fragments[RunningFragmentIndex];
	
	// The fragment is finished running, and this feature is only being used for manual advance
	if (Fragment.GetIsAwaitingManualAdvance())
	{
		return true;
	}

	// Is skipping allowed or not?
	bool bPreventSkippingTimers = !Fragment.GetSkippable(this->GetSkippable());
	
	if (bPreventSkippingTimers)
	{
		return false;
	}
		
	/*
	bool bWillAutoAdvance = GetFragmentAutoAdvance(RunningFragmentIndex);

	// Will this fragment auto-advance, and if so, are we already nearly finished playing it? If so, ignore skip requests.
	if (bWillAutoAdvance)
	{
		float MinTimeRemainingToAllowSkip = UYapProjectSettings::GetMinimumTimeRemainingToAllowSkip();

		if (MinTimeRemainingToAllowSkip > 0)
		{
			float SpeechTimeRemaining = (FragmentTimerHandle.IsValid()) ? GetWorld()->GetTimerManager().GetTimerRemaining(FragmentTimerHandle) : 0.0f;
			float PaddingTimeRemaining = (PaddingTimerHandle.IsValid()) ? GetWorld()->GetTimerManager().GetTimerRemaining(PaddingTimerHandle) : 0.0f;

			if ((SpeechTimeRemaining + PaddingTimeRemaining) < MinTimeRemainingToAllowSkip)
			{
				return false;
			}
		}
	}

	// Did we only just start playing this fragment? If so, ignore skip requests. This might help users from accidentally skipping a new piece of dialogue.
	float MinTimeElapsedToAllowSkip = UYapProjectSettings::GetMinimumTimeElapsedToAllowSkip();

	if (MinTimeElapsedToAllowSkip > 0)
	{
		float SpeechTimeElapsed = GetWorld()->GetTimeSeconds() - Fragment.GetStartTime();

		if ((SpeechTimeElapsed) < MinTimeElapsedToAllowSkip)
		{
			return false;
		}
	}
	*/
	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::InitializeInstance()
{
	Super::InitializeInstance();

	for (FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag().IsValid())
		{
			UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
			Subsystem->RegisterTaggedFragment(Fragment.GetFragmentTag(), this);
		}
	}
	
	TriggerPreload();
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	if (!CanEnterNode())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;	
	}

	bool bStartedSuccessfully = IsPlayerPrompt() ? TryBroadcastPrompts() : TryStartFragments();

	if (bStartedSuccessfully)
	{
		++NodeActivationCount;
	}
	else
	{
		TriggerOutput(BypassPinName, true);
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnPassThrough_Implementation()
{
	if (IsPlayerPrompt())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::PassThrough);
	}
	else
	{
		TriggerOutput("Out", true, EFlowPinActivationType::PassThrough);
	}
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CanEnterNode()
{
	return CheckConditions() && CheckActivationLimits();
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CheckConditions()
{
	for (UYapCondition* Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}

		if (!Condition->EvaluateCondition_Internal())
		{
			return false;
		}
	}
	
	return true;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::UsesTitleText() const
{
	if (IsPlayerPrompt())
	{
		return !UYapProjectSettings::GetTypeGroup(TypeGroup).GetHideTitleTextOnPromptNodes();
	}

	return UYapProjectSettings::GetTypeGroup(TypeGroup).GetShowTitleTextOnTalkNodes();
}

const FYapFragment& UFlowNode_YapDialogue::GetFragment(uint8 FragmentIndex) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	return Fragments[FragmentIndex];
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetSkippable() const
{
	return Skippable.Get(!UYapProjectSettings::GetTypeGroup(TypeGroup).GetForcedDialogueDuration());
}

bool UFlowNode_YapDialogue::GetNodeAutoAdvance() const
{
	return AutoAdvance.Get(!UYapProjectSettings::GetTypeGroup(TypeGroup).GetManualAdvanceOnly());
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetFragmentAutoAdvance(uint8 FragmentIndex) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	const FYapFragment& Fragment = Fragments[FragmentIndex]; 
	
#if WITH_EDITOR
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
#endif
		// Only check this during play, never at editor time
		
		// If this dialogue is occurring outside of a conversation it must auto advance
		/*
		if (!UYapSubsystem::IsAssetInConversation(GetFlowAsset()))
		{
			return true;
		}
		*/
		
		// Fragments that have no speech time must always be manual-advance
		if (Fragment.GetTimeMode(TypeGroup) == EYapTimeMode::None)
		{
			return false;
		}
#if WITH_EDITOR
	}
#endif	

	// Use fragment override
	if (Fragment.GetAutoAdvanceSetting().IsSet())
	{
		return Fragment.GetAutoAdvanceSetting().GetValue();
	}

	// Use node override
	if (AutoAdvance.IsSet())
	{
		return AutoAdvance.GetValue();
	}

	bool bAutoAdvance = GetNodeAutoAdvance();

	// Check if this fragment is going to progress into a prompt node...
	if (DialogueNodeType == EYapDialogueNodeType::Talk && !bAutoAdvance && FragmentIndex == Fragments.Num() - 1 && UYapProjectSettings::GetTypeGroup(TypeGroup).GetAutoAdvanceToPromptNodes())
	{
		if (IsOutputConnectedToPromptNode())
		{
			return true;
		}
	}
	
	return bAutoAdvance; 
}

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
TOptional<bool> UFlowNode_YapDialogue::GetSkippableSetting() const
{
	return Skippable;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::InvalidateFragmentTags()
{
	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		Fragment.InvalidateFragmentTag(this);
	}
}
#endif

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::TryBroadcastPrompts()
{
	PromptIndices.Empty(Fragments.Num());
	
	UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
	
 	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		FYapFragment& Fragment = Fragments[i];

 		if (!Fragment.CheckConditions())
 		{
 			continue;
 		}
 		
		if (Fragment.IsActivationLimitMet())
		{
			continue;
		}

 		const FYapBit& Bit = Fragment.GetBit();

 		const FYapConversation& Conversation = Subsystem->GetConversation(GetFlowAsset()); 
 		
 		FYapData_PlayerPromptCreated Data;
 		Data.Conversation = Conversation.GetConversationName();
 		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
 		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
 		Data.MoodTag = Fragment.GetMoodTag();
 		Data.DialogueText = Bit.GetDialogueText();
 		Data.TitleText = Bit.GetTitleText();
 		
		FYapPromptHandle PromptHandle = Subsystem->BroadcastPrompt(Data, TypeGroup);

 		PromptIndices.Add(PromptHandle, i);
	}
	
	{
		FYapData_PlayerPromptsReady Data;
		Data.Conversation = Subsystem->GetConversation(GetFlowAsset()).GetConversationName();
		Subsystem->OnFinishedBroadcastingPrompts(Data, TypeGroup);
	}
	
	Subsystem->OnPromptChosen.AddDynamic(this, &ThisClass::OnPromptChosen);
	
	// TODO - automatically return if there are no prompts or if there is only one prompt, optional plugin settings 
	/*
	if (PromptFragmentIndices.Num() == 0)
	{
		return false;
	}

	if (PromptFragmentIndices.Num() == 1)
	{
		if (UYapProjectSettings::GetAutoSelectLastPromptSetting())
		{
			LastHandle.RunPrompt(this);
		}
	}
	*/
	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::RunPrompt(uint8 FragmentIndex)
{	
	if (!RunFragment(FragmentIndex))
	{
		RunningFragmentIndex = INDEX_NONE;
		TriggerOutput(BypassPinName, true);
	}
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::TryStartFragments()
{
	bool bStartedSuccessfully = false;
	
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		bStartedSuccessfully = RunFragment(i);

		if (bStartedSuccessfully)
		{
			break;
		}
	}

	return bStartedSuccessfully;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::RunFragment(uint8 FragmentIndex)
{
	if (!Fragments.IsValidIndex(FragmentIndex))
	{
		UE_LOG(LogYap, Error, TEXT("Attempted run invalid fragment index!"));
		return false;
	}

	FYapFragment& Fragment = Fragments[FragmentIndex];

	if (!FragmentCanRun(FragmentIndex))
	{
		Fragment.SetStartTime(-1.0);
		Fragment.SetEndTime(-1.0);
		Fragment.SetEntryState(EYapFragmentEntryStateFlags::Failed);
		return false;
	}

	const FYapBit& Bit = Fragment.GetBit();

	TOptional<float> Time = Fragment.GetSpeechTime(TypeGroup);

	float EffectiveTime;
	
	if (Time.IsSet())
	{
		EffectiveTime = Time.GetValue();
	}
	else
	{
		EffectiveTime = UYapProjectSettings::GetTypeGroup(TypeGroup).GetMinimumSpeakingTime();
	}

	UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();

	FYapData_SpeechBegins Data;
	Data.Conversation = Subsystem->GetConversation(GetFlowAsset()).GetConversationName();
	Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
	Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
	Data.MoodTag = Fragment.GetMoodTag();
	Data.DialogueText = Bit.GetDialogueText();
	Data.TitleText = Bit.GetTitleText();
	Data.SpeechTime = EffectiveTime;
	Data.FragmentTime = Fragment.GetProgressionTime(TypeGroup); 
	Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
	Data.bSkippable = Fragment.GetSkippable(GetSkippable());
	
	RunningSpeechHandle = Subsystem->RunSpeech(Data, TypeGroup);
	Subsystem->OnSpeechSkip.AddDynamic(this, &ThisClass::OnSkipAction);
	
	Fragment.IncrementActivations();

	if (Fragment.UsesStartPin())
	{
		const FFlowPin StartPin = Fragment.GetStartPin();
		TriggerOutput(StartPin.PinName, false);
	}
	
	RunningFragmentIndex = FragmentIndex;
	Fragment.SetStartTime(GetWorld()->GetTimeSeconds());
	Fragment.SetEntryState(EYapFragmentEntryStateFlags::Success);

	if (Fragment.GetSpeechTime(TypeGroup).IsSet())
	{
		float SpeechTime = Fragment.GetSpeechTime(TypeGroup).Get(0.0f);
		
		if (SpeechTime > 0)
		{
			// TODO the subsystem is running a timer too. I need to get rid of this one and slave to the subsystem's progression.
			GetWorld()->GetTimerManager().SetTimer(Fragment.SpeechTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeechComplete, FragmentIndex), SpeechTime, false);
		}
		else
		{
			OnSpeechComplete(FragmentIndex);
		}
	}

	float ProgressionTime = Fragment.GetProgressionTime(TypeGroup);

	if (ProgressionTime > 0)
	{
		// TODO the subsystem is running a timer too. I need to get rid of this one and slave to the subsystem's progression.
		GetWorld()->GetTimerManager().SetTimer(Fragment.ProgressionTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnProgressionComplete, FragmentIndex), ProgressionTime, false);
	}
	else
	{
		OnProgressionComplete(FragmentIndex);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnSpeechComplete(uint8 FragmentIndex)
{
	UE_LOG(LogYap, Verbose, TEXT("OnSpeechComplete"));

	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	GetWorld()->GetTimerManager().ClearTimer(Fragment.SpeechTimerHandle);
	
	if (Fragment.UsesEndPin())
	{
		const FFlowPin EndPin = Fragment.GetEndPin();
		TriggerOutput(EndPin.PinName, false);
	}
	
	Fragment.SetEndTime(GetWorld()->GetTimeSeconds());
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnProgressionComplete(uint8 FragmentIndex)
{
	UE_LOG(LogYap, Verbose, TEXT("OnProgressionComplete"));
	
	uint8 FinishedFragmentIndex = RunningFragmentIndex;
	FYapFragment& Fragment = Fragments[RunningFragmentIndex];

	UYapSubsystem::Get()->OnSpeechSkip.RemoveDynamic(this, &ThisClass::OnSkipAction);
	
	if (Fragment.ProgressionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(Fragment.ProgressionTimerHandle);
	}
	
	if (GetFragmentAutoAdvance(FinishedFragmentIndex))
	{
		RunningFragmentIndex = INDEX_NONE;
		AdvanceFromFragment(FinishedFragmentIndex);
	}
	else
	{
		Fragment.SetAwaitingManualAdvance();
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::AdvanceFromFragment(uint8 FragmentIndex)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];
	Fragment.SetRunState(EYapFragmentRunState::Idle);

	if (IsPlayerPrompt())
	{
		TriggerOutput(Fragment.GetPromptPin().PinName, true);
		RunningFragmentIndex = INDEX_NONE;
	}
	else
	{
		if (TalkSequencing == EYapDialogueTalkSequencing::SelectOne)
		{
			TriggerOutput(OutputPinName, true);
			RunningFragmentIndex = INDEX_NONE;
		}
		else
		{
			for (uint8 NextIndex = FragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
			{
				bool bRanNextFragment = RunFragment(NextIndex);

				if (!bRanNextFragment && TalkSequencing == EYapDialogueTalkSequencing::RunUntilFailure)
				{
					// Whoops, this is the end of the line
					TriggerOutput(OutputPinName, true);
					RunningFragmentIndex = INDEX_NONE;
					return;
				}
				else if (bRanNextFragment)
				{
					// We'll delegate further behavior to the next running fragment
					return;
				}
			}

			// No more fragments to try and run!
			TriggerOutput(OutputPinName, true);
			RunningFragmentIndex = INDEX_NONE;
		}
	}
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::IsBypassPinRequired() const
{
	// If there are any conditions, we will need a bypass node in case all conditions are false
	if (Conditions.Num() > 0 || GetNodeActivationLimit() > 0)
	{
		return true;
	}
	
	// If all of the fragments have conditions, we will need a bypass node in case all fragments are unusable
	for (const FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetConditions().Num() == 0 && Fragment.GetActivationLimit() == 0)
		{
			return false;
		}
	}

	return true;
}

bool UFlowNode_YapDialogue::IsOutputConnectedToPromptNode() const
{
	if (DialogueNodeType != EYapDialogueNodeType::Talk)
	{
		return false;
	}
	
	FConnectedPin OutputConnection = GetConnection(OutputPinName);

	if (OutputConnection.NodeGuid.IsValid())
	{
		UFlowNode* Node = GetFlowAsset()->GetNode(OutputConnection.NodeGuid);

		if (UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Node))
		{
			if (DialogueNode->DialogueNodeType == EYapDialogueNodeType::PlayerPrompt)
			{
				return true;
			}
		}
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::FragmentCanRun(uint8 FragmentIndex)
{
	const FYapFragment& Fragment = GetFragmentByIndex(FragmentIndex);
	
	if (!Fragment.CheckConditions())
	{
		return false;
	}
	
	if (Fragment.IsActivationLimitMet())
	{
		return false;
	}

	return true;
}

// ------------------------------------------------------------------------------------------------

const FYapFragment& UFlowNode_YapDialogue::GetFragmentByIndex(uint8 Index) const
{
	check(Fragments.IsValidIndex(Index));

	return Fragments[Index];
}

void UFlowNode_YapDialogue::OnPromptChosen(UObject* Instigator, FYapPromptHandle Handle)
{
	UYapSubsystem::Get()->OnPromptChosen.RemoveDynamic(this, &ThisClass::OnPromptChosen);
	
	RunPrompt(PromptIndices[Handle]);
}

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FYapFragment& UFlowNode_YapDialogue::GetFragmentByIndexMutable(uint8 Index)
{
	check (Fragments.IsValidIndex(Index))

	return Fragments[Index];
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
TArray<FYapFragment>& UFlowNode_YapDialogue::GetFragmentsMutable()
{
	return Fragments;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::RemoveFragment(int32 Index)
{
	Fragments.RemoveAt(Index);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FText UFlowNode_YapDialogue::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Dialogue");
	}

	return FText::FromString(" ");
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UFlowNode_YapDialogue::SupportsContextPins() const
{
	return true;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UFlowNode_YapDialogue::GetUsesMultipleInputs()
{
	return false;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UFlowNode_YapDialogue::GetUsesMultipleOutputs()
{
	return true;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
EYapDialogueTalkSequencing UFlowNode_YapDialogue::GetMultipleFragmentSequencing() const
{
	return TalkSequencing;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs() const
{
	TArray<FFlowPin> ContextOutputPins = Super::GetContextOutputs();

	if (!IsPlayerPrompt())
	{
		ContextOutputPins.Add(OutputPinName);
	}

	for (uint8 Index = 0; Index < Fragments.Num(); ++Index)
	{
		const FYapFragment& Fragment = Fragments[Index];
		
		if (Fragment.UsesEndPin())
		{
			ContextOutputPins.Add(Fragment.GetEndPin());
		}
		
		if (Fragment.UsesStartPin())
		{
			ContextOutputPins.Add(Fragment.GetStartPin());
		}

		if (IsPlayerPrompt())
		{
			ContextOutputPins.Add(Fragment.GetPromptPin());
		}
	}

	if (IsBypassPinRequired())
	{
		ContextOutputPins.Add(BypassPinName);
	}
	
	return ContextOutputPins;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::SetNodeActivationLimit(int32 NewValue)
{
	bool bBypassRequired = IsBypassPinRequired();
	
	NodeActivationLimit = NewValue;

	if (bBypassRequired != IsBypassPinRequired())
	{
		(void)OnReconstructionRequested.ExecuteIfBound();
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::CycleFragmentSequencingMode()
{
	uint8 AsInt = static_cast<uint8>(TalkSequencing);

	if (++AsInt >= static_cast<uint8>(EYapDialogueTalkSequencing::COUNT))
	{
		AsInt = 0;
	}

	TalkSequencing = static_cast<EYapDialogueTalkSequencing>(AsInt);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::DeleteFragmentByIndex(int16 DeleteIndex)
{
	if (!Fragments.IsValidIndex(DeleteIndex))
	{
		UE_LOG(LogYap, Error, TEXT("Invalid deletion index!"));
	}

	Fragments.RemoveAt(DeleteIndex);

	UpdateFragmentIndices();
	
	(void)OnReconstructionRequested.ExecuteIfBound();
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::UpdateFragmentIndices()
{
	for (int i = 0; i < Fragments.Num(); ++i)
	{
		Fragments[i].SetIndexInDialogue(i);
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::SwapFragments(uint8 IndexA, uint8 IndexB)
{
	Fragments.Swap(IndexA, IndexB);

	UpdateFragmentIndices();

	(void)OnReconstructionRequested.ExecuteIfBound();
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FString UFlowNode_YapDialogue::GetNodeDescription() const
{
	return "";
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const
{
	if (GameplayTagNode == nullptr)
	{
		bArg = false;
		return;
	}

	TSharedPtr<FGameplayTagNode> ParentTagNode = GameplayTagNode->GetParentTagNode();

	if (ParentTagNode == nullptr)
	{
		bArg = false;
		return;
	}
	
	const FGameplayTagContainer& ParentTagContainer = ParentTagNode->GetSingleTagContainer();

	if (ParentTagContainer.HasTagExact(UYapProjectSettings::GetTypeGroup(TypeGroup).GetDialogueTagsParent()))
	{
		bArg = true;
	}

	bArg = false;
}
#endif

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CheckActivationLimits() const
{
	bool bCanRunAnything = true;
	
	// Make sure entire node has not hit limits
	if (GetNodeActivationLimit() > 0 && GetNodeActivationCount() >= GetNodeActivationLimit())
	{
		return false;
	}

	// If even one fragment has not met activation limits, we're OK
	for (int i = 0; i < Fragments.Num(); ++i)
	{
		int32 ActivationLimit = Fragments[i].GetActivationLimit();
		int32 ActivationCount = Fragments[i].GetActivationCount();

		if (ActivationLimit == 0 || ActivationCount < ActivationLimit)
		{
			return true;
		}
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
#if WITH_EDITOR
void UFlowNode_YapDialogue::ToggleNodeType()
{
	uint8 AsInt = static_cast<uint8>(DialogueNodeType);

	if (++AsInt >= static_cast<uint8>(EYapDialogueNodeType::COUNT))
	{
		AsInt = 0;
	}

	DialogueNodeType = static_cast<EYapDialogueNodeType>(AsInt);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::ForceReconstruction()
{
	(void)OnReconstructionRequested.ExecuteIfBound();
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::PostEditImport()
{
	Super::PostEditImport();

	for (FYapFragment& Fragment : Fragments)
	{
		Fragment.ResetGUID();
		Fragment.ResetOptionalPins();
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::PostLoad()
{
	Super::PostLoad();
	
	TriggerPreload();
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::PreloadContent()
{
	UWorld* World = GetWorld();

	EYapLoadContext LoadContext = EYapLoadContext::Async;
	EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified;

#if WITH_EDITOR
	if (!World || (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::GamePreview))
	{
		LoadContext = EYapLoadContext::AsyncEditorOnly;
	}
#endif

	for (FYapFragment& Fragment : Fragments)
	{
		Fragment.PreloadContent(MaturitySetting, LoadContext);
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE