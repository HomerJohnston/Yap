// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "Yap/YapBroker.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/Interfaces/IYapConversationHandler.h"
//#include "Yap/YapRunningFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSquirrelNoise.h"
#include "Yap/Handles/YapPromptHandle.h"
//#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Interfaces/IYapFreeSpeechHandler.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "TimerManager.h"

#include "GameFramework/Character.h"
#include "Yap/YapCharacterManager.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_BROADCAST_EVT_TARGS(NAME, CPPFUNC, K2FUNC) U##NAME, I##NAME, &I##NAME::CPPFUNC, &I##NAME::K2FUNC

FName UYapSubsystem::Yap_UnnamedConvo("Yap.Conversation.__UnnamedConvo__");

bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;

// ------------------------------------------------------------------------------------------------

FYap__ActiveSpeechContainer* FYap__ActiveSpeechMap::AddSpeech(const FYapSpeechHandle& Handle, const FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner)
{
	if (AllSpeech.Contains(Handle))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to add speech handle more than once, ignoring! <%s>"), *Handle.ToString());
		return nullptr;
	}
	
	FYap__ActiveSpeechContainer& NewSpeechContainer = AllSpeech.Add(Handle);

	if (SpeakerID != NAME_None)
	{
		FYapSpeechHandlesArray& HandlesContainer = ContainersBySpeakerID.FindOrAdd(SpeakerID);
		HandlesContainer.Handles.Add(Handle);
		NewSpeechContainer.SpeakerID = SpeakerID;
	}

	if (IsValid(SpeechOwner))
	{
		FYapSpeechHandlesArray& HandlesContainer = ContainersByOwner.FindOrAdd(SpeechOwner);
		HandlesContainer.Handles.Add(Handle);
		NewSpeechContainer.SpeechOwner = SpeechOwner;
	}

	if (ConversationOwner)
	{
		if (FYapConversationHandle* ConversationHandle = ConversationsByOwner.Find(ConversationOwner))
		{
			NewSpeechContainer.ConversationHandle = *ConversationHandle;

			FYapConversation* Conversation = Conversations.Find(*ConversationHandle);

			check(Conversation);
			
			Conversation->AddRunningFragment(Handle);
		}
		else
		{
			UE_LOG(LogYap, Error, TEXT("AddSpeech was called with a conversation name but no conversation was found, this should never happen!"));
		}
	}

	return &NewSpeechContainer;
}

FYapConversation& FYap__ActiveSpeechMap::AddConversation(const FName ConversationName, UObject* ConversationOwner, FYapConversationHandle& ConversationHandle)
{
	ConversationHandle = FYapConversationHandle(ConversationOwner, ConversationName);

	if (FYapConversation* ExistingConversation = Conversations.Find(ConversationHandle))
	{
		return *ExistingConversation;
	}

	ConversationsByOwner.Add(ConversationOwner, ConversationHandle);
	
	FYapConversation& Conversation = Conversations.Add(ConversationHandle);
	Conversation.SetConversationHandle(ConversationHandle);
	Conversation.SetConversationName(UYapSubsystem::Yap_UnnamedConvo);

	return Conversation;
}

void FYap__ActiveSpeechMap::RemoveConversation(FYapConversationHandle ConversationHandle)
{
	int32 Removed = Conversations.Remove(ConversationHandle);
	
	if (Removed == 0)
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to remove conversation <%s> but converation was not found!"), *ConversationHandle.ToString());
	}

	for (auto It = ConversationsByOwner.CreateIterator(); It; ++It)
	{
		if (It.Value() == ConversationHandle)
		{
			It.RemoveCurrent();
		}
	}
}

FYapConversation* FYap__ActiveSpeechMap::FindConversationByOwner(const UObject* Owner)
{
	if (FYapConversationHandle* Handle = ConversationsByOwner.Find(Owner))
	{
		return Conversations.Find(*Handle);
	}

	return nullptr;
}

FYapConversationHandle* FYap__ActiveSpeechMap::FindConversationHandleByOwner(const UObject* Owner)
{
	return ConversationsByOwner.Find(Owner);
}

FYapConversation* FYap__ActiveSpeechMap::FindConversation(const FYapConversationHandle& ConversationHandle)
{
	return Conversations.Find(ConversationHandle);
}

FName FYap__ActiveSpeechMap::FindSpeakerID(const FYapSpeechHandle& Handle)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		return Container->SpeakerID;
	}
	
	UE_LOG(LogYap, Warning, TEXT("Tried to find speaker ID for handle but handle was not found! <%s>"), *Handle.ToString());
	return NAME_None;
}

FTimerHandle FYap__ActiveSpeechMap::FindTimerHandle(const FYapSpeechHandle& Handle)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		return Container->SpeechTimerHandle;
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to find timer handle for handle but handle was not found! <%s>"), *Handle.ToString());
	return {};
}

FYapSpeechEvent FYap__ActiveSpeechMap::FindSpeechFinishedEvent(const FYapSpeechHandle& Handle)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		return Container->OnSpeechFinish;
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to find speech finish event for handle but handle was not found! <%s>"), *Handle.ToString());
	return {};
}

FYapConversationHandle FYap__ActiveSpeechMap::FindSpeechConversationHandle(const FYapSpeechHandle& Handle)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		return Container->ConversationHandle;
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to find conversation for handle but handle was not found! <%s>"), *Handle.ToString());
	return {};
}

bool FYap__ActiveSpeechMap::IsSpeechRunning(const FYapSpeechHandle& Handle) const
{
	return AllSpeech.Contains(Handle);
}

void FYap__ActiveSpeechMap::RemoveSpeech(const FYapSpeechHandle& Handle)
{
	FYap__ActiveSpeechContainer Container;

	if (AllSpeech.RemoveAndCopyValue(Handle, Container))
	{		
		ContainersByOwner[Container.SpeechOwner].Handles.Remove(Handle);
		ContainersBySpeakerID[Container.SpeakerID].Handles.Remove(Handle);
		
		if (ContainersByOwner[Container.SpeechOwner].Handles.Num() == 0)
		{
			ContainersByOwner.Remove(Container.SpeechOwner);
		}

		if (ContainersBySpeakerID[Container.SpeakerID].Handles.Num() == 0)
		{
			ContainersBySpeakerID.Remove(Container.SpeakerID);
		}
		
		if (Container.ConversationHandle.IsValid())
		{
			if (FYapConversation* Conversation = Conversations.Find(Container.ConversationHandle))
			{
				Conversation->RemoveRunningSpeech(Handle);
			}
		}
	}
}

void FYap__ActiveSpeechMap::BindToSpeechFinish(const FYapSpeechHandle& Handle, const FYapSpeechEventDelegate& Delegate)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		Container->OnSpeechFinish.Add(Delegate);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to bind to speech finish but handle was not found! <%s>"), *Handle.ToString());
	}
}

void FYap__ActiveSpeechMap::UnbindToSpeechFinish(const FYapSpeechHandle& Handle, const FYapSpeechEventDelegate& Delegate)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		Container->OnSpeechFinish.Remove(Delegate);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to unbind from speech finish but handle was not found! <%s>"), *Handle.ToString());
	}
}

void FYap__ActiveSpeechMap::SetTimer(const FYapSpeechHandle& Handle, FTimerHandle TimerHandle)
{
	if (FYap__ActiveSpeechContainer* Container = AllSpeech.Find(Handle))
	{
		Container->SpeechTimerHandle = TimerHandle;
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to set timer for speech but handle was not found! <%s>"), *Handle.ToString());
	}
}

TArray<FYapSpeechHandle> FYap__ActiveSpeechMap::GetHandles(FName SpeakerID)
{
	if (FYapSpeechHandlesArray* Container = ContainersBySpeakerID.Find(SpeakerID))
	{
		return Container->Handles;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("No handles found for <%s>"), *SpeakerID.ToString());
	return { };
}

TArray<FYapSpeechHandle> FYap__ActiveSpeechMap::GetHandles(UObject* SpeechOwner)
{
	if (FYapSpeechHandlesArray* Container = ContainersByOwner.Find(SpeechOwner))
	{
		return Container->Handles;
	}

	UE_LOG(LogYap, VeryVerbose, TEXT("No handles found for <%s>"), *SpeechOwner->GetName());
	return { };
}

TArray<FYapSpeechHandle> FYap__ActiveSpeechMap::GetHandles(const FYapConversationHandle& ConversationHandle)
{
	// TODO
	unimplemented();
	return { };
}

// ================================================================================================

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

void UYapSubsystem::BindToSpeechFinish(const UObject* WorldContextObject, const FYapSpeechHandle& Handle, const FYapSpeechEventDelegate& Delegate)
{
	if (UYapSubsystem* Subsystem = Get(WorldContextObject))
	{
		Subsystem->ActiveSpeechMap.BindToSpeechFinish(Handle, Delegate);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Could not find UYapSubsystem!"));
	}
}

void UYapSubsystem::UnbindToSpeechFinish(const UObject* WorldContextObject, const FYapSpeechHandle& Handle, const FYapSpeechEventDelegate& Delegate)
{
	if (UYapSubsystem* Subsystem = Get(WorldContextObject))
	{
		Subsystem->ActiveSpeechMap.UnbindToSpeechFinish(Handle, Delegate);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Could not find UYapSubsystem!"));
	}
}

UYapCharacterManager& UYapSubsystem::GetCharacterManager(const UObject* WorldContextObject)
{
	UYapSubsystem* Subsystem = Get(WorldContextObject);

	check(Subsystem);
	
	if (!IsValid(Subsystem->CharacterManager))
	{
		Subsystem->CharacterManager = NewObject<UYapCharacterManager>(Subsystem);
		Subsystem->CharacterManager->Initialize();
	}

	return *Subsystem->CharacterManager;
}

void UYapSubsystem::RegisterConversationHandler(UObject* NewHandler, const FYapDialogueNodeClassType& NodeType)
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

void UYapSubsystem::UnregisterConversationHandler(UObject* HandlerToRemove, const FYapDialogueNodeClassType& NodeType)
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

void UYapSubsystem::RegisterFreeSpeechHandler(UObject* NewHandler, const FYapDialogueNodeClassType& NodeType)
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

void UYapSubsystem::UnregisterFreeSpeechHandler(UObject* HandlerToRemove, const FYapDialogueNodeClassType& NodeType)
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

UYapCharacterComponent* UYapSubsystem::FindCharacterComponent(const UWorld* World, const FName CharacterName)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = Get(World)->YapCharacterComponents.Find(CharacterName);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::IsNodeInConversation(const UFlowNode_YapDialogue* DialogueNode)
{
	UObject* Owner = DialogueNode->GetFlowAsset();

	if (UYapSubsystem* Subsystem = Get(DialogueNode))
	{
		return !!Subsystem->ActiveSpeechMap.FindConversationByOwner(Owner);
	}

	return false;
}

bool UYapSubsystem::IsSpeechInConversation(const UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	if (UYapSubsystem* Subsystem = Get(WorldContext))
	{
		return Subsystem->ActiveSpeechMap.FindSpeechConversationHandle(Handle).IsValid();
	}

	UE_LOG(LogYap, Error, TEXT("Could not get Yap Subsystem!"));

	return false;
}

/*
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
*/

UYapBroker& UYapSubsystem::GetBroker()
{
	if (!IsValid(Broker))
	{
		TSubclassOf<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass();
		
		Broker = NewObject<UYapBroker>(this, BrokerClass);
	}

	return *Broker;
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapSubsystem::GetCurrentMaturitySetting(const UWorld* World)
{
	if (!IsValid(World))
	{
		return EYapMaturitySetting::Mature;
	}
	
	const UYapBroker& Broker = UYapBroker::Get(World);

	EYapMaturitySetting MaturitySetting = Broker.GetMaturitySetting();

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
	if (UFlowNode_YapDialogue** DialoguePtr = TaggedFragments.Find(FragmentTag))
	{
		return (*DialoguePtr)->FindTaggedFragment(FragmentTag);
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

FYapConversation& UYapSubsystem::OpenConversation(FName ConversationName, UObject* ConversationOwner)
{
	if (!ConversationName.IsValid())
	{
		ConversationName = Yap_UnnamedConvo;
	}

	FYapConversationHandle NewHandle;
	FYapConversation& NewConversation = ActiveSpeechMap.AddConversation(ConversationName, ConversationOwner, NewHandle);

	ConversationQueue.EmplaceAt(0, NewHandle);
	
	if (ConversationQueue.Num() == 1)
	{
		StartOpeningConversation(NewConversation);
	}

	return NewConversation;
}

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::CloseConversation(FYapConversationHandle& Handle)
{
	// TODO clean this up?
	if (FYapConversation* ConversationPtr = ActiveSpeechMap.FindConversation(Handle))
	{
		check(ConversationQueue.Contains(Handle));

		return StartClosingConversation(Handle);
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to close conversation handle {%s} but it did not exist!"), *Handle.ToString());
	
	return EYapConversationState::Undefined;
}

// ------------------------------------------------------------------------------------------------

/*
EYapConversationState UYapSubsystem::CloseConversation(const FGameplayTag& ConversationName)
{
	if (FYapConversationHandle* ConversationHandle = ActiveSpeechMap.ConversationHandles.Find(ConversationName.GetTagName()))
	{
		return CloseConversation(*ConversationHandle);
	}

	UE_LOG(LogYap, Warning, TEXT("Tried to close conversation named {%s} but it did not exist!"), *ConversationName.ToString());

	return EYapConversationState::Undefined;
}
*/

// ------------------------------------------------------------------------------------------------

EYapConversationState UYapSubsystem::CloseConversation(const UObject* Owner)
{
	if (FYapConversationHandle* ConversationHandle = ActiveSpeechMap.FindConversationHandleByOwner(Owner))
	{
		return CloseConversation(*ConversationHandle);
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
	
	if (FYapConversation* ConversationPtr = ActiveSpeechMap.FindConversation(Handle))
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
	if (FYapConversation* ConversationPtr = ActiveSpeechMap.FindConversation(Handle))
	{
		ConversationPtr->StartClosing(this);

		if (ConversationPtr->GetState() == EYapConversationState::Closed)
		{
			ConversationQueue.Remove(Handle);

			ActiveSpeechMap.RemoveConversation(Handle);
			
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
	
	ActiveSpeechMap.RemoveConversation(Handle);
	
	StartNextQueuedConversation();
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle UYapSubsystem::BroadcastPrompt(const FYapData_PlayerPromptCreated& Data, FYapDialogueNodeClassType NodeType)
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

void UYapSubsystem::OnFinishedBroadcastingPrompts(const FYapData_PlayerPromptsReady& Data, FYapDialogueNodeClassType NodeType)
{
	auto* HandlerArray = FindConversationHandlerArray(NodeType);

	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptsReady, Execute_K2_ConversationPlayerPromptsReady)>(HandlerArray, Data);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RunSpeech(const FYapData_SpeechBegins& SpeechData, FYapDialogueNodeClassType NodeType, const FYapSpeechHandle& SpeechHandle)
{
	TArray<FYapSpeechHandle> ActiveSpeechHandle = ActiveSpeechMap.GetHandles(SpeechData.SpeakerID);

	UFlowNode_YapDialogue* CDO = NodeType.Get()->GetDefaultObject<UFlowNode_YapDialogue>();
	const UYapNodeConfig& Config = CDO->GetNodeConfig();

	if (!Config.DialoguePlayback.bPermitOverlappingSpeech)
	{
		for (int32 i = ActiveSpeechHandle.Num() - 1; i >= 0; --i)
		{
			if (ActiveSpeechHandle[i] == SpeechHandle)
			{
				continue;
			}
			
			OnSpeechComplete(ActiveSpeechHandle[i], true);
		}
	}

	// TODO should SpeechData contain the conversation handle instead of the name?
	if (SpeechData.Conversation != NAME_None)
	{
		FYapConversationHandle ConversationHandle = ActiveSpeechMap.FindSpeechConversationHandle(SpeechHandle);
		
		if (ConversationHandle.IsValid())
		{
			if (FYapSpeechHandlesArray* FragileHandles = FragileSpeechHandles.Find(ConversationHandle))
			{
				TArray<FYapSpeechHandle> FragileHandlesCopy = FragileHandles->Handles;

				for (FYapSpeechHandle& Handle : FragileHandlesCopy)
				{
					if (ActiveSpeechMap.IsSpeechRunning(Handle))
					{
						CancelSpeech(this, Handle);
					}
				}

				FragileSpeechHandles.Remove(ConversationHandle);
			}
		}
		
		auto* HandlerArray = FindConversationHandlerArray(NodeType);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(HandlerArray, SpeechData, SpeechHandle);
	}
	else
	{
		auto* HandlerArray = FindFreeSpeechHandlerArray(NodeType);
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(HandlerArray, SpeechData, SpeechHandle);
	}

	if (SpeechData.SpeechTime > 0)
	{
		FTimerHandle SpeechTimerHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeechComplete, SpeechHandle, true, EYapSpeechCompleteResult::Normal);
		GetWorld()->GetTimerManager().SetTimer(SpeechTimerHandle, Delegate, SpeechData.SpeechTime, false);

		ActiveSpeechMap.SetTimer(SpeechHandle, SpeechTimerHandle);
	}
	else
	{
		OnSpeechComplete(SpeechHandle, true);
	}
}

void UYapSubsystem::MarkConversationSpeechAsFragile(const FYapSpeechHandle& Handle)
{
	FYapConversationHandle ConversationHandle = ActiveSpeechMap.FindSpeechConversationHandle(Handle);

	FYapSpeechHandlesArray& FragileHandles = FragileSpeechHandles.FindOrAdd(ConversationHandle);

	FragileHandles.Handles.Add(Handle);
}

// ------------------------------------------------------------------------------------------------

FYapConversation* UYapSubsystem::GetConversationByOwner(const UObject* WorldContext, UObject* Owner)
{
	UYapSubsystem* Subsystem = Get(WorldContext);
	
	return Subsystem->ActiveSpeechMap.FindConversationByOwner(Owner);
}

// ------------------------------------------------------------------------------------------------

FYapConversation* UYapSubsystem::GetConversationByHandle(const UObject* WorldContext, const FYapConversationHandle& Handle)
{
	UYapSubsystem* Subsystem = Get(WorldContext);

	return Subsystem->ActiveSpeechMap.FindConversation(Handle);
}

// ------------------------------------------------------------------------------------------------

/*
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
*/

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
		const FYapConversation* Conversation = GetConversationByHandle(WorldContext, *ConversationHandle);

		if (Conversation)
		{
			auto* HandlerArray = Subsystem->FindConversationHandlerArray(Conversation->GetNodeType());
		
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

bool UYapSubsystem::CancelSpeech(UObject* WorldContext, FYapSpeechHandle& Handle)
{
	if (!IsValid(WorldContext))
	{
		UE_LOG(LogYap, Warning, TEXT("Subsystem: CancelSpeech failed - world context was invalid"));
		return false;
	}

	UYapSubsystem* Subsystem = Get(WorldContext);

	if (Subsystem)
	{
		return Subsystem->EndSpeech(Handle, EYapSpeechCompleteResult::Cancelled);
	}
	
	UE_LOG(LogYap, Error, TEXT("Subsystem: CancelSpeech - SUBSYSTEM NOT FOUND"));
	
	return false;
}

bool UYapSubsystem::CancelSpeech(UObject* SpeechOwner)
{
	UYapSubsystem* Subsystem = Get(SpeechOwner);

	if (Subsystem)
	{
		TArray<FYapSpeechHandle> Handles = Subsystem->ActiveSpeechMap.GetHandles(SpeechOwner);

		for (FYapSpeechHandle& Handle : Handles)
		{
			CancelSpeech(SpeechOwner, Handle);
		}
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Failed to find subsystem - did you pass an invalid speech owner into Cancel Speech?"));	
	}

	return false;
}

bool UYapSubsystem::AdvanceSpeech(UObject* WorldContext, FYapSpeechHandle& Handle)
{
	if (!IsValid(WorldContext))
	{
		UE_LOG(LogYap, Warning, TEXT("Subsystem: AdvanceSpeech failed - world context was invalid"));
		return false;
	}

	UYapSubsystem* Subsystem = Get(WorldContext);

	if (Subsystem)
	{
		return Subsystem->EndSpeech(Handle, EYapSpeechCompleteResult::Advanced);
	}
	
	UE_LOG(LogYap, Error, TEXT("Subsystem: CancelSpeech - SUBSYSTEM NOT FOUND"));
	
	return false;
}

bool UYapSubsystem::AdvanceSpeech(UObject* SpeechOwner)
{
	UYapSubsystem* Subsystem = Get(SpeechOwner);

	if (Subsystem)
	{
		TArray<FYapSpeechHandle> Handles = Subsystem->ActiveSpeechMap.GetHandles(SpeechOwner);

		for (FYapSpeechHandle& Handle : Handles)
		{
			AdvanceSpeech(SpeechOwner, Handle);
		}
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Failed to find subsystem - did you pass an invalid speech owner into Cancel Speech?"));	
	}

	return false;
}

bool UYapSubsystem::EndSpeech(FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogYap, Display, TEXT("Subsystem: CancelSpeech failed - speech handle was invalid"));
		return false;
	}
	
	UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: CancelSpeech {%s}"), *Handle.ToString());

	FYapSpeechHandle HandleCopy = Handle;
	Handle.Invalidate();
	
	FTimerHandle TimerHandle = ActiveSpeechMap.FindTimerHandle(HandleCopy);
	
	if (TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

		FYapSpeechEvent Evt;

		return EmitSpeechResult(HandleCopy, Result);
	}

	UE_LOG(LogYap, Display, TEXT("Subsystem: CancelSpeech [%s] ignored - SpeechTimers array did not contain an entry for this handle. Invalidating this handle."), *Handle.ToString());
	
	return false;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::AdvanceConversation(UObject* Instigator, const FYapConversationHandle& ConversationHandle)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation START [%s]"), *ConversationHandle.ToString());

	if (!ConversationHandle.IsValid())
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation {%s} ignored - handle was invalid"), *ConversationHandle.ToString());
	}
	
	UYapSubsystem* Subsystem = Get(Instigator);

	FYapConversation* ConversationPtr = Subsystem->ActiveSpeechMap.FindConversation(ConversationHandle);

	if (!ConversationPtr)
	{
		UE_LOG(LogYap, Warning, TEXT("UYapSubsystem::AdvanceConversation - could not find conversation for handle {%s}"), *ConversationHandle.ToString());
		return;
	}
	
	TArray<FYapSpeechHandle> RunningFragments = ConversationPtr->GetRunningFragments();
	
	//UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation CALLING ONADVANCECONVERSATIONDELEGATE [%s]"), *ConversationHandle.ToString());
	// Broadcast to Yap systems; in dialogue nodes, this will kill any running paddings
	Subsystem->OnAdvanceConversationDelegate.Broadcast(Instigator, ConversationHandle);
	
	// Finish all running speeches
	for (const FYapSpeechHandle& SpeechHandle : RunningFragments)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation CALLING ONSPEECHCOMPLETE [%s]"), *ConversationHandle.ToString());
		Subsystem->OnSpeechComplete(SpeechHandle, true, EYapSpeechCompleteResult::Advanced);
	}
	
	UE_LOG(LogYap, VeryVerbose, TEXT("Subsystem: AdvanceConversation FINISH [%s]"), *ConversationHandle.ToString());
}

bool UYapSubsystem::EmitSpeechResult(const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result)
{	
	FYapSpeechEvent Evt = ActiveSpeechMap.FindSpeechFinishedEvent(Handle);

	FTimerHandle Timer = ActiveSpeechMap.FindTimerHandle(Handle);

	ActiveSpeechMap.RemoveSpeech(Handle);
	
	Evt.Broadcast(this, Handle, Result);
	
	if (Timer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer);
	}
	
	return true;
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

	YapCharacterComponents.Add(YapCharacterComponent->GetCharacterID(), YapCharacterComponent);
	
	RegisteredYapCharacterActors.Add(Actor);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	YapCharacterComponents.Remove(YapCharacterComponent->GetCharacterID());
	RegisteredYapCharacterActors.Remove(Actor);
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddConversationHandlerArray(FYapDialogueNodeClassType NodeType)
{	
	return ConversationHandlers.FindOrAdd(NodeType).Array;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindConversationHandlerArray(FYapDialogueNodeClassType NodeType)
{
	FYapHandlersArray* Handlers = ConversationHandlers.Find(NodeType.Get());

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>& UYapSubsystem::FindOrAddFreeSpeechHandlerArray(FYapDialogueNodeClassType NodeType)
{
	return FreeSpeechHandlers.FindOrAdd(NodeType).Array;
}

// ------------------------------------------------------------------------------------------------

TArray<TObjectPtr<UObject>>* UYapSubsystem::FindFreeSpeechHandlerArray(FYapDialogueNodeClassType NodeType)
{
	FYapHandlersArray* Handlers = FreeSpeechHandlers.Find(NodeType.Get());

	if (Handlers)
	{
		return &Handlers->Array;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

FYapSpeechHandle UYapSubsystem::GetNewSpeechHandle(FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner)
{
	return GetNewSpeechHandle(FGuid::NewGuid(), SpeakerID, SpeechOwner, ConversationOwner);
}

FYapSpeechHandle UYapSubsystem::GetNewSpeechHandle(FGuid Guid, FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner)
{
	FYapSpeechHandle NewHandle(GetWorld(), Guid);

	ActiveSpeechMap.AddSpeech(NewHandle, SpeakerID, SpeechOwner, ConversationOwner);

	return NewHandle;
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
		Broker->BeginPlay();
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnSpeechComplete(FYapSpeechHandle Handle, bool bBroadcast, EYapSpeechCompleteResult SpeechResult)
{
	UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete entering {%s}"), *GetName(), *Handle.ToString());

	if (SpeechResult == EYapSpeechCompleteResult::Undefined)
	{
		SpeechResult = EYapSpeechCompleteResult::Normal;
	}

	if (bBroadcast)
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: OnSpeechComplete {%s}"), *GetName(), *Handle.ToString());

		if (!EmitSpeechResult(Handle, SpeechResult))
		{
			UE_LOG(LogYap, Warning, TEXT("Handle was not registered into SpeechCompleteEvents! Can't broadcast Complete event! %s"), *Handle.ToString());
		}
	}
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return true;
	//return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE