// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "Yap/YapBroker.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/Interfaces/IYapConversationHandler.h"
#include "Yap/YapRunningFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Interfaces/IYapFreeSpeechHandler.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_BROADCAST_EVT_TARGS(NAME, CPPFUNC, K2FUNC) U##NAME, I##NAME, &I##NAME::CPPFUNC, &I##NAME::K2FUNC

TWeakObjectPtr<UWorld> UYapSubsystem::World = nullptr;
bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;

// ------------------------------------------------------------------------------------------------

UYapSubsystem::UYapSubsystem()
{
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterConversationHandler(UObject* NewHandler)
{
	if (NewHandler->Implements<UYapConversationHandler>())
	{
		Get()->ConversationHandlers.AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but object does not implement the IYapConversationHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterConversationHandler(UObject* HandlerToRemove)
{
	Get()->ConversationHandlers.Remove(HandlerToRemove);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterFreeSpeechHandler(UObject* NewHandler)
{
	if (NewHandler->Implements<UYapFreeSpeechHandler>())
	{
		Get()->FreeSpeechHandlers.AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a free speech handler, but object does not implement the IYapFreeSpeechHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterFreeSpeechHandler(UObject* HandlerToRemove)
{
	Get()->FreeSpeechHandlers.Remove(HandlerToRemove);
}

// ------------------------------------------------------------------------------------------------

UYapCharacterComponent* UYapSubsystem::FindCharacterComponent(FGameplayTag CharacterTag)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = Get()->YapCharacterComponents.Find(CharacterTag);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

UYapBroker* UYapSubsystem::GetBroker()
{
	UYapBroker* Broker = Get()->Broker;

#if WITH_EDITOR
	ensureMsgf(IsValid(Broker), TEXT("Conversation Broker is invalid. Did you create one and assign it in project settings? Docs: https://github.com/HomerJohnston/UE-FlowYap/wiki/Conversation-Broker"));
#endif
	
	return Broker;		
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapSubsystem::GetCurrentMaturitySetting()
{
	EYapMaturitySetting MaturitySetting;
	
	if (!ensureMsgf(World.IsValid(), TEXT("World was invalid in UYapSubsystem::GetGameMaturitySetting(). This should never happen! Defaulting to mature.")))
	{
		MaturitySetting = EYapMaturitySetting::Mature; 
	}
	else
	{
		UYapBroker* Broker = GetBroker();

		if (ensureMsgf(IsValid(Broker), TEXT("No broker set in project settings! Defaulting to mature.")))
		{
			MaturitySetting = Broker->GetMaturitySetting();
		}
		else
		{
			MaturitySetting = EYapMaturitySetting::Mature;
		}	
	}

	// Something went wrong... we will hard-code default to mature.
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		bool bSetWarningIssued = false;

		if (!bGetGameMaturitySettingWarningIssued)
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem::GetGameMaturitySetting failed to get a valid game maturity setting! Using default project maturity setting. This could be caused by a faulty Broker implementation."));
			bSetWarningIssued = true;
		}
		
		MaturitySetting = EYapMaturitySetting::Mature;
		
		if (bSetWarningIssued)
		{
			bGetGameMaturitySettingWarningIssued = true;
		}
	}
	
	return MaturitySetting;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode)
{
	if (TaggedFragments.Contains(FragmentTag))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register tagged fragment with tag [%s] but this tag was already registered! Find and fix the duplicate tag usage."), *FragmentTag.ToString()); // TODO if I pass in the full fragment I could log the dialogue text to make this easier for designers?
		return;
	}
	
	TaggedFragments.Add(FragmentTag, DialogueNode);
}

// ------------------------------------------------------------------------------------------------

FYapFragment* UYapSubsystem::FindTaggedFragment(const FGameplayTag& FragmentTag)
{
	UFlowNode_YapDialogue** DialoguePtr = TaggedFragments.Find(FragmentTag);

	if (DialoguePtr)
	{
		return (*DialoguePtr)->FindTaggedFragment(FragmentTag);
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::OpenConversation(const FGameplayTag& ConversationName)
{
	// Return existing conversation by same name
	auto Match = [&ConversationName] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ConversationName;
	};
	
	int32 Index = ConversationQueue.IndexOfByPredicate(Match);
	if (Index != INDEX_NONE)
	{
		UE_LOG(LogYap, Display, TEXT("Tried to start a new conversation but converation was already open or in queue!"), *ConversationName.ToString());
		return ConversationQueue[Index];
	}
	
	// Conversation structs will usually be small, a few bytes. Using arrays as queues for easier serialization.
	ConversationQueue.EmplaceAt(0, FYapConversation(ConversationName));

	if (ConversationQueue.Num() == 1)
	{
		StartOpeningConversation(ConversationQueue[0]);
	}

	return ConversationQueue[0];
}

// ------------------------------------------------------------------------------------------------

EYapCloseConversationResult UYapSubsystem::RequestCloseConversation(const FGameplayTag& ConversationName)
{
	if (ConversationQueue.Num() == 0)
	{
		return EYapCloseConversationResult::Failed;
	}

	auto Match = [&ConversationName] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ConversationName;
	};
	
	int32 ConversationIndex = ConversationQueue.IndexOfByPredicate(Match);

	if (ActiveConversationName == ConversationName)
	{
		// Don't remove it from the queue yet, let it actually finish
		return StartClosingConversation(ConversationName);
	}
	else
	{
		// Just remove this conversation from queues.
		ConversationQueue.RemoveAll(Match);
		return EYapCloseConversationResult::Closed;
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::StartNextQueuedConversation()
{
	if (ConversationQueue.Num() == 0)
	{
		return;
	}
	else
	{
		StartOpeningConversation(ConversationQueue[ConversationQueue.Num() - 1]);	
	}
}


// ------------------------------------------------------------------------------------------------

void UYapSubsystem::StartOpeningConversation(FYapConversation& Conversation)
{
	if (ActiveConversationName == Conversation.GetConversationName())
	{
		UE_LOG(LogYap, Display, TEXT("Tried to start a new conversation but converation was already active!"), *Conversation.GetConversationName().ToString());
		return;
	}

	UE_LOG(LogYap, Display, TEXT("Subsystem: Starting conversation %s!"), *Conversation.GetConversationName().ToString());
	ActiveConversationName = Conversation.GetConversationName();
	
	FYapData_ConversationOpened Data;
	Data.Conversation = ActiveConversationName;

	FYapConversationHandle Handle(Conversation.GetGuid());

	// Game code may add opening locks to the conversation here
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationOpened, Execute_K2_ConversationOpened)>(ConversationHandlers, Data, Handle);

	Conversation.StartOpening();
}

// ------------------------------------------------------------------------------------------------

EYapCloseConversationResult UYapSubsystem::StartClosingConversation(const FGameplayTag& ConversationName)
{
	auto Find = [&ConversationName] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ConversationName;
	};

	FYapConversation& Conversation = GetConversation(ConversationName);

	Conversation.StartClosing();

	if (Conversation.GetState() == EYapConversationState::Closed)
	{
		ActiveConversationName = FGameplayTag::EmptyTag;
		ConversationQueue.RemoveAll(Find);
		StartNextQueuedConversation();
		return EYapCloseConversationResult::Closed;
	}

	Conversation.OnConversationClosed.AddDynamic(this, &UYapSubsystem::OnActiveConversationClosed);
	
	return EYapCloseConversationResult::Closing;
}

void UYapSubsystem::OnActiveConversationClosed()
{
	auto Find = [this] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ActiveConversationName;
	};
	
	ConversationQueue.RemoveAll(Find);
	ActiveConversationName = FGameplayTag::EmptyTag;
	StartNextQueuedConversation();
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle UYapSubsystem::BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	/*
	// TODO not sure if there's a clean way to avoid const_cast. The problem is that GetDirectedAt and GetSpeaker (used below) are mutable, because they forcefully load assets.
	FYapFragment& Fragment = const_cast<FYapFragment&>(Dialogue->GetFragmentByIndex(FragmentIndex));
	FYapBit& Bit = const_cast<FYapBit&>(Fragment.GetBit()); 

	FGameplayTag ConversationName;

	if (ActiveConversationName.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversationName.Conversation.GetValue();
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);

	if (ConversationName.IsValid())
	{
		FYapData_ConversationPlayerPromptCreated Data;
		Data.Conversation = ConversationName;
		Data.Handle = Handle;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();

		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptCreated, Execute_K2_ConversationPlayerPromptCreated)>(ConversationHandlers, Data);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to create a player prompt, but there is no active conversation!"));
	}

	*/
	return FYapPromptHandle();
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnFinishedBroadcastingPrompts()
{
	/*
	FGameplayTag ConversationName = ActiveConversation.IsConversationInProgress() ? ActiveConversation.Conversation.GetValue() : FGameplayTag::EmptyTag;

	if (ConversationName.IsValid())
	{
		FYapData_ConversationPlayerPromptsReady Data;
		Data.Conversation = ConversationName;
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptsReady, Execute_K2_ConversationPlayerPromptsReady)>(ConversationHandlers, Data);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to broadcast player prompts created, but there is no active conversation!"));
	}
	*/
}

// ------------------------------------------------------------------------------------------------

FYapFragmentHandle UYapSubsystem::RunSpeech(const FYapData_SpeechBegins& SpeechData)
{
	FYapRunningFragment RunningFragment;

	FYapFragmentHandle FragmentHandle(RunningFragment);
	
	if (SpeechData.SpeechTime > 0)
	{
		FTimerHandle SpeechTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeechComplete, FragmentHandle);
		GetWorld()->GetTimerManager().SetTimer(SpeechTimerHandle, Delegate, SpeechData.SpeechTime, false);
		RunningFragment.SetSpeechTimerHandle(SpeechTimerHandle);
	}

	if (SpeechData.FragmentTime > 0)
	{
		FTimerHandle FragmentTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnFragmentComplete, FragmentHandle);
		GetWorld()->GetTimerManager().SetTimer(FragmentTimerHandle, Delegate, SpeechData.SpeechTime, false);
		RunningFragment.SetFragmentTimerHandle(FragmentTimerHandle);
	}
	
	if (SpeechData.Conversation.IsValid())
	{
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(ConversationHandlers, SpeechData, FragmentHandle);
	}
	else
	{
		//BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(FreeSpeechHandlers, SpeechData, FragmentHandle);
	}

	return FragmentHandle;
}

// ------------------------------------------------------------------------------------------------

FGameplayTag UYapSubsystem::GetConversation(UFlowAsset* FlowAsset)
{
	return FGameplayTag::EmptyTag;
}

FYapConversation& UYapSubsystem::GetConversation(FYapConversationHandle Handle)
{
	auto Find = [Handle] (const FYapConversation& Conversation)
	{
		return Conversation.GetGuid() == Handle.Guid;
	};

	auto& Queue = Get()->ConversationQueue;
	
	FYapConversation* ConversationPtr = Get()->ConversationQueue.FindByPredicate(Find);

	if (ConversationPtr)
	{
		return *ConversationPtr;
	}

	static FYapConversation NullConversation;
	return NullConversation;
}

FYapConversation& UYapSubsystem::GetConversation(const FGameplayTag& ConversationName)
{
	TArray<FYapConversation>& Conversations = Get()->ConversationQueue;
	
	for (int32 i = 0; i < Conversations.Num(); ++i)
	{
		if (Conversations[i].GetConversationName() == ConversationName)
		{
			return Conversations[i];
		}
	}

	static FYapConversation NullConversation;
	return NullConversation;	
}

// ------------------------------------------------------------------------------------------------

/*
void UYapSubsystem::BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	// TODO not sure if there's a clean way to avoid const_cast. The problem is that GetDirectedAt and GetSpeaker (used below) are mutable, because they forcefully load assets.
	FYapFragment& Fragment = const_cast<FYapFragment&>(Dialogue->GetFragmentByIndex(FragmentIndex));
	FYapBit& Bit = const_cast<FYapBit&>(Fragment.GetBit());
	FYapRunningFragmentHandle FragmentHandleRef(Dialogue->DialogueHandle.GetGuid());
	GuidDialogueMap.Add(FragmentHandleRef, Dialogue);
	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}
	
	TOptional<float> Time = Fragment.GetSpeechTime();

	float EffectiveTime;
	
	if (Time.IsSet())
	{
		EffectiveTime = Time.GetValue();
	}
	else
	{
		EffectiveTime = UYapProjectSettings::GetMinimumFragmentTime();
	}
	
	
	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechBegins Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = FragmentHandleRef;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();
		Data.SpeechTime = EffectiveTime;
		Data.PaddedTime = Fragment.GetProgressionTime(); 
		Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
		Data.bSkippable = Fragment.GetSkippable(Dialogue->GetSkippable());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechBegins Data;
		Data.DialogueHandleRef = FragmentHandleRef;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();
		Data.SpeechTime = EffectiveTime;
		Data.PaddedTime = Fragment.GetProgressionTime(); 
		Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
		Data.bSkippable = Fragment.GetSkippable(Dialogue->GetSkippable());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(FreeSpeechHandlers, Data);
	}
}
*/

// ------------------------------------------------------------------------------------------------

/*
void UYapSubsystem::BroadcastDialogueEnd(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGameplayTag ConversationName;
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	FYapRunningFragmentHandle DialogueHandleRef(Dialogue->DialogueHandle.GetGuid());

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechEnds Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.bWasTimed = Fragment.GetSpeechTime() != 0.0f;
		Data.PaddedTime = Fragment.GetProgressionTime();
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechEnds, Execute_K2_ConversationSpeechEnds)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechEnds Data;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.PaddedTime = Fragment.GetProgressionTime();
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechEnds, Execute_K2_TalkSpeechEnds)>(FreeSpeechHandlers, Data);
	}	
}
*/

// ------------------------------------------------------------------------------------------------

/*
void UYapSubsystem::BroadcastFragmentComplete(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGameplayTag ConversationName;
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	FYapRunningFragmentHandle DialogueHandleRef(Dialogue->DialogueHandle.GetGuid());

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechPaddingEnds Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = FYapRunningFragmentHandle(Dialogue->DialogueHandle.GetGuid());
		Data.bManualAdvance = !Dialogue->GetFragmentAutoAdvance(FragmentIndex);
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechPaddingEnds, Execute_K2_ConversationSpeechPaddingEnds)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechPaddingEnds Data;
		Data.DialogueHandleRef = FYapRunningFragmentHandle(Dialogue->DialogueHandle.GetGuid());
		Data.bManualAdvance = !Dialogue->GetFragmentAutoAdvance(FragmentIndex);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechPaddingEnds, Execute_K2_TalkSpeechPaddingEnds)>(FreeSpeechHandlers, Data);
	}

	GuidDialogueMap.Remove(DialogueHandleRef);
}
*/

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::RunPrompt(const FYapPromptHandle& Handle)
{
	return false;
	/*
	UYapSubsystem* This = Get();
	check(This);
	
	// TODO handle invalid handles gracefully
	Handle.GetDialogueNode()->RunPrompt(Handle.GetFragmentIndex());

	FGameplayTag ConversationName;

	if (This->ActiveConversation.FlowAsset == Handle.GetDialogueNode()->GetFlowAsset())
	{
		ConversationName = This->ActiveConversation.Conversation.GetValue();
	}
	
	FYapData_ConversationPlayerPromptChosen Data;
	Data.Conversation = ConversationName;
	
	This->BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptChosen, Execute_K2_ConversationPlayerPromptChosen)>(This->ConversationHandlers, Data);

	return true;
	*/
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::SkipDialogue(const FYapFragmentHandle& Handle)
{
	return false;
	/*
	// TODO handle invalid handles gracefully
	TWeakObjectPtr<UFlowNode_YapDialogue>* DialogueWeak = Get()->GuidDialogueMap.Find(Handle);

	if (DialogueWeak)
	{
		return (*DialogueWeak)->Skip();
	}

	return false;
	*/
}

// ------------------------------------------------------------------------------------------------

FYapRunningFragment& UYapSubsystem::GetFragmentHandle(FYapFragmentHandle HandleRef)
{
	return Get()->RunningFragments.FindChecked(HandleRef);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	if (RegisteredYapCharacterActors.Contains(Actor))
	{
		UE_LOG(LogYap, Error, TEXT("Multiple character components on actor, ignoring! Actor: %s"), *Actor->GetName());
		return;
	}

	YapCharacterComponents.Add(YapCharacterComponent->GetCharacterTag(), YapCharacterComponent);
	
	RegisteredYapCharacterActors.Add(Actor);
}

void UYapSubsystem::UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	YapCharacterComponents.Remove(YapCharacterComponent->GetCharacterTag());
	RegisteredYapCharacterActors.Remove(Actor);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	World = Cast<UWorld>(GetOuter());

	// TODO handle null unset values
	TSubclassOf<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass().LoadSynchronous();

	if (BrokerClass)
	{
		Broker = NewObject<UYapBroker>(this, BrokerClass);
	}

	bGetGameMaturitySettingWarningIssued = false;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Deinitialize()
{
	World = nullptr;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	if (IsValid(Broker))
	{
		Broker->Initialize_Internal();
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnSpeechComplete(FYapFragmentHandle Handle)
{
	RunningSpeech.Remove(Handle);
}

void UYapSubsystem::OnFragmentComplete(FYapFragmentHandle Handle)
{
	RunningFragments.Remove(Handle);
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#undef LOCTEXT_NAMESPACE