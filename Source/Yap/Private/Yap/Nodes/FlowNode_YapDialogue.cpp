// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
#include "GameplayTagsModule.h"
#include "Nodes/Route/FlowNode_Reroute.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/YapBit.h"
#include "Yap/YapCondition.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSquirrelNoise.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	//UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTag), EYap_TagFilter::Prompts);
	
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
	// TODO replace with new logic using FNames
	/*
	for (FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag() == Tag)
		{
			return &Fragment;
		}
	}
*/
	return nullptr;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnCancel(UObject* Instigator, FYapSpeechHandle Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnCancelAction for handle {%s}"), *GetName(), *Handle.ToString())

	UE_LOG(LogYap, Error, TEXT("%s: SPEECH CANCELLING IS NOT IMPLEMENTED YET {%s}"), *GetName(), *Handle.ToString())

	// TODO

	/*
	if (!CanSkip(Handle))
	{
		// The skip event wasn't for this node, ignore it
		// This system is designed under the assumption that there will usually only be a few pieces of dialogue running simultaneously, at most.
		// It's pretty cheap to just send the event to everyone and ignore the skip request if it wasn't for us.
		return;
	}

	FYapFragment& SkippedFragment = Fragments[FocusedFragmentIndex.Get(INDEX_NONE)];

#if !UE_BUILD_SHIPPING
	if (SkippedFragment.GetIsAwaitingManualAdvance())
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: Manually advancing..."), *GetName(), FocusedFragmentIndex.Get(INDEX_NONE));
	}
	else
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: Skipping running fragment..."), *GetName(), FocusedFragmentIndex.Get(INDEX_NONE));
	}
#endif
	
	for (uint8 i = 0; i <= FocusedFragmentIndex.Get(0); ++i)
	{
		GetWorld()->GetTimerManager().ClearTimer(Fragments[i].PaddingTimerHandle);
	}

	FragmentsInPadding.Empty();
	
	AdvanceFromFragment(Handle, FocusedFragmentIndex.Get(INDEX_NONE));
	*/
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnAdvanceConversation(UObject* Instigator, FYapConversationHandle Handle)
{
	// TODO make sure this dialogue node is actually in this conversation, URGENT
	if (!FocusedFragmentIndex.IsSet() || !FocusedSpeechHandle.IsValid())
	{
		UE_LOG(LogYap, Warning, TEXT("%s: OnAdvanceConversation called while focused fragment wasn't set; ignoring request"), *GetName());
		return;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnAdvanceConversation [CH %s]"), *GetName(), *Handle.ToString());

	FragmentsInPadding.Empty();

	auto RunningFragmentsCopy = RunningFragments;
	
	for (auto& [SpeechHandle, Index] : RunningFragmentsCopy)
	{
		FYapFragment& Fragment = Fragments[Index];
		FTimerHandle& PaddingTimerHandle = Fragment.PaddingTimerHandle;

		if (PaddingTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(PaddingTimerHandle);
		}

		FinishFragment(SpeechHandle, Index);
	}

	AdvanceFromFragment(FocusedSpeechHandle, FocusedFragmentIndex.GetValue());
}

void UFlowNode_YapDialogue::FinishNode(FName OutputPinToTrigger)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: FinishNode - Unbinding from OnAdvanceConversation"), *GetName());
	
	UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.RemoveDynamic(this, &ThisClass::OnAdvanceConversation);
		
	bNodeActive = false;
	FocusedFragmentIndex.Reset();
	FocusedSpeechHandle.Invalidate();
	
	TriggerOutput(OutputPinToTrigger, true, EFlowPinActivationType::Default);
}

void UFlowNode_YapDialogue::SetActive()
{
	return;

#if 0
	// TODO is there another way I can do this. This is ugly looking.
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));
	UYapSpeechHandleBFL::BindToOnSpeechComplete(GetWorld(), FocusedSpeechHandle, Delegate);
#endif
}

void UFlowNode_YapDialogue::SetInactive()
{
	return;

#if 0
	// Unbind this node from speech complete events
	// TODO clean this crap up, is there any other method I can use to achieve this that isn't so dumb looking?
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));
	UYapSpeechHandleBFL::UnbindToOnSpeechComplete(GetWorld(), FocusedSpeechHandle, Delegate);
#endif
}

TOptional<float> UFlowNode_YapDialogue::GetSpeechTime(uint8 FragmentIndex) const
{
	if (!GetWorld())
	{
		return NullOpt;
	}
	
	const FYapFragment& Fragment = GetFragment(FragmentIndex);

	return Fragment.GetSpeechTime(GetWorld(), GetNodeConfig());
}

#if WITH_EDITOR
TOptional<float> UFlowNode_YapDialogue::GetSpeechTime(uint8 FragmentIndex, EYapMaturitySetting Maturity, EYapLoadContext LoadContext) const
{	
	const FYapFragment& Fragment = GetFragment(FragmentIndex);

	return Fragment.GetSpeechTime(GetWorld(), Maturity, LoadContext, GetNodeConfig());
}
#endif

float UFlowNode_YapDialogue::GetPadding(uint8 FragmentIndex) const
{
	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		return 0.0f;
	}
	
	const FYapFragment& Fragment = GetFragment(FragmentIndex);

	float Value = Fragment.GetPaddingValue(GetWorld(), GetNodeConfig());

	return Value;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CanSkip(FYapSpeechHandle Handle) const
{
	if (FocusedSpeechHandle != Handle || !Handle.IsValid())
	{
		return false;
	}

	check(FocusedFragmentIndex.IsSet());
	
	const FYapFragment& Fragment = Fragments[FocusedFragmentIndex.GetValue()];
	
	// The fragment is finished running, and this feature is only being used for manual advance
	if (Fragment.IsAwaitingManualAdvance())
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
		if (Fragment.GetFragmentID().IsValid())
		{
			// TODO
			UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
			//Subsystem->RegisterTaggedFragment(Fragment.GetFragmentTag(), this);
		}
	}
	
	TriggerPreload();
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	if (CanEnterNode())
	{
		bNodeActive = true;
		FocusedFragmentIndex.Reset();
		FocusedSpeechHandle.Invalidate();
		
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: Binding OnAdvanceConversation and OnCancel"), *GetName());
		//UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.AddDynamic(this, &ThisClass::OnAdvanceConversation);
		//UYapSubsystem::Get(this)->OnCancelDelegate.AddDynamic(this, &ThisClass::OnCancel);
		
		bool bStartedSuccessfully = IsPlayerPrompt() ? TryBroadcastPrompts() : TryStartFragments();

		if (bStartedSuccessfully)
		{
			++NodeActivationCount;
		}
		else
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s: Unbinding OnAdvanceConversation and OnCancel"), *GetName());
			//UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.RemoveDynamic(this, &ThisClass::OnAdvanceConversation);
			//UYapSubsystem::Get(this)->OnCancelDelegate.RemoveDynamic(this, &ThisClass::OnCancel);
			
			TriggerOutput(BypassPinName, true, EFlowPinActivationType::Default);
		}
	}
	else
	{
#if !UE_BUILD_SHIPPING
		if (!IsBypassPinRequired())
		{
			UE_LOG(LogYap, Error, TEXT("Failed to enter dialogue node and bypass pin is not enabled - execution will halt! This should not happen!"));
			return;
		}
#endif
		TriggerOutput(BypassPinName, true, EFlowPinActivationType::Default);
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnPassThrough_Implementation()
{
	if (IsPlayerPrompt())
	{
		TriggerOutput(BypassPinName, true, EFlowPinActivationType::PassThrough);
	}
	else
	{
		TriggerOutput(OutputPinName, true, EFlowPinActivationType::PassThrough);
	}
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::CanEnterNode()
{
	if (bNodeActive)
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to enter dialogue node [%s], but dialogue node is already active! Yap does not currently support running the same node multiple times simultaneously."), *GetName());
		return false;
	}
	
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

bool UFlowNode_YapDialogue::HasValidConfig() const
{
	if (IsValid(Config))
	{
		return true;
	}

	return !UYapProjectSettings::GetDefaultNodeConfig().IsNull();
}

// ------------------------------------------------------------------------------------------------

const UYapNodeConfig& UFlowNode_YapDialogue::GetNodeConfig() const
{
	if (IsValid(Config))
	{
		return *Config;
	}

	const TSoftObjectPtr<UYapNodeConfig>& ConfigAsset = UYapProjectSettings::GetDefaultNodeConfig();
	
	if (ConfigAsset.IsNull())
	{
		return *GetDefault<UYapNodeConfig>();
	}

	// TODO I should probably pin this somehow or make sure it's always loaded by Yap
	return *ConfigAsset.LoadSynchronous();
}

const FYapFragment& UFlowNode_YapDialogue::GetFragment(uint8 FragmentIndex) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	return Fragments[FragmentIndex];
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetSkippable() const
{
	return Skippable.Get(!GetNodeConfig().GetForcedDialogueDuration());
}

bool UFlowNode_YapDialogue::GetNodeAutoAdvance() const
{
	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		return true;
	}
	
	if (IsPlayerPrompt() && GetNodeConfig().GetPromptAdvancesImmediately())
	{
		return true;
	}
	
	return AutoAdvance.Get(!GetNodeConfig().GetManualAdvanceOnly());
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetFragmentAutoAdvance(uint8 FragmentIndex) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	const FYapFragment& Fragment = Fragments[FragmentIndex]; 

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

	return GetNodeAutoAdvance();
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

	const FYapConversation& Conversation = Subsystem->GetConversationByOwner(this, GetFlowAsset()); 

	FYapPromptHandle LastHandle;
	
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
 		const UYapNodeConfig& ActiveConfig = GetNodeConfig();
 		
 		FYapData_PlayerPromptCreated Data;
 		Data.Conversation = Conversation.GetHandle();

 		if (ActiveConfig.GetUsesDirectedAt())
 		{
 			Data.DirectedAt = Fragment.GetDirectedAt(GetWorld(), EYapLoadContext::Sync);
 		}

 		if (ActiveConfig.GetUsesSpeaker())
 		{
 			Data.Speaker = Fragment.GetSpeakerCharacter(GetWorld(), EYapLoadContext::Sync);
	 		Data.SpeakerName = Fragment.GetSpeakerTag().GetTagName();	
 		}

 		if (ActiveConfig.GetUsesMoodTags())
 		{
 			Data.MoodTag = Fragment.GetMoodTag();
 		}
 		
 		Data.DialogueText = Bit.GetDialogueText();

 		if (ActiveConfig.GetUsesTitleText(GetNodeType()))
 		{
 			Data.TitleText = Bit.GetTitleText();
 		}
 		
		LastHandle = Subsystem->BroadcastPrompt(Data, this->GetClass());

 		PromptIndices.Add(LastHandle, i);
	}

	if (PromptIndices.Num() == 0)
	{
		return false;
	}
	
	Subsystem->OnPromptChosen.AddDynamic(this, &ThisClass::OnPromptChosen);
	
	if (PromptIndices.Num() == 1 && GetNodeConfig().Prompts.bAutoSelectLastPrompt)
	{
		LastHandle.RunPrompt(this);
	}
	else
	{
		FYapData_PlayerPromptsReady Data;
		Data.Conversation = Conversation.GetHandle();
		Subsystem->OnFinishedBroadcastingPrompts(Data, this->GetClass());
	}
	
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

	if (GetMultipleFragmentSequencing() == EYapDialogueTalkSequencing::SelectRandom)
	{
		TArray<uint8> ValidFragments;

		ValidFragments.Reserve(Fragments.Num());

		for (uint8 i = 0; i < Fragments.Num(); ++i)
		{
			if (Fragments[i].CanRun())
			{
				ValidFragments.Add(i);
			}
		}

		if (ValidFragments.Num() > 0)
		{
			if (ValidFragments.Num() == 1)
			{
				bStartedSuccessfully = RunFragment(ValidFragments[0]);
			}
			else if (UYapSubsystem* Subsystem = UYapSubsystem::Get(this))
			{
				double Val = Subsystem->GetNoiseGenerator().NextReal();
				double Val2 = Subsystem->GetNoiseGenerator().NextReal();
				double Val3 = Subsystem->GetNoiseGenerator().NextReal();
				double Val4 = Subsystem->GetNoiseGenerator().NextReal();

				double IntervalVal = Val * (double)ValidFragments.Num();

				uint8 Final = (uint8)FMath::FloorToInt(IntervalVal);
				
				bStartedSuccessfully = RunFragment(ValidFragments[Final]);
			}
		}
	}
	else
	{
		for (uint8 i = 0; i < Fragments.Num(); ++i)
		{
			bStartedSuccessfully = RunFragment(i);

			if (bStartedSuccessfully)
			{
				break;
			}
		}	
	}
	
	return bStartedSuccessfully;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::RunFragment(uint8 FragmentIndex)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: RunFragment START -------------------------------"), *GetName(), FragmentIndex);

	if (!Fragments.IsValidIndex(FragmentIndex))
	{
		UE_LOG(LogYap, Error, TEXT("FAILED - invalid fragment index [%i]"), FragmentIndex);
		return false;
	}

	FYapFragment& Fragment = Fragments[FragmentIndex];

	if (!FragmentCanRun(FragmentIndex))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("FAILED - FragmentCanRun returned false"));
		Fragment.SetStartTime(-1.0);
		Fragment.SetEndTime(-1.0);
		Fragment.SetEntryState(EYapFragmentEntryStateFlags::Failed);
		return false;
	}

	Fragment.SetRunState(EYapFragmentRunState::Running);
	Fragment.ClearAwaitingManualAdvance();
	
	const FYapBit& Bit = Fragment.GetBit(GetWorld());
	const UYapNodeConfig& ActiveConfig = GetNodeConfig();

	TOptional<float> Time = Fragment.GetSpeechTime(GetWorld(), ActiveConfig);

	float EffectiveTime = 0.0f;
	
	if (Time.IsSet())
	{
		EffectiveTime = FMath::Max(Time.GetValue(), ActiveConfig.GetMinimumSpeakingTime());
	}

	UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
	
	FYapData_SpeechBegins Data;
	Data.Conversation = Subsystem->GetConversationByOwner(GetWorld(), GetFlowAsset()).GetConversationName();

	if (ActiveConfig.GetUsesDirectedAt())
	{
		Data.DirectedAt = Fragment.GetDirectedAt(GetWorld(), EYapLoadContext::Sync);
	}

	if (ActiveConfig.GetUsesSpeaker())
	{
		Data.Speaker = Fragment.GetSpeakerCharacter(GetWorld(), EYapLoadContext::Sync);
		Data.SpeakerID = Fragment.GetSpeakerTag().GetTagName();
	}

	if (ActiveConfig.GetUsesMoodTags())
	{
		Data.MoodTag = Fragment.GetMoodTag();
	}
	
	Data.DialogueText = Bit.GetDialogueText();
	Data.SpeechTime = EffectiveTime;

	if (ActiveConfig.GetUsesAudioAsset())
	{
		Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();		
	}
	
	Data.bSkippable = Fragment.GetSkippable(GetSkippable());

	if (!ActiveConfig.GetUsesTitleText(GetNodeType()))
	{
		Data.TitleText = Bit.GetTitleText();
	}

#if !UE_BUILD_SHIPPING
	const UObject* Speaker = Data.Speaker.GetObject();
	
	if (IsValid(Speaker))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: [%s] %s"), *GetName(), FragmentIndex, *IYapCharacterInterface::GetName(Speaker).ToString(), *Bit.GetDialogueText().ToString());		
	}
	else
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: [No Speaker] %s"), *GetName(), FragmentIndex, *Bit.GetDialogueText().ToString());		
	}
#endif
	
	// Make a handle for the pending speech and bind to completion events of it
	FocusedSpeechHandle = Subsystem->GetNewSpeechHandle(Fragment.GetGuid());
	FocusedFragmentIndex = FragmentIndex;

	BindToSubsystemSpeechCompleteEvent(FocusedSpeechHandle);

	RunningFragments.Add(FocusedSpeechHandle, FragmentIndex);
	SpeakingFragments.Add(FocusedSpeechHandle);

	Fragment.SetStartTime(GetWorld()->GetTimeSeconds());
	Fragment.SetEntryState(EYapFragmentEntryStateFlags::Success);
	
	Fragment.IncrementActivations();
	
	Subsystem->RunSpeech(Data, GetClass(), FocusedSpeechHandle);

	if (!Time.IsSet() || GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		OnPaddingComplete(FocusedSpeechHandle);
	}
	else if (Fragment.GetUsesPadding(GetWorld(), ActiveConfig))
	{
		float PaddingCompletionTime = Fragment.GetProgressionTime(GetWorld(), ActiveConfig);

		if (PaddingCompletionTime > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(Fragment.PaddingTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingComplete, FocusedSpeechHandle), PaddingCompletionTime, false);
			FragmentsInPadding.Add(FocusedSpeechHandle);	
		}
		else
		{
			OnPaddingComplete(FocusedSpeechHandle);
		}
	}
	
	TriggerSpeechStartPin(FragmentIndex);
	
	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::BindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: Binding to Speech Completed Events {%s}"), *GetName(), *Handle.ToString());
	
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));

	UYapSubsystem::BindToSpeechFinish(this, Handle, Delegate);
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::UnbindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: Unbinding from Speech Completed Events {%s}"), *GetName(), *Handle.ToString());
	
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_TwoParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle));

	UYapSubsystem::UnbindToSpeechFinish(this, Handle, Delegate);
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnSpeechComplete(UObject* Instigator, FYapSpeechHandle Handle)
{
	uint8* FragmentIndex = RunningFragments.Find(Handle);
	
	if (!FragmentIndex)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete; ignored; handle {%s} was not running"), *GetName(), *Handle.ToString());
		return;
	}
	
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: OnSpeechComplete {%s}"), *GetName(), *FragmentIndex, *Handle.ToString());

	SpeakingFragments.Remove(Handle);

	UnbindToSubsystemSpeechCompleteEvent(Handle);
	
	TriggerSpeechEndPin(*FragmentIndex);
	
	// No positive padding - this fragment is done
	if (!FragmentsInPadding.Contains(Handle))
	{
		FinishFragment(Handle, *FragmentIndex);
		TryAdvanceFromFragment(Handle, *FragmentIndex);
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnPaddingComplete(FYapSpeechHandle Handle)
{
	uint8* FragmentIndex = RunningFragments.Find(Handle);
	
	if (!FragmentIndex)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnPaddingComplete call ignored; handle {%s} was not running"), *GetName(), *Handle.ToString());
		return;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: OnPaddingComplete {%s}"), *GetName(), *FragmentIndex, *Handle.ToString());

	FragmentsInPadding.Remove(Handle);
	
	if (!SpeakingFragments.Contains(Handle))
	{
		// Positive padding - we've already finished speaking, this fragment is done
		FinishFragment(Handle, *FragmentIndex);
		TryAdvanceFromFragment(Handle, *FragmentIndex);
	}
	else
	{
		// Negative padding - we MUST advance now, but the fragment isn't done
		AdvanceFromFragment(Handle, *FragmentIndex);
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::FinishFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: FinishFragment {%s}"), *GetName(), FragmentIndex, *Handle.ToString());
		
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	Fragment.SetEndTime(GetWorld()->GetTimeSeconds());

	RunningFragments.Remove(Handle);
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::TryAdvanceFromFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex)
{
	if (FocusedFragmentIndex != FragmentIndex)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment failed; current focused fragment is: %s"), *GetName(), FragmentIndex, FocusedFragmentIndex.IsSet() ? *FString::FromInt(FocusedFragmentIndex.GetValue()) : TEXT("UNSET") );
		return;
	}
	
	FYapFragment& Fragment = Fragments[FragmentIndex];

	
	// TODO When calling AdvanceFromFragment in Skip function, if the game is set to do manual advancement, this won't run. Push this into a separate function I can call or add another route into this.
	if (GetFragmentAutoAdvance(FragmentIndex))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment passed - GetFragmentAutoAdvance true)"), *GetName(), FragmentIndex);

		AdvanceFromFragment(Handle, FragmentIndex);
	}
	else
	{
		if (GetNodeConfig().GetIgnoreManualAdvanceOutsideConversations() && !UYapSubsystem::IsSpeechInConversation(this, Handle))
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment passed - GetFragmentAutoAdvance true)"), *GetName(), FragmentIndex);
			AdvanceFromFragment(Handle, FragmentIndex);
		}
		else
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment failed - fragment awaiting manual advance"), *GetName(), FragmentIndex);
			Fragment.SetAwaitingManualAdvance();
		}
	}
}

void UFlowNode_YapDialogue::AdvanceFromFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: AdvanceFromFragment"), *GetName(), FragmentIndex);
	
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	Fragment.SetRunState(EYapFragmentRunState::Idle);
	
	if (FragmentIndex != FocusedFragmentIndex)
	{
		return;
	}
		
	if (IsPlayerPrompt())
	{
		FinishNode(Fragment.GetPromptPin().PinName);
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
			case EYapDialogueTalkSequencing::SelectRandom:
			{
				/*
				TArray<uint8> ValidFragments;

				ValidFragments.Reserve(Fragments.Num());

				for (uint8 i = 0; Fragments.Num(); ++i)
				{
					if (Fragments[i].CanRun())
					{
						ValidFragments.Add(i);
					}
				}

				UYapSubsystem* Subsystem = UYapSubsystem::Get(this);
				
				if (Subsystem)
				{
					double Val = Subsystem->GetNoiseGenerator().NextReal();

					double IntervalVal = Val / (double)ValidFragments.Num();

					uint8 Final = (uint8)FMath::FloorToInt(IntervalVal);
					
					
				}
				else
				{
					checkNoEntry();
				}
				*/
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

void UFlowNode_YapDialogue::TriggerSpeechStartPin(uint8 FragmentIndex)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	if (Fragment.UsesStartPin())
	{
		const FFlowPin StartPin = Fragment.GetStartPin();
		TriggerOutput(StartPin.PinName, false);
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::TriggerSpeechEndPin(uint8 FragmentIndex)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	if (Fragment.UsesEndPin())
	{
		const FFlowPin EndPin = Fragment.GetEndPin();
		TriggerOutput(EndPin.PinName, false);
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
	
	TArray<const UFlowNode*> NodesToCheck = {this} ;

	while (NodesToCheck.Num() > 0)
	{
		const UFlowNode* Node = NodesToCheck.Pop(EAllowShrinking::No);

		const TArray<FFlowPin>& OutputPinsToCheck = Node->GetOutputPins();

		for (const FFlowPin& Pin : OutputPinsToCheck)
		{
			FConnectedPin OutputConnection = Node->GetConnection(Pin.PinName);

			UFlowNode* ConnectedNode = GetFlowAsset()->GetNode(OutputConnection.NodeGuid);

			if (!IsValid(ConnectedNode))
			{
				continue;
			}
			
			if (UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(ConnectedNode))
			{
				if (DialogueNode->DialogueNodeType == EYapDialogueNodeType::PlayerPrompt)
				{
					return true;
				}
			}

			// TODO use a project setting to make it possible to add more "pass through" types - for example if Flow or a custom project adds a "portal" node in the future
			if (ConnectedNode->IsA(UFlowNode_Reroute::StaticClass()))
			{
				NodesToCheck.Add(ConnectedNode);
			}
		}
	}
	
	/*
	if (OutputConnection.NodeGuid.IsValid())
	{
		UFlowNode* ConnectedNode = GetFlowAsset()->GetNode(OutputConnection.NodeGuid);

		if (UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(ConnectedNode))
		{
			if (DialogueNode->DialogueNodeType == EYapDialogueNodeType::PlayerPrompt)
			{
				return true;
			}
		}
		else if (UFlowNode_Reroute* RerouteNode = Cast<UFlo)
	}
	*/

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

#if WITH_EDITOR
FYapFragment& UFlowNode_YapDialogue::GetFragmentMutableByIndex(uint8 Index)
{
	check(Fragments.IsValidIndex(Index));

	return Fragments[Index];
}
#endif

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
	return Super::GetNodeTitle();
	/*
	if (IsTemplate())
	{
		return FText::FromString("Dialogue");
	}

	return FText::FromString(" ");
	*/
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

EYapDialogueTalkSequencing UFlowNode_YapDialogue::GetMultipleFragmentSequencing() const
{
	return TalkSequencing;
}

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
	OldTalkSequencing = EYapDialogueTalkSequencing::COUNT;
	
	uint8 AsInt = static_cast<uint8>(TalkSequencing);

	if (++AsInt >= static_cast<uint8>(EYapDialogueTalkSequencing::COUNT))
	{
		switch (GetNodeType())
		{
			case EYapDialogueNodeType::Talk:
			{
				AsInt = 0;
				break;
			}
			case EYapDialogueNodeType::TalkAndAdvance:
			{
				AsInt = (uint8)EYapDialogueTalkSequencing::SelectOne;
				break;
			}
			default:
			{
				break;
			}
		}
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
	FString SuperDescription = Super::GetNodeDescription();

	if (!SuperDescription.IsEmpty())
	{
		return SuperDescription;
	}

	if (!HasValidConfig())
	{
		if (GetClass() == UFlowNode_YapDialogue::StaticClass())
		{
			return "Invalid config!\nYou must assign a default config in Yap project settings for the default dialogue node to work.";
		}

		return "Invalid config!\nYou must assign a config for this node type in the Yap Node asset.";
	}

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

	/*
	if (ParentTagContainer.HasTagExact(GetNodeConfig().GetDialogueTagsParent()))
	{
		bArg = true;
	}
	*/
	
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
bool UFlowNode_YapDialogue::ToggleNodeType(bool bHasOutputConnections)
{
	uint8 AllowableNodeTypes = GetNodeConfig().General.AllowableNodeTypes;
	
	// If the node config isn't valid, do nothing
	if (AllowableNodeTypes == 0)
	{
		UE_LOG(LogYap, Error, TEXT("%s: Cannot toggle node type - node config is invalid!"), *GetName());
		return false;
	}

	// Only allow specific changes when pins are connected
	if (bHasOutputConnections)
	{
		if (GetNodeType() == EYapDialogueNodeType::PlayerPrompt)
		{
			// Do nothing
		}
		else if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
		{
			AllowableNodeTypes = AllowableNodeTypes & (uint8)(EYapDialogueNodeType::Talk);
		}
		else if (GetNodeType() == EYapDialogueNodeType::Talk)
		{
			AllowableNodeTypes = AllowableNodeTypes & (uint8)(EYapDialogueNodeType::TalkAndAdvance);
		}
	}

	if (AllowableNodeTypes == 0)
	{
		return false;
	}
	
	// Cycle through allowable node types
	uint8 AsInt = static_cast<uint8>(DialogueNodeType);

	do
	{
		AsInt = AsInt << 1;
		
		if (AsInt >= static_cast<uint8>(EYapDialogueNodeType::COUNT))
		{
			AsInt = 1 << 0;
		}
	}
	while ((AsInt & AllowableNodeTypes) == 0); // If the next setting isn't an allowable node type keep trying new allowable node types

	DialogueNodeType = static_cast<EYapDialogueNodeType>(AsInt);
	
	if (DialogueNodeType == EYapDialogueNodeType::TalkAndAdvance)
	{
		if (TalkSequencing < EYapDialogueTalkSequencing::SelectOne)
		{
			OldTalkSequencing = TalkSequencing;
			TalkSequencing = EYapDialogueTalkSequencing::SelectOne;
		}
	}
	else if (DialogueNodeType == EYapDialogueNodeType::Talk)
	{
		if (OldTalkSequencing != EYapDialogueTalkSequencing::COUNT)
		{
			TalkSequencing = OldTalkSequencing;
		}
	}

	return true;
}

FString UFlowNode_YapDialogue::GetStatusString() const
{
	return {};
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

void UFlowNode_YapDialogue::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	
	// TODO this should be removed in ~2026
	if (!IsTemplate() && !GEditor->IsPlayingSessionInEditor())
	{
		for (FYapFragment& Fragment : Fragments)
		{
			// Has a speaker asset set, but no tag
			if (!Fragment.SpeakerAsset.IsNull() && !Fragment.Speaker.IsValid())
			{
				UObject* LoadedSpeakerAsset = Fragment.SpeakerAsset.LoadSynchronous();

				if (const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedSpeakerAsset))
				{
					LoadedSpeakerAsset = Blueprint->GeneratedClass.GetDefaultObject();
				}

				if (LoadedSpeakerAsset->Implements<UYapCharacterInterface>())
				{
					FGameplayTag AssetTag = UYapProjectSettings::FindCharacterTag(Fragment.SpeakerAsset);
				
					if (AssetTag.IsValid())
					{
						Fragment.Speaker = AssetTag;
						Fragment.SpeakerAsset.Reset();

						UE_LOG(LogYap, Display, TEXT("Fixed up fragment using old speaker data (set speaker from asset <%s> to gameplay tag <%s>)"), *LoadedSpeakerAsset->GetName(), *Fragment.Speaker.ToString());
					
						Modify();
					}
					else
					{
						UE_LOG(LogYap, Warning, TEXT("Could not fix up fragment using old speaker data (speaker uses asset <%s>, but asset has no gameplay tag set)"), *LoadedSpeakerAsset->GetName());
					}
				}
			}
			else if (!Fragment.SpeakerAsset.IsNull() && Fragment.Speaker.IsValid())
			{
				Fragment.SpeakerAsset.Reset();
				Modify();
			}

			if (!Fragment.DirectedAtAsset.IsNull() && !Fragment.DirectedAt.IsValid())
			{
				UObject* LoadedDirectedAtAsset = Fragment.DirectedAtAsset.LoadSynchronous();

				if (const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedDirectedAtAsset))
				{
					LoadedDirectedAtAsset = Blueprint->GeneratedClass.GetDefaultObject();
				}

				if (LoadedDirectedAtAsset->Implements<UYapCharacterInterface>())
				{
					FGameplayTag AssetTag = UYapProjectSettings::FindCharacterTag(Fragment.DirectedAtAsset);
				
					if (AssetTag.IsValid())
					{
						Fragment.DirectedAt = AssetTag;
						Fragment.DirectedAtAsset.Reset();

						UE_LOG(LogYap, Display, TEXT("Fixed up fragment using old directed at data (set directed at from asset <%s> to gameplay tag <%s>)"), *LoadedDirectedAtAsset->GetName(), *Fragment.DirectedAt.ToString());
					
						Modify();
					}
					else
					{
						UE_LOG(LogYap, Warning, TEXT("Could not fix up fragment using old directed at data (directed at uses asset <%s>, but asset has no gameplay tag set)"), *LoadedDirectedAtAsset->GetName());
					}
				}
			}
			else if (!Fragment.DirectedAtAsset.IsNull() && Fragment.DirectedAt.IsValid())
			{
				Fragment.DirectedAtAsset.Reset();
				Modify();
			}
		}
	}
}
#endif

#if WITH_EDITOR
void UFlowNode_YapDialogue::FixNode(UEdGraphNode* NewGraphNode)
{
	Super::FixNode(NewGraphNode);
	
	uint8 AllowableNodeTypes = GetNodeConfig().General.AllowableNodeTypes;

	if (AllowableNodeTypes == 0)
	{
		return;
	}

	uint8 Breakout = 0;
	while (((uint8)(DialogueNodeType) & AllowableNodeTypes) == 0)
	{
		ToggleNodeType(false);
		Breakout++;

		if (Breakout > (uint8)EYapDialogueNodeType::COUNT)
		{
			break;
		}
	}
}
#endif

// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE