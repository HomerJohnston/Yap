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
#include "Algo/ForEach.h"
#include "Engine/Blueprint.h"
#include "Yap/Enums/YapAutoAdvanceFlags.h"
#include "Yap/Enums/YapInterruptibleFlags.h"

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

// The challenge here is that the Yap subsystem doesn't do padding. Only the flow graph does padding.
// If the subsystem receives an advancement request but the dialogue node is done speaking (in padding)
// then the subsystem won't push any OnSpeechCompleted events to the dialogue node. I need to subscribe
// the flow graph dialogue node to cancel events separately.
void UFlowNode_YapDialogue::OnAdvanceConversation(UObject* Instigator, FYapConversationHandle Handle)
{
	// TODO make sure this dialogue node is actually in this conversation, URGENT
	if (!FocusedFragmentIndex.IsSet() || !FocusedSpeechHandle.IsValid())
	{
		UE_LOG(LogYap, Warning, TEXT("%s: OnAdvanceConversation called while focused fragment wasn't set; ignoring request"), *GetName());
		return;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnAdvanceConversation [CH %s]"), *GetName(), *Handle.ToString());
	
	auto RunningFragmentsCopy = RunningFragments;

	bool bForceAdvance = Fragments[FocusedFragmentIndex.GetValue()].IsAwaitingManualAdvance() || !SpeakingFragments.Contains(FocusedSpeechHandle) && FragmentsInPadding.Contains(FocusedSpeechHandle);
	
	for (auto& [SpeechHandle, Index] : RunningFragmentsCopy)
	{
		FYapFragment& Fragment = Fragments[Index];
		FTimerHandle& PaddingTimerHandle = Fragment.PaddingTimerHandle;

		if (PaddingTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(PaddingTimerHandle);
		}
	}

	FragmentsInPadding.Empty(FragmentsInPadding.Num());
	
	if (bForceAdvance)
	{
		// If the focused (most recent) fragment is just waiting for padding then the normal OnSpeechComplete event isn't going to fire for it. We need to forcefully advance.
		FinishFragment(FocusedSpeechHandle, FocusedFragmentIndex.GetValue());
		AdvanceFromFragment(FocusedSpeechHandle, FocusedFragmentIndex.GetValue());
	}
	else
	{
		// The OnSpeechComplete event is going to fire, set a flag for it to use
		bForceAdvanceOnSpeechComplete = true;
	}
}

void UFlowNode_YapDialogue::FinishNode(FName OutputPinToTrigger)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: FinishNode - Unbinding from OnAdvanceConversation"), *GetName());
	
	bNodeActive = false;
	FocusedFragmentIndex.Reset();
	FocusedSpeechHandle.Invalidate();
	
	TriggerOutput(OutputPinToTrigger, true, EFlowPinActivationType::Default);
}

void UFlowNode_YapDialogue::SetActive()
{
	return;
}

void UFlowNode_YapDialogue::SetInactive()
{
	return;
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
	/*
	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		return 0.0f;
	}
	*/
	
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

	bool bInConversation = UYapSubsystem::IsSpeechInConversation(this, Handle);
	
	// Is skipping allowed or not?
	//bool bPreventSkippingTimers = !Fragment.GetInterruptible((this->GetInterruptible(bInConversation), false);
	
	//if (bPreventSkippingTimers)
	//{
//		return false;
//	}

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
		
		bool bStartedSuccessfully = IsPlayerPrompt() ? TryBroadcastPrompts() : TryStartFragments();

		if (bStartedSuccessfully)
		{			
			++NodeActivationCount;
		}
		else
		{
#if !UE_BUILD_SHIPPING
			if (!Connections.Contains(BypassPinName))
			{
				UE_LOG(LogYap, Error, TEXT("Tried to trigger bypass pin, but it was not connected!"));
			}
#endif
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

// ------------------------------------------------------------------------------------------------

const FYapFragment& UFlowNode_YapDialogue::GetFragment(uint8 FragmentIndex) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	return Fragments[FragmentIndex];
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetInterruptible(bool bInConversation) const
{
	EYapInterruptibleFlags Flags = InterruptibleFlags.IsSet()
		? InterruptibleFlags.GetValue()
		: (EYapInterruptibleFlags)GetNodeConfig().DialoguePlayback.AutoAdvanceFlags;
	
	if (bInConversation)
	{
		return ((Flags & EYapInterruptibleFlags::Conversation) == EYapInterruptibleFlags::Conversation);
	}
	else
	{
		return ((Flags & EYapInterruptibleFlags::FreeSpeech) == EYapInterruptibleFlags::FreeSpeech);			
	}
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetNodeAutoAdvance(bool bInConversation) const
{
	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		return true;
	}
	
	if (IsPlayerPrompt() && GetNodeConfig().GetPromptAdvancesImmediately())
	{
		return true;
	}

	EYapAutoAdvanceFlags Mask;
	
	if (bInConversation)
	{
		Mask = EYapAutoAdvanceFlags::Conversation;
	}
	else
	{
		Mask = EYapAutoAdvanceFlags::FreeSpeech;
	}

	EYapAutoAdvanceFlags Flags = (AutoAdvanceFlags.IsSet()) ? AutoAdvanceFlags.GetValue() : (EYapAutoAdvanceFlags)GetNodeConfig().DialoguePlayback.AutoAdvanceFlags;
	
	return (Mask & Flags) == Mask;
}

// ------------------------------------------------------------------------------------------------

bool UFlowNode_YapDialogue::GetFragmentAutoAdvance(uint8 FragmentIndex, bool bInConversation) const
{
	check(Fragments.IsValidIndex(FragmentIndex));

	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		return true;
	}
	
	const FYapFragment& Fragment = Fragments[FragmentIndex]; 
	
	// Use fragment override
	if (Fragment.GetAutoAdvanceSetting().IsSet())
	{
		EYapAutoAdvanceFlags Mask;

		if (bInConversation)
		{
			Mask = EYapAutoAdvanceFlags::Conversation;
		}
		else
		{
			Mask = EYapAutoAdvanceFlags::FreeSpeech;
		}

		EYapAutoAdvanceFlags Flags = Fragment.GetAutoAdvanceSetting().GetValue();

		return (Mask & Flags) == Mask;
	}

	return GetNodeAutoAdvance(bInConversation);
}

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

	const FYapConversation* Conversation = Subsystem->GetConversationByOwner(this, GetFlowAsset()); 

	if (!Conversation)
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to broadcast prompt options but there was no active conversation!"));
		return false;
	}
	
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
 		Data.Conversation = Conversation->GetHandle();

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
		Data.Conversation = Conversation->GetHandle();
		Subsystem->OnFinishedBroadcastingPrompts(Data, this->GetClass());
	}
	
	return true;
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::RunPrompt(uint8 FragmentIndex)
{
	UYapSubsystem::Get(GetWorld())->OnPromptChosen.RemoveDynamic(this, &ThisClass::OnPromptChosen);

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
			if (i == LastRanFragment && !GetNodeConfig().DialoguePlayback.bRandomAllowsSelectingSameFragment)
			{
				continue;
			}
			
			if (Fragments[i].CanRun())
			{
				ValidFragments.Add(i);
			}
		}

		// If we didn't find any valid fragments but we culled out the last ran fragment, put it back in the pool.
		if (ValidFragments.Num() == 0 && LastRanFragment != INDEX_NONE && Fragments.Num() > LastRanFragment)
		{
			ValidFragments.Add(LastRanFragment);
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

	// TODO: the select random node needs to check this for all fragments. I should probably chop off the rest of this function into something else and call that from the random mode.
	if (!FragmentCanRun(FragmentIndex))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("FAILED - FragmentCanRun returned false"));
		Fragment.SetStartTime(-1.0);
		Fragment.SetEndTime(-1.0);
		return false;
	}

	LastRanFragment = FragmentIndex;
	
	Fragment.SetRunState(EYapFragmentRunState::Running);
	Fragment.ClearAwaitingManualAdvance();
	
	const FYapBit& Bit = Fragment.GetBit(GetWorld());
	const UYapNodeConfig& ActiveConfig = GetNodeConfig();

	TOptional<float> SpeechTime = Fragment.GetSpeechTime(GetWorld(), ActiveConfig);

	float EffectiveTime = 0.0f;
	
	if (SpeechTime.IsSet())
	{
		EffectiveTime = SpeechTime.GetValue();
	}
	
	UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
	
	FYapData_SpeechBegins Data;

	if (FYapConversation* Conversation = Subsystem->GetConversationByOwner(GetWorld(), GetFlowAsset()))
	{
		Data.Conversation = Conversation->GetConversationName();
		InConversation = Data.Conversation;
	}
	else
	{
		InConversation = NAME_None;
	}
	
	bool bInConversation = InConversation != NAME_None;

	float PaddingTime = 0;

	if (Fragment.GetUsesPadding(GetWorld(), ActiveConfig))
	{
		PaddingTime = Fragment.GetProgressionTime(GetWorld(), ActiveConfig);
		
		if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
		{
			EffectiveTime = PaddingTime;
			PaddingTime = 0;
		}
	}

	if (!GetFragmentAutoAdvance(FragmentIndex, bInConversation))
	{
		EffectiveTime = PaddingTime;
		PaddingTime = 0;
	}
	
	if (ActiveConfig.GetUsesDirectedAt())
	{
		Data.DirectedAtID = Fragment.GetDirectedAtTag().GetTagName();
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
	
	Data.bSkippable = Fragment.GetInterruptible(GetInterruptible(bInConversation), bInConversation);

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
	FocusedSpeechHandle = Subsystem->GetNewSpeechHandle(Fragment.GetGuid(), Data.SpeakerID, Data.Speaker.GetObject(), bInConversation ? GetFlowAsset() : nullptr);
	FocusedFragmentIndex = FragmentIndex;

	AddRunningFragment(FocusedSpeechHandle, FragmentIndex);
	SpeakingFragments.Add(FocusedSpeechHandle);

	Fragment.SetStartTime(GetWorld()->GetTimeSeconds());
	Fragment.IncrementActivations();
	
	Subsystem->RunSpeech(Data, GetClass(), FocusedSpeechHandle);

	if (GetNodeType() == EYapDialogueNodeType::TalkAndAdvance) // TODO || something else?
	{
		UYapSubsystem::Get(this)->MarkConversationSpeechAsFragile(FocusedSpeechHandle);
	}
	
	BindToSubsystemSpeechCompleteEvent(FocusedSpeechHandle);
	
	if (EffectiveTime <= 0.0f || GetNodeType() == EYapDialogueNodeType::TalkAndAdvance)
	{
		OnPaddingComplete(FocusedSpeechHandle);
	}
	
	if (PaddingTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(Fragment.PaddingTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingComplete, FocusedSpeechHandle), PaddingTime, false);
		FragmentsInPadding.Add(FocusedSpeechHandle);	
	}
	
	TriggerSpeechStartPin(FragmentIndex);
	
	return true;
}

void UFlowNode_YapDialogue::AddRunningFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex)
{
	if (RunningFragments.Num() == 0)
	{
		if (GetNodeType() != EYapDialogueNodeType::TalkAndAdvance)
		{
			UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.AddDynamic(this, &ThisClass::OnAdvanceConversation);
		}
	}

	RunningFragments.Add(Handle, FragmentIndex);
}

void UFlowNode_YapDialogue::RemoveRunningFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex)
{
	RunningFragments.Remove(Handle);

	if (RunningFragments.Num() == 0)
	{
		if (GetNodeType() != EYapDialogueNodeType::TalkAndAdvance)
		{
			//UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.RemoveDynamic(this, &ThisClass::OnAdvanceConversation);
		}

		InConversation = NAME_None;
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::BindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: Binding to Speech Completed Events {%s}"), *GetName(), *Handle.ToString());
	
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_ThreeParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle, EYapSpeechCompleteResult));

	UYapSubsystem::BindToSpeechFinish(this, Handle, Delegate);
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::UnbindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: Unbinding from Speech Completed Events {%s}"), *GetName(), *Handle.ToString());
	
	FYapSpeechEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED_ThreeParams(ThisClass, OnSpeechComplete, UObject*, FYapSpeechHandle, EYapSpeechCompleteResult));

	UYapSubsystem::UnbindToSpeechFinish(this, Handle, Delegate);
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnSpeechComplete(UObject* Instigator, FYapSpeechHandle Handle, EYapSpeechCompleteResult Result)
{
	uint8* FragmentIndex = RunningFragments.Find(Handle);
	
	if (!FragmentIndex)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete; ignored; handle {%s} was not running"), *GetName(), *Handle.ToString());
		return;
	}
	
	UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: OnSpeechComplete {%s}"), *GetName(), *FragmentIndex, *Handle.ToString());

	FYapFragment& Fragment = Fragments[*FragmentIndex];
	
	SpeakingFragments.Remove(Handle);

	TriggerSpeechEndPin(*FragmentIndex);

	// No positive padding - this fragment is done
	if (!FragmentsInPadding.Contains(Handle))
	{
		FinishFragment(Handle, *FragmentIndex);

		if (GetNodeType() != EYapDialogueNodeType::TalkAndAdvance)
		{
			TryAdvanceFromFragment(Handle, *FragmentIndex);
		}
	}
}

// ------------------------------------------------------------------------------------------------

void UFlowNode_YapDialogue::OnPaddingComplete(FYapSpeechHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}
	
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

	RemoveRunningFragment(Handle, FragmentIndex);
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

	bool bInConversation = UYapSubsystem::IsNodeInConversation(this);
	
	// TODO When calling AdvanceFromFragment in Skip function, if the game is set to do manual advancement, this won't run. Push this into a separate function I can call or add another route into this.
	if (bForceAdvanceOnSpeechComplete || GetFragmentAutoAdvance(FragmentIndex, bInConversation))
	{
		bForceAdvanceOnSpeechComplete = false;
		
		UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment passed - GetFragmentAutoAdvance true)"), *GetName(), FragmentIndex);

		AdvanceFromFragment(Handle, FragmentIndex);
	}
	else
	{
		/*
		if (GetNodeConfig().GetManualAdvanceFreeSpeech() && !bInConversation)
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment passed - GetFragmentAutoAdvance true)"), *GetName(), FragmentIndex);
			AdvanceFromFragment(Handle, FragmentIndex);
		}
		else
		{
		*/
			UE_LOG(LogYap, VeryVerbose, TEXT("%s [%i]: TryAdvanceFromFragment failed - fragment awaiting manual advance"), *GetName(), FragmentIndex);
			Fragment.SetAwaitingManualAdvance();
		/*
		}
		*/
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
	
	Fragment.ClearAwaitingManualAdvance();
	
	UYapSubsystem::Get(this)->OnAdvanceConversationDelegate.RemoveDynamic(this, &ThisClass::OnAdvanceConversation);

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
	// If it's a prompt node, enable the bypass -- it will be used if the node is entered without an active conversation
	if (GetNodeType() == EYapDialogueNodeType::PlayerPrompt)
	{
		return true;
	}
	
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

#if WITH_EDITOR
void UFlowNode_YapDialogue::InsertFragment(const FYapFragment& NewFragment, int32 InsertionIndex)
{
	Fragments.Insert(NewFragment, InsertionIndex);

	UpdateFragments(
	{
		UpdateFragmentIndices,
	});
	
	UpdateFragmentAudioIDs();
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
void UFlowNode_YapDialogue::DeleteFragmentByIndex(int32 DeleteIndex)
{
	if (!Fragments.IsValidIndex(DeleteIndex))
	{
		UE_LOG(LogYap, Error, TEXT("%s: Invalid deletion index [%i]!"), *GetName(), DeleteIndex);
	}

	Fragments.RemoveAt(DeleteIndex);

	UpdateFragments( { UpdateFragmentIndices} );

	UpdateFragmentAudioIDs();
}
#endif

#if WITH_EDITOR
void UFlowNode_YapDialogue::UpdateFragments(TArray<TFunction<void(UFlowNode_YapDialogue*, FYapFragment&, int32)>> Funcs)
{
	for (int i = 0; i < Fragments.Num(); ++i)
	{
		FYapFragment& Fragment = Fragments[i];
		
		for (auto& Func : Funcs)
		{
			Func(this, Fragment, i);
		}
	}

	ForceReconstruction();
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::UpdateFragmentIndices(UFlowNode_YapDialogue* Node, FYapFragment& Fragment, int32 Index)
{
	Fragment.SetIndexInDialogue(Index);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::UpdateFragmentAudioIDs()
{
	UYapBroker::GetInEditor().UpdateNodeFragmentIDs(this);
	
/*
	TArray<TCHAR> IllegalChars = Node->GetNodeConfig().GetIllegalAudioIDCharacters().GetCharArray();

	TArray<char> AlphaNumerics {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	TArray<TCHAR> Alphas {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

	for (int i = 0; i < IllegalChars.Num(); ++i)
	{
		AlphaNumerics.Remove(IllegalChars[i]);
		Alphas.Remove(IllegalChars[i]);
	}

	for (int i = 0; i < Pattern.Len(); ++i)
	{
		const TCHAR& Char = Pattern[i];

		switch (Char)
		{
			case '*':
			{
				Pattern[i] = Alphas[FMath::RandHelper(Alphas.Num())];
				break;
			}
			case '?':
			{
				Pattern[i] = AlphaNumerics[FMath::RandHelper(AlphaNumerics.Num())];
			}
			default:
			{
				
			}
		}
	}

	Fragment.SetAudioID(Pattern);
	*/
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR

#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UFlowNode_YapDialogue::SwapFragments(uint8 IndexA, uint8 IndexB)
{
	if (!Fragments.IsValidIndex(IndexA) || !Fragments.IsValidIndex(IndexB))
	{
		UE_LOG(LogYap, Error, TEXT("Could not swap fragments [%i] and [%i]! Unknown error."), IndexA, IndexB);
		return;
	}
	
	Fragments.Swap(IndexA, IndexB);

	UpdateFragments(
	{
		UpdateFragmentIndices,
	});

	//UpdateFragmentAudioIDs();
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

FString UFlowNode_YapDialogue::GetAudioID(uint8 FragmentIndex) const
{
	const FYapAudioIDFormat& AudioIDFormat = GetNodeConfig().GetAudioIDFormat();
	
	return GetAudioIDRoot() + AudioIDFormat.Separator + AudioIDFormat.ParseFragmentID(GetFragmentByIndex(FragmentIndex).GetAudioID());
}

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
		Yap::Editor::PostNotificationInfo_Warning(
			LOCTEXT("ToggleNodeTypeConfigError_Title", "Cannot Change Node Type"),
			LOCTEXT("ToggleNodeTypeConfigError_Desc", "Check Yap Node Config for this node type - no valid allowable node types?"));
			
		return false;
	}

	// Only allow specific changes when pins are connected
	if (bHasOutputConnections)
	{
		if (GetNodeType() == EYapDialogueNodeType::PlayerPrompt)
		{
			Yap::Editor::PostNotificationInfo_Warning(
				LOCTEXT("ToggleNodeTypeError_Title", "Cannot Change Node Type"),
				LOCTEXT("ToggleNodeTypeError_Desc", "You must disconnect outputs before you can change the node type."));
			
			return false;
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
		Yap::Editor::PostNotificationInfo_Warning(
			LOCTEXT("ToggleNodeTypeError_Title", "Cannot Change Node Type"),
			LOCTEXT("ToggleNodeTypeError_Desc", "You must disconnect outputs before you can change the node type."));
			
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

#if WITH_EDITOR
	if (!World && GEditor && GEditor->IsPlaySessionInProgress())
	{
		World = GEditor->PlayWorld;
	}
#endif

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
		Fragment.PreloadContent(World, MaturitySetting, LoadContext);
	}
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
