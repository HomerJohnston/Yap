// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "Yap/YapBroker.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/Interfaces/IYapConversationHandler.h"
#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Interfaces/IYapFreeSpeechHandler.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_BROADCAST_EVT_TARGS(NAME, CPPFUNC, K2FUNC) U##NAME, I##NAME, &I##NAME::CPPFUNC, &I##NAME::K2FUNC

TWeakObjectPtr<UWorld> UYapSubsystem::World = nullptr;
bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;
FYapConversation UYapSubsystem::NullConversation;

// ------------------------------------------------------------------------------------------------

UYapSubsystem::UYapSubsystem()
{
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterConversationHandler(UObject* NewHandler, FGameplayTag TypeGroup)
{
	if (NewHandler->Implements<UYapConversationHandler>())
	{
		FindOrAddConversationHandlerArray(TypeGroup).AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but object does not implement the IYapConversationHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterConversationHandler(UObject* HandlerToRemove, FGameplayTag TypeGroup)
{
	auto* Array = FindConversationHandlerArray(TypeGroup);

	if (Array)
	{
		Array->Remove(HandlerToRemove);
	}

	if (Array->IsEmpty())
	{
		Get()->ConversationHandlers.Remove(TypeGroup);
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterFreeSpeechHandler(UObject* NewHandler, FGameplayTag TypeGroup)
{
	if (NewHandler->Implements<UYapFreeSpeechHandler>())
	{
		FindOrAddFreeSpeechHandlerArray(TypeGroup).AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a free speech handler, but object does not implement the IYapFreeSpeechHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterFreeSpeechHandler(UObject* HandlerToRemove, FGameplayTag TypeGroup)
{
	auto* Array = FindFreeSpeechHandlerArray(TypeGroup);

	if (Array)
	{
		Array->Remove(HandlerToRemove);
	}
	
	if (Array->IsEmpty())
	{
		Get()->FreeSpeechHandlers.Remove(TypeGroup);
	}
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

FYapConversation& UYapSubsystem::OpenConversation(const FGameplayTag& ConversationName, UObject* ConversationOwner)
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
	ConversationQueue.EmplaceAt(0, FYapConversation(ConversationName, ConversationOwner));

	if (ConversationQueue.Num() == 1)
	{
		StartOpeningConversation(ConversationQueue[0]);
	}

	return ConversationQueue[0];
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::RequestCloseConversation(const FGameplayTag& ConversationName)
{
	if (ConversationQueue.Num() == 0)
	{
		return EYapConversationState::Closed;
	}

	auto Match = [&ConversationName] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ConversationName;
	};
	
	if (ActiveConversationName == ConversationName)
	{
		// Don't remove it from the queue yet, let it actually finish
		return StartClosingConversation(ConversationName);
	}
	else
	{
		// Just remove this conversation from queues.
		ConversationQueue.RemoveAll(Match);
		return EYapConversationState::Closed;
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
	Data.Conversation = ActiveConversationName.GetValue();

	FYapConversationHandle Handle(Conversation.GetGuid());

	auto* HandlerArray = FindConversationHandlerArray(Conversation.GetTypeGroup());

	// Game code may add opening locks to the conversation here
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationOpened, Execute_K2_ConversationOpened)>(HandlerArray, Data, Handle);

	Conversation.StartOpening();
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::StartClosingConversation(const FGameplayTag& ConversationName)
{
	auto Find = [&ConversationName] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ConversationName;
	};

	FYapConversation& Conversation = GetConversation(ConversationName);

	Conversation.StartClosing();

	if (Conversation.GetState() == EYapConversationState::Closed)
	{
		ActiveConversationName.Reset();
		ConversationQueue.RemoveAll(Find);
		StartNextQueuedConversation();
		return EYapConversationState::Closed;
	}

	Conversation.OnConversationClosed.AddDynamic(this, &UYapSubsystem::OnActiveConversationClosed);
	
	return EYapConversationState::Closing;
}

void UYapSubsystem::OnActiveConversationClosed()
{
	auto Find = [this] (const FYapConversation& Conversation)
	{
		return Conversation.GetConversationName() == ActiveConversationName;
	};
	
	ConversationQueue.RemoveAll(Find);
	ActiveConversationName.Reset();
	StartNextQueuedConversation();
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle UYapSubsystem::BroadcastPrompt(const FYapData_PlayerPromptCreated& Data, const FGameplayTag& TypeGroup)
{
	if (Data.Conversation.IsValid())
	{
		FSetElementId Elem = ActivePromptHandles.Emplace({TypeGroup});

		auto* HandlerArray = FindConversationHandlerArray(TypeGroup);

		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptCreated, Execute_K2_ConversationPlayerPromptCreated)>(HandlerArray, Data, ActivePromptHandles[Elem]);

		return ActivePromptHandles[Elem];
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to create a player prompt, but there is no active conversation!"));
	}

	static FYapPromptHandle NullHandle;
	NullHandle.Invalidate();
	return NullHandle;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnFinishedBroadcastingPrompts(const FYapData_PlayerPromptsReady& Data, const FGameplayTag& TypeGroup)
{
	if (Data.Conversation.IsValid())
	{
		auto* HandlerArray = FindConversationHandlerArray(TypeGroup);

		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptsReady, Execute_K2_ConversationPlayerPromptsReady)>(HandlerArray, Data);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to broadcast player prompts created, but there is no active conversation!"));
	}
}

// ------------------------------------------------------------------------------------------------

FYapSpeechHandle UYapSubsystem::RunSpeech(const FYapData_SpeechBegins& SpeechData, const FGameplayTag& TypeGroup)
{
	// TODO see if I can get rid of this shit
	FYapRunningFragment RunningFragment;

	FYapSpeechHandle SpeechHandle(RunningFragment);

	SpeechCompleteEvents.Add(SpeechHandle);
	FragmentCompleteEvents.Add(SpeechHandle);
	
	if (SpeechData.SpeechTime > 0)
	{
		FTimerHandle SpeechTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeechComplete, SpeechHandle);
		GetWorld()->GetTimerManager().SetTimer(SpeechTimerHandle, Delegate, SpeechData.SpeechTime, false);
		RunningFragment.SetSpeechTimerHandle(SpeechTimerHandle);
	}

	if (SpeechData.FragmentTime > 0)
	{
		FTimerHandle FragmentTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnFragmentComplete, SpeechHandle);
		GetWorld()->GetTimerManager().SetTimer(FragmentTimerHandle, Delegate, SpeechData.SpeechTime, false);
		RunningFragment.SetFragmentTimerHandle(FragmentTimerHandle);
	}
	
	if (SpeechData.Conversation.IsValid())
	{
		auto* HandlerArray = FindConversationHandlerArray(TypeGroup);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(HandlerArray, SpeechData, SpeechHandle);
	}
	else
	{
		auto* HandlerArray = FindFreeSpeechHandlerArray(TypeGroup);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(HandlerArray, SpeechData, SpeechHandle);
	}

	return SpeechHandle;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversation(UObject* ConversationOwner)
{
	for (FYapConversation& Conversation : Get()->ConversationQueue)
	{
		if (Conversation.GetOwner() == ConversationOwner)
		{
			return Conversation;
		}
	}

	return NullConversation;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversation(FYapConversationHandle Handle)
{
	for (FYapConversation& Conversation : Get()->ConversationQueue)
	{
		if (Conversation.GetGuid() == Handle.Guid)
		{
			return Conversation;
		}
	}

	return NullConversation;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversation(const FGameplayTag& ConversationName)
{
	for (FYapConversation& Conversation : Get()->ConversationQueue)
	{
		if (Conversation.GetConversationName() == ConversationName)
		{
			return Conversation;
		}
	}

	return NullConversation;	
}

// ------------------------------------------------------------------------------------------------

FGameplayTag UYapSubsystem::GetActiveConversation()
{
	UYapSubsystem* Subsystem = Get();
	
	if (Subsystem->ConversationQueue.IsEmpty())
	{
		return FGameplayTag::EmptyTag;
	}

	return Subsystem->ConversationQueue[Subsystem->ConversationQueue.Num() - 1].GetConversationName();
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::RunPrompt(const FYapPromptHandle& Handle)
{
	// TODO handle invalid handles gracefully

	UYapSubsystem* Subsystem = Get();
	
	// Broadcast to Yap systems
	Subsystem->OnPromptChosen.Broadcast(Subsystem, Handle);

	// Broadcast to game listeners
	FYapData_PlayerPromptChosen Data;
	
	auto* HandlerArray = FindFreeSpeechHandlerArray(Handle.GetTypeGroup());
	
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptChosen, Execute_K2_ConversationPlayerPromptChosen)>(HandlerArray, Data, Handle);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::SkipSpeech(const FYapSpeechHandle& Handle)
{
	// TODO handle invalid handles gracefully
	
	UYapSubsystem* Subsystem = Get();
	
	// Broadcast to Yap systems
	Subsystem->OnSpeechSkip.Broadcast(Subsystem, Handle);

	// Broadcast to game listeners
	// TODO
	
	return true;
}

// ------------------------------------------------------------------------------------------------

/*
FYapRunningFragment& UYapSubsystem::GetFragmentHandle(FYapSpeechHandle HandleRef)
{
	return Get()->RunningFragments.FindChecked(HandleRef);
}
*/

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

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddConversationHandlerArray(const FGameplayTag& TypeGroup)
{
	return Get()->ConversationHandlers.FindOrAdd(TypeGroup).Array;
}

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindConversationHandlerArray(const FGameplayTag& TypeGroup)
{
	FYapHandlersArray* Handlers = Get()->ConversationHandlers.Find(TypeGroup);

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
}

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddFreeSpeechHandlerArray(const FGameplayTag& TypeGroup)
{
	return Get()->FreeSpeechHandlers.FindOrAdd(TypeGroup).Array;
}

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindFreeSpeechHandlerArray(const FGameplayTag& TypeGroup)
{
	FYapHandlersArray* Handlers = Get()->FreeSpeechHandlers.Find(TypeGroup);

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
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

void UYapSubsystem::OnSpeechComplete(FYapSpeechHandle Handle)
{
	SpeechCompleteEvents[Handle].Broadcast(this, Handle);
	SpeechCompleteEvents.Remove(Handle);
}

void UYapSubsystem::OnFragmentComplete(FYapSpeechHandle Handle)
{
	FragmentCompleteEvents[Handle].Broadcast(this, Handle);
	FragmentCompleteEvents.Remove(Handle);
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#undef LOCTEXT_NAMESPACE