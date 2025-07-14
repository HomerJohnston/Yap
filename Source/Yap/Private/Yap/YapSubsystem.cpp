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
#include "Yap/YapSquirrelNoise.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Interfaces/IYapFreeSpeechHandler.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameFramework/Character.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_BROADCAST_EVT_TARGS(NAME, CPPFUNC, K2FUNC) U##NAME, I##NAME, &I##NAME::CPPFUNC, &I##NAME::K2FUNC

UE_DEFINE_GAMEPLAY_TAG_STATIC(Yap_UnnamedConvo, "Yap.Conversation.__UnnamedConvo__");

bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;
FYapConversation UYapSubsystem::NullConversation;

// ------------------------------------------------------------------------------------------------

UYapSubsystem::UYapSubsystem()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	TMap<TWeakObjectPtr<AActor>, int> TestMap;
	
	const ACharacter* CharacterPtr = nullptr;

	if (TestMap.Contains(CharacterPtr))
	{
		
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterConversationHandler(UObject* NewHandler, FYapDialogueNodeType NodeType)
{
	if (!IsValid(NewHandler))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register null or invalid conversation handler, ignoring!"));
		return;
	}

	if (NewHandler->Implements<UYapConversationHandler>())
	{
		Get(NewHandler->GetWorld())->FindOrAddConversationHandlerArray(NodeType).AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but object does not implement the IYapConversationHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterConversationHandler(UObject* HandlerToRemove, FYapDialogueNodeType NodeType)
{	
	if (!IsValid(HandlerToRemove))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to unregister null or invalid conversation handler, ignoring!"));
		return;
	}
	
	auto* Array = Get(HandlerToRemove->GetWorld())->FindConversationHandlerArray(NodeType);

	if (!Array)
	{
		return;
	}
	
	Array->Remove(HandlerToRemove);

	
	if (Array->IsEmpty())
	{
		Get(HandlerToRemove->GetWorld())->ConversationHandlers.Remove(NodeType.Get());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterFreeSpeechHandler(UObject* NewHandler, FYapDialogueNodeType NodeType)
{
	if (!IsValid(NewHandler))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register null or invalid free speech handler, ignoring!"));
		return;
	}
	
	if (NewHandler->Implements<UYapFreeSpeechHandler>())
	{
		Get(NewHandler->GetWorld())->FindOrAddFreeSpeechHandlerArray(NodeType).AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a free speech handler, but object does not implement the IYapFreeSpeechHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterFreeSpeechHandler(UObject* HandlerToRemove, FYapDialogueNodeType NodeType)
{
	if (!IsValid(HandlerToRemove))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to unregister null or invalid free speech handler, ignoring!"));
		return;
	}
	
	auto* Array = Get(HandlerToRemove->GetWorld())->FindFreeSpeechHandlerArray(NodeType);

	if (!Array)
	{
		return;
	}

	Array->Remove(HandlerToRemove);
	
	if (Array->IsEmpty())
	{
		// For some reason my implicit type conversion doesn't work for TArray funcs
		Get(HandlerToRemove->GetWorld())->FreeSpeechHandlers.Remove(NodeType.Get());
	}
}

// ------------------------------------------------------------------------------------------------

UYapCharacterComponent* UYapSubsystem::FindCharacterComponent(UWorld* World, FGameplayTag CharacterTag)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = Get(World)->YapCharacterComponents.Find(CharacterTag);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
const UYapBroker& UYapSubsystem::GetBroker_Editor()
{
	// During play, use GEditor to get the broker normally
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		return GetBroker(GEditor->PlayWorld);
	}

	// Outside of play, get a default instance of the broker class
	TSubclassOf<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass();

	return *BrokerClass.GetDefaultObject();
}
#endif

UYapBroker& UYapSubsystem::GetBroker(UObject* WorldContext)
{
	UYapSubsystem* Instance = Get(WorldContext);
	
	if (!IsValid(Instance->Broker))
	{
		TSubclassOf<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass();
		
		Instance->Broker = NewObject<UYapBroker>(Instance, BrokerClass);
	}

	return *Instance->Broker;
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapSubsystem::GetCurrentMaturitySetting(UWorld* World)
{
	EYapMaturitySetting MaturitySetting;

	if (!IsValid(World))
	{
		return EYapMaturitySetting::Mature;
	}
	
	UYapBroker& Broker = GetBroker(World);

	MaturitySetting = Broker.GetMaturitySetting();

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

FYapConversation& UYapSubsystem::OpenConversation(FGameplayTag ConversationName, UObject* ConversationOwner)
{
	if (!ConversationName.IsValid())
	{
		ConversationName = Yap_UnnamedConvo;
	}
	
	// Check if this conversation already exists, if so just return it. Yap assumes there will be a few conversations open at a time (at most!), so iteration is cheap.
	for (auto& [Handle, Conversation] : Conversations)
	{
		if (Conversation.GetConversationName() == ConversationName && Conversation.GetOwner() == ConversationOwner)
		{
			UE_LOG(LogYap, Warning, TEXT("Tried to start a new conversation {%s} owned by {%s} but conversation was already open or in queue!"), *ConversationName.ToString(), *ConversationOwner->GetName());
			return Conversation;
		}
	}

	FYapConversationHandle NewHandle = ConversationQueue.EmplaceAt_GetRef(0);
	FYapConversation& NewConversation = Conversations.Emplace(NewHandle, {ConversationName, ConversationOwner, NewHandle});
	
	if (ConversationQueue.Num() == 1)
	{
		StartOpeningConversation(NewConversation);
	}

	return NewConversation;
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::CloseConversation(FYapConversationHandle& Handle)
{
	FYapConversation* ConversationPtr = Conversations.Find(Handle);

	if (ConversationPtr)
	{
		check(ConversationQueue.Contains(Handle));

		return StartClosingConversation(Handle);
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to close conversation handle {%s} but it did not exist!"), *Handle.ToString());
	
	return EYapConversationState::Undefined;
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::CloseConversation(const FGameplayTag& ConversationName)
{
	for (auto& [Handle, Conversation] : Conversations)
	{
		if (Conversation.GetConversationName() == ConversationName)
		{
			return CloseConversation(Handle);
		}
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to close conversation named {%s} but it did not exist!"), *ConversationName.ToString());

	return EYapConversationState::Undefined;
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::CloseConversation(const UObject* Owner)
{
	for (auto& [Handle, Conversation] : Conversations)
	{
		if (Conversation.GetOwner() == Owner)
		{
			return CloseConversation(Handle);
		}
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to close conversation for owner {%s} but it did not exist!"), *Owner->GetName());

	return EYapConversationState::Undefined;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::StartOpeningConversation(const FYapConversationHandle& Handle)
{
	if (GetActiveConversation() == Handle)
	{
		UE_LOG(LogYap, Display, TEXT("Tried to start a new conversation but conversation was already active!"));
		return;
	}
	
	FYapConversation* ConversationPtr = Conversations.Find(Handle);

	if (ConversationPtr)
	{
		(void) StartOpeningConversation(*ConversationPtr);
	}
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::StartOpeningConversation(FYapConversation& Conversation)
{
	if (Conversation.GetState() == EYapConversationState::Open || Conversation.GetState() == EYapConversationState::Opening)
	{
		UE_LOG(LogYap, Display, TEXT("Tried to start a new conversation but conversation was already active!"));
		return false;
	}
	
	UE_LOG(LogYap, Display, TEXT("Subsystem: Starting conversation %s"), *Conversation.GetConversationName().ToString());
	
	FYapData_ConversationOpened Data;
	Data.Conversation = Conversation.GetConversationName();

	auto* HandlerArray = FindConversationHandlerArray(Conversation.GetNodeType());

	// Game code may add opening locks to the conversation here
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationOpened, Execute_K2_ConversationOpened)>(HandlerArray, Data, Conversation.GetHandle());

	Conversation.StartOpening(this);

	return true;
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::StartClosingConversation(FYapConversationHandle& Handle)
{
	FYapConversation* ConversationPtr = Conversations.Find(Handle);

	if (ConversationPtr)
	{
		ConversationPtr->StartClosing(this);

		if (ConversationPtr->GetState() == EYapConversationState::Closed)
		{
			ConversationQueue.Remove(Handle);
			
			Conversations.Remove(Handle);

			Handle.Invalidate();
			
			StartNextQueuedConversation();

			return EYapConversationState::Closed;
		}
		else
		{
			ConversationPtr->OnConversationClosed.AddDynamic(this, &UYapSubsystem::OnActiveConversationClosed);
	
			return EYapConversationState::Closing;	
		}
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Failed to close conversation {s}, conversation does not exist!"));
		return EYapConversationState::Undefined;
	}
}

const FYapConversationHandle& UYapSubsystem::GetActiveConversation()
{
	int32 Index = ConversationQueue.Num() - 1;

	if (Index == INDEX_NONE)
	{
		return FYapConversationHandle::GetNullHandle();
	}
		
	return ConversationQueue[ConversationQueue.Num() - 1];
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

void UYapSubsystem::OnActiveConversationClosed(UObject* Instigator, FYapConversationHandle Handle)
{	
	ConversationQueue.Remove(Handle);
	
	Conversations.Remove(Handle);
	
	StartNextQueuedConversation();
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle UYapSubsystem::BroadcastPrompt(const FYapData_PlayerPromptCreated& Data, FYapDialogueNodeType NodeType)
{
	FYapPromptHandle Handle(NodeType);

	const FYapConversationHandle& ConversationHandle = Data.Conversation;
	
	if (!ConversationHandle.IsValid())
	{
		UE_LOG(LogYap, Error, TEXT("Tried to create a player prompt, but there is no active conversation!"));
		
		static FYapPromptHandle NullHandle;
		NullHandle.Invalidate();
		return NullHandle;
	}
	
	PromptHandleConversationTags.Add(Handle, ConversationHandle);

	auto* HandlerArray = FindConversationHandlerArray(NodeType);

	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptCreated, Execute_K2_ConversationPlayerPromptCreated)>(HandlerArray, Data, Handle);

	return Handle;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnFinishedBroadcastingPrompts(const FYapData_PlayerPromptsReady& Data, FYapDialogueNodeType NodeType)
{
	auto* HandlerArray = FindConversationHandlerArray(NodeType);

	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptsReady, Execute_K2_ConversationPlayerPromptsReady)>(HandlerArray, Data);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RunSpeech(const FYapData_SpeechBegins& SpeechData, FYapDialogueNodeType NodeType, FYapSpeechHandle& Handle)
{
	// TODO should SpeechData contain the conversation handle instead of the name?
	if (SpeechData.Conversation.IsValid())
	{
		for (auto& [ConversationHandle, Conversation] : Conversations)
		{
			if (Conversation.GetConversationName() == SpeechData.Conversation)
			{
				SpeechConversationMapping.Add(Handle, ConversationHandle);
				Conversation.AddRunningFragment(Handle);
			}
		}
		
		auto* HandlerArray = FindConversationHandlerArray(NodeType);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(HandlerArray, SpeechData, Handle);
	}
	else
	{
		auto* HandlerArray = FindFreeSpeechHandlerArray(NodeType);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(HandlerArray, SpeechData, Handle);
	}

	if (SpeechData.SpeechTime > 0)
	{
		FTimerHandle SpeechTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeechComplete, Handle);
		GetWorld()->GetTimerManager().SetTimer(SpeechTimerHandle, Delegate, SpeechData.SpeechTime, false);

		SpeechTimers.Add(Handle, SpeechTimerHandle);
	}
	else
	{
		OnSpeechComplete(Handle);
	}
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversationByOwner(UObject* WorldContext, UObject* Owner)
{
	for (auto& [Handle, Conversation] : Get(WorldContext->GetWorld())->Conversations)
	{
		if (Conversation.GetOwner() == Owner)
		{
			return Conversation;
		}
	}

	return NullConversation;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversationByHandle(UObject* WorldContext, const FYapConversationHandle& Handle)
{
	FYapConversation* ConversationPtr = Get(WorldContext->GetWorld())->Conversations.Find(Handle);

	if (ConversationPtr)
	{
		return *ConversationPtr;
	}

	return NullConversation;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::GetConversationByName(const FGameplayTag& ConversationName, UObject* Owner)
{
	for (auto& [Handle, Conversation] : Get(Owner->GetWorld())->Conversations)
	{
		if (Conversation.GetOwner() == Owner && Conversation.GetConversationName() == ConversationName)
		{
			return Conversation;
		}
	}

	return NullConversation;	
}

// ------------------------------------------------------------------------------------------------

FGameplayTag UYapSubsystem::GetActiveConversationName(UWorld* World)
{
	UYapSubsystem* Subsystem = Get(World);
	
	if (Subsystem->ConversationQueue.IsEmpty())
	{
		return FGameplayTag::EmptyTag;
	}

	int32 ActiveIndex = Subsystem->ConversationQueue.Num() - 1;
	
	const FYapConversationHandle& ActiveHandle = Subsystem->ConversationQueue[ActiveIndex]; 

	const FYapConversation* ActiveConversationPtr = Subsystem->Conversations.Find(ActiveHandle); 

	if (ActiveConversationPtr)
	{
		return ActiveConversationPtr->GetConversationName();
	}
	else
	{
		return FGameplayTag::EmptyTag;
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RunPrompt(UObject* WorldContext, const FYapPromptHandle& Handle)
{
	if (!IsValid(WorldContext))
	{
		UE_LOG(LogYap, Error, TEXT("Tried to call UYapSubsystem::RunPrompt with a null world context, ignoring!"));
	}
	
	if (!Handle.IsValid())
	{
		UE_LOG(LogYap, Error, TEXT("Tried to call UYapSubsystem::RunPrompt with a null handle, ignoring!"));
		return;
	}

	

	UYapSubsystem* Subsystem = Get(WorldContext);
	
	// Broadcast to game listeners
	FYapData_PlayerPromptChosen Data;

	FYapConversationHandle* ConversationHandle = Subsystem->PromptHandleConversationTags.Find(Handle);

	if (ConversationHandle)
	{
		const FYapConversation& Conversation = GetConversationByHandle(WorldContext, *ConversationHandle);

		if (!Conversation.IsNull())
		{
			auto* HandlerArray = Subsystem->FindConversationHandlerArray(Conversation.GetNodeType());
		
			BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptChosen, Execute_K2_ConversationPlayerPromptChosen)>(HandlerArray, Data, Handle);
		}
		else
		{
			// TODO error handling!
		}
	}
	else
	{
		auto* HandlerArray = Subsystem->FindFreeSpeechHandlerArray(Handle.GetNodeType());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptChosen, Execute_K2_ConversationPlayerPromptChosen)>(HandlerArray, Data, Handle);
	}

	// Broadcast to Yap systems
	Subsystem->OnPromptChosen.Broadcast(Subsystem, Handle);
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::CancelSpeech(UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	if (!IsValid(WorldContext))
	{
		UE_LOG(LogYap, Warning, TEXT("Subsystem: CancelSpeech failed - world context was invalid"));
		return false;
	}

	if (!Handle.IsValid())
	{
		UE_LOG(LogYap, Display, TEXT("Subsystem: CancelSpeech failed - speech handle was invalid"));
		return false;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: CancelSpeech {%s}"), *Handle.ToString());
	
	UYapSubsystem* Subsystem = Get(WorldContext);

	FYapConversationHandle* ConversationHandlePtr = Subsystem->SpeechConversationMapping.Find(Handle);

	/*
	if (ConversationHandlePtr)
	{
		//Subsystem->AdvanceConversation(Subsystem, *ConversationHandlePtr);
		return true;
	}
	*/
	
	if (FTimerHandle* TimerHandle = Subsystem->SpeechTimers.Find(Handle))
	{
		WorldContext->GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);

		// Broadcast to Yap systems; in dialogue nodes, this will kill any running paddings
		Subsystem->OnCancelDelegate.Broadcast(Subsystem, Handle);

		Subsystem->OnSpeechComplete(Handle);
	
		return true;
	}

	UE_LOG(LogYap, Warning, TEXT("Subsystem: CancelSpeech [%s] ignored - SpeechTimers array did not contain an entry for this handle"), *Handle.ToString());
	
	return false;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::AdvanceConversation(UObject* Instigator, const FYapConversationHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation [%s]"), *Handle.ToString());

	if (!Handle.IsValid())
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation {%s} ignored - handle was invalid"), *Handle.ToString());
	}
	
	UYapSubsystem* Subsystem = Get(Instigator);

	FYapConversation* ConversationPtr = Subsystem->Conversations.Find(Handle);

	if (!ConversationPtr)
	{
		UE_LOG(LogYap, Warning, TEXT("UYapSubsystem::AdvanceConversation - could not find conversation for handle {%s}"), *Handle.ToString());
		return;
	}
	TArray<FYapSpeechHandle> RunningFragments = ConversationPtr->GetRunningFragments();

	// Finish all running speeches
	for (const FYapSpeechHandle& SpeechHandle : RunningFragments)
	{
		Subsystem->OnSpeechComplete(SpeechHandle);
	}
	
	// Broadcast to Yap systems; in dialogue nodes, this will kill any running paddings
	Subsystem->OnAdvanceConversationDelegate.Broadcast(Instigator, Handle);
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

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	YapCharacterComponents.Remove(YapCharacterComponent->GetCharacterTag());
	RegisteredYapCharacterActors.Remove(Actor);
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddConversationHandlerArray(FYapDialogueNodeType NodeType)
{	
	return ConversationHandlers.FindOrAdd(NodeType).Array;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindConversationHandlerArray(FYapDialogueNodeType NodeType)
{
	FYapHandlersArray* Handlers = ConversationHandlers.Find(NodeType.Get());

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddFreeSpeechHandlerArray(FYapDialogueNodeType NodeType)
{
	return FreeSpeechHandlers.FindOrAdd(NodeType).Array;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindFreeSpeechHandlerArray(FYapDialogueNodeType NodeType)
{
	FYapHandlersArray* Handlers = FreeSpeechHandlers.Find(NodeType.Get());

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

FYapSpeechHandle UYapSubsystem::GetNewSpeechHandle(FGuid Guid)
{
	FYapSpeechHandle NewHandle(GetWorld(), Guid);
	
	SpeechCompleteEvents.Add(NewHandle);

	return NewHandle;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterSpeechHandle(FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("RegisterSpeechHandle {%s}"), *Handle.ToString());
	SpeechCompleteEvents.Add(Handle);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterSpeechHandle(FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("UnregisterSpeechHandle {%s}"), *Handle.ToString());
	SpeechCompleteEvents.Remove(Handle);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Broker = NewObject<UYapBroker>(this, UYapProjectSettings::GetBrokerClass());

	bGetGameMaturitySettingWarningIssued = false;

	NoiseGenerator = NewObject<UYapSquirrel>(this);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Deinitialize()
{
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
	auto* Delegate = SpeechCompleteEvents.Find(Handle);

	if (Delegate)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete {%s}"), *GetName(), *Handle.ToString());

		FYapSpeechEvent Evt;
		SpeechCompleteEvents.RemoveAndCopyValue(Handle, Evt);
		Evt.Broadcast(this, Handle);
		
		//This more rudimentary method was throwing an ensure in MTAccessDetector.h destructor, Line ~502. I have no idea why, though.
		//SpeechCompleteEvents[Handle].Broadcast(this, Handle);
		//SpeechCompleteEvents.Remove(Handle);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Handle was not registered into SpeechCompleteEvents! %s"), *Handle.ToString());
	}
	
	FTimerHandle* Timer = SpeechTimers.Find(Handle);

	if (Timer)
	{
		GetWorld()->GetTimerManager().ClearTimer(*Timer);
	}

	SpeechTimers.Remove(Handle);

	FYapConversationHandle* ConversationHandle = SpeechConversationMapping.Find(Handle);

	if (ConversationHandle && ConversationHandle->IsValid())
	{
		FYapConversation* ConversationPtr = Conversations.Find(*ConversationHandle);

		if (ConversationPtr)
		{
			ConversationPtr->RemoveRunningFragment(Handle);
		}
		
		SpeechConversationMapping.Remove(Handle);
	}
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE