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

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnSkipAction(UObject* Instigator, FYapSpeechHandle Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSkipAction for handle {%s}"), *GetName(), *Handle.ToString())
	
	if (!CanSkip(Handle))
	{
		// The skip event wasn't for this node, ignore it
		// This system is designed under the assumption that there will usually only be a few pieces of dialogue running simultaneously, at most.
		// It's pretty cheap to just send the event to everyone and ignore the skip request if it wasn't for us.
		return;
	}

	FYapFragment& SkippedFragment = Fragments[RunningFragmentIndex.Get(INDEX_NONE)];

#if !UE_BUILD_SHIPPING
	if (SkippedFragment.GetIsAwaitingManualAdvance())
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: Manually advancing..."), *GetName(), RunningFragmentIndex.Get(INDEX_NONE));
	}
	else
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: Skipping running fragment..."), *GetName(), RunningFragmentIndex.Get(INDEX_NONE));
	}
#endif
	
	for (uint8 Index : FragmentsUsingPadding)
	{
		GetWorld()->GetTimerManager().ClearTimer(Fragments[Index].PaddingTimerHandle);
	}

	FragmentsUsingPadding.Empty();
	
	AdvanceFromFragment(RunningFragmentIndex.Get(INDEX_NONE));
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnConversationSkip(UObject* Instigator, FYapConversationHandle Handle)
{
	if (!RunningFragmentIndex.IsSet())
	{		
		UE_LOG(LogYap, Warning, TEXT("OnConversationSkip was called, but running fragment was unset, ignoring"));
		return;
	}

	// TODO make sure this dialogue node is actually in the conversation
	
	// It will be reset during iteration! Cache it
	uint8 ToIndex = RunningFragmentIndex.GetValue();
	
	for (uint8 i = 0; i <= ToIndex; ++i)
	{
		FYapFragment& Fragment = Fragments[i];
	
		FTimerHandle& PaddingTimerHandle = Fragment.PaddingTimerHandle;

		if (PaddingTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(PaddingTimerHandle);
		}

		if (Fragment.GetRunState() != EYapFragmentRunState::Idle)
		{
			FYapSpeechHandle TempHandle(GetWorld(), Fragment.GetGuid());
		
			OnSpeechComplete(Instigator, TempHandle);
		}
	}
	
	RunningFragments.Empty();

	// OnSpeechComplete may have advanced us; if it didn't, do it manually
	if (RunningFragmentIndex == ToIndex)
	{
		AdvanceFromFragment(ToIndex);
	}
}

void UFlowNode_YapDialogue::FinishNode(FName OutputPinToTrigger)
{
	UYapSubsystem::Get(this)->OnConversationSkip.RemoveDynamic(this, &ThisClass::OnConversationSkip);
	UYapSubsystem::Get(this)->OnSpeechSkip.RemoveDynamic(this, &ThisClass::OnSkipAction);
		
	TriggerOutput(OutputPinToTrigger, true, EFlowPinActivationType::Default);
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CanSkip(FYapSpeechHandle Handle) const
{
	if (RunningSpeechHandle != Handle || !Handle.IsValid())
	{
		return false;
	}

	check(RunningFragmentIndex.IsSet());
	
	const FYapFragment& Fragment = Fragments[RunningFragmentIndex.GetValue()];
	
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
	if (CanEnterNode())
	{
		bool bStartedSuccessfully = IsPlayerPrompt() ? TryBroadcastPrompts() : TryStartFragments();

		if (bStartedSuccessfully)
		{
			++NodeActivationCount;

			UYapSubsystem::Get(this)->OnConversationSkip.AddDynamic(this, &ThisClass::OnConversationSkip);
			UYapSubsystem::Get(this)->OnSpeechSkip.AddDynamic(this, &ThisClass::OnSkipAction);
		}
		else
		{
			TriggerOutput(BypassPinName, true, EFlowPinActivationType::Default);
		}
	}
	else
	{
		TriggerOutput(BypassPinName, true, EFlowPinActivationType::Default);
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
			UE_LOG(LogYap, Warning, TEXT("%s: Null condition found!"), *GetName());
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

	UWorld* World = GetWorld();
	
#if WITH_EDITOR
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		// TODO hack, is this safe?
		if (World == nullptr) { World = GEditor->PlayWorld; }
#endif
		// Only check for these during play, never at editor time
		
		// TODO If this dialogue is occurring outside of a conversation it must auto advance
		/*
		if (!UYapSubsystem::IsAssetInConversation(GetFlowAsset()))
		{
			return true;
		}
		*/
		
		// Fragments that have no speech time must always be manual-advance
		if (Fragment.GetTimeMode(World, TypeGroup) == EYapTimeMode::None)
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

	// TODO this needs to parse through reroute nodes somehow :\
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

 		const FYapBit& Bit = Fragment.GetBit(GetWorld());

 		const FYapConversation& Conversation = Subsystem->GetConversation(this, GetFlowAsset()); 
 		
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
		Data.Conversation = Subsystem->GetConversation(this, GetFlowAsset()).GetConversationName();
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
		UE_LOG(LogYap, Error, TEXT("%s [%i]: RunPrompt failed! This should never happen. Execution of this flow will stop."), *GetName(), FragmentIndex);
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
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: RunFragment"), *GetName(), FragmentIndex);

	if (!Fragments.IsValidIndex(FragmentIndex))
	{
		UE_LOG(LogYap, Error, TEXT("%s: Attempted run invalid fragment index [%i]"), *GetName(), FragmentIndex);
		return false;
	}

	FYapFragment& Fragment = Fragments[FragmentIndex];

	if (!FragmentCanRun(FragmentIndex))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: RunFragment failed starting condition checks"), *GetName(), FragmentIndex);
		Fragment.SetStartTime(-1.0);
		Fragment.SetEndTime(-1.0);
		Fragment.SetEntryState(EYapFragmentEntryStateFlags::Failed);
		return false;
	}

	Fragment.SetRunState(EYapFragmentRunState::Running);
	
	const FYapBit& Bit = Fragment.GetBit(GetWorld());

	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: Running... %s"), *GetName(), FragmentIndex, *Bit.GetDialogueText().ToString());

	TOptional<float> Time = Fragment.GetSpeechTime(GetWorld(), TypeGroup);

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
	Data.Conversation = Subsystem->GetConversation(GetWorld(), GetFlowAsset()).GetConversationName();
	Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
	Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
	Data.MoodTag = Fragment.GetMoodTag();
	Data.DialogueText = Bit.GetDialogueText();
	Data.TitleText = Bit.GetTitleText();
	Data.SpeechTime = EffectiveTime;
	//Data.FragmentTime = Fragment.GetProgressionTime(TypeGroup); // Removed; // TODO further consideration needed; See notes in YapDataStructures.h
	Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
	Data.bSkippable = Fragment.GetSkippable(GetSkippable());

	// Make a handle for the pending speech and bind to completion events of it
	RunningSpeechHandle = FYapSpeechHandle(GetWorld(), Fragment.GetGuid());

	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: RunFragment; setting RunningFragmentIndex"), *GetName(), FragmentIndex);
	RunningFragmentIndex = FragmentIndex;

	Fragment.IncrementActivations();
	
	// TODO clean this crap up
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));
	UYapSpeechHandleBFL::BindToOnSpeechComplete(GetWorld(), RunningSpeechHandle, Delegate);
	
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: RunFragment; adding handle to RunningFragments {%s}"), *GetName(), FragmentIndex, *RunningSpeechHandle.ToString());
	RunningFragments.Add(RunningSpeechHandle, FragmentIndex);

	Fragment.SetStartTime(GetWorld()->GetTimeSeconds());
	Fragment.SetEntryState(EYapFragmentEntryStateFlags::Success);
	
	// We must actually run the speech last in this function
	Subsystem->RunSpeech(Data, TypeGroup, RunningSpeechHandle);

	if (Fragment.UsesStartPin())
	{
		const FFlowPin StartPin = Fragment.GetStartPin();
		TriggerOutput(StartPin.PinName, false);
	}
	
	if (Fragment.GetUsesPadding(TypeGroup))
	{
		float PaddingCompletionTime = Fragment.GetProgressionTime(GetWorld(), TypeGroup);
		GetWorld()->GetTimerManager().SetTimer(Fragment.PaddingTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::TryAdvanceFromFragment, FragmentIndex), PaddingCompletionTime, false);
		
		FragmentsUsingPadding.Add(FragmentIndex);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnSpeechComplete(UObject* Instigator, FYapSpeechHandle Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete [%s]"), *GetName(), *Handle.ToString());
	
	// This function only gets called when the subsystem finishes the speech
	uint8* FragmentIndex = RunningFragments.Find(Handle);

	if (!FragmentIndex)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete call ignored; handle {%s} was not running"), *GetName(), *Handle.ToString());
		return;
	}

	RunningFragments.Remove(Handle);
		
	// TODO clean this crap up, is there any other method I can use to achieve this that isn't so dumb looking?
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));
	UYapSpeechHandleBFL::UnbindToOnSpeechComplete(GetWorld(), RunningSpeechHandle, Delegate);
	
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: RunFragment; removing handle from RunningFragments {%s}"), *GetName(), *Handle.ToString());

	FinishFragment(*FragmentIndex);
	
	// TODO this is a hack. I should instead store a set of "padded" fragments during this node's run to check against. It should get reset on input trigger.
	if (RunningFragmentIndex == *FragmentIndex)// && !Fragments[*FragmentIndex].GetUsesPadding(TypeGroup))
	{
		TryAdvanceFromFragment(*FragmentIndex);
	}
}

void UFlowNode_YapDialogue::FinishFragment(uint8 FragmentIndex)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	if (Fragment.UsesEndPin())
	{
		const FFlowPin EndPin = Fragment.GetEndPin();
		TriggerOutput(EndPin.PinName, false);
	}
	
	Fragment.SetEndTime(GetWorld()->GetTimeSeconds());
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::TryAdvanceFromFragment(uint8 FragmentIndex)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: AdvanceFromFragment"), *GetName(), FragmentIndex);

	FYapFragment& Fragment = Fragments[FragmentIndex];

	Fragment.PaddingTimerHandle.Invalidate();

	// TODO When calling AdvanceFromFragment in Skip function, if the game is set to do manual advancement, this won't run. Push this into a separate function I can call or add another route into this.
	if (GetFragmentAutoAdvance(FragmentIndex))
	{
		AdvanceFromFragment(FragmentIndex);
	}
	else
	{
		Fragment.SetAwaitingManualAdvance();
	}
}

void UFlowNode_YapDialogue::AdvanceFromFragment(uint8 FragmentIndex)
{	
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	Fragment.SetRunState(EYapFragmentRunState::Idle);
	
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: AdvanceFromFragment; resetting RunningFragmentIndex"), *GetName(), FragmentIndex)
	RunningFragmentIndex.Reset();
	
	if (IsPlayerPrompt())
	{
		FinishNode(Fragment.GetPromptPinName());
	}
	else
	{
		switch (TalkSequencing)
		{
			case EYapDialogueTalkSequencing::SelectOne:
			{
				FinishNode(OutputPinName);
				
				break;
			}
			case EYapDialogueTalkSequencing::RunAll:
			{
				for (uint8 NextIndex = FragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
				{
					if (RunFragment(NextIndex))
					{
						// The next fragment will continue execution
						return;
					}
				}
			
				FinishNode(OutputPinName);
					
				break;
			}
			case EYapDialogueTalkSequencing::RunUntilFailure:
			{
				for (uint8 NextIndex = FragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
				{
					if (!RunFragment(NextIndex))
					{					
						FinishNode(OutputPinName);
						
						return;
					}
				}
				
				FinishNode(OutputPinName);

				break;
			}
			default:
			{
				checkNoEntry();
			}
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
	UYapSubsystem::Get(GetWorld())->OnPromptChosen.RemoveDynamic(this, &ThisClass::OnPromptChosen);

	uint8* FragmentIndex = PromptIndices.Find(Handle);

	if (FragmentIndex)
	{
		RunPrompt(*FragmentIndex);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("%s: Tried to choose prompt but could not find {%s}"), *GetName(), *Handle.ToString())
	}
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
		UE_LOG(LogYap, Error, TEXT("%s: Invalid deletion index [%i]!"), *GetName(), DeleteIndex);
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
		Fragment.PreloadContent(GetWorld(), MaturitySetting, LoadContext);
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE