// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "YapConversation.h"
#include "YapCharacterComponent.h"
#include "YapBroker.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Enums/YapMaturitySetting.h"
#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapDataStructures.h"

#include "YapSubsystem.generated.h"

class UYapConversationHandler;
class UYapBroker;
struct FYapPromptHandle;
class IYapConversationHandler;
struct FYapBit;
class UYapCharacterComponent;
enum class EYapMaturitySetting : uint8;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYapPromptChosen, UObject*, Instigator, FYapPromptHandle, Handle);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYapSpeechEvent, UObject*, Instigator, FYapSpeechHandle, Handle);

// ================================================================================================

UENUM()
enum class EYapGetHandlerMode : uint8
{
	CreateNewArray,
};

// ================================================================================================

USTRUCT()
struct FYapHandlersArray
{
	GENERATED_BODY()

	TArray<TObjectPtr<UObject>> Array;
};

// ================================================================================================

UCLASS()
class YAP_API UYapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

friend class UFlowNode_YapDialogue;
friend struct FYapFragment;
friend struct FYapPromptHandle;
	
public:
	UYapSubsystem();
	
	// =========================================
	// STATE
	// =========================================
protected:

	/** All registered conversation handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapHandlersArray> ConversationHandlers;

	/** All registered free speech handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapHandlersArray> FreeSpeechHandlers;

	/** The broker object. Active only during play. Editor work uses the CDO instead. */
	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;

	/**  */
	UPROPERTY(Transient)
	TMap<FYapPromptHandle, FGameplayTag> PromptHandleConversationTags;

	// ----------------------
	
	/** Master container of conversations */
	UPROPERTY(Transient)
	TMap<FYapConversationHandle, FYapConversation> Conversations;
	
	/** Queue of conversations. The top one is always going to be "active". If two "Open Conversation" nodes run, the second one will wait in this queue until the first one closes. */
	UPROPERTY(Transient)
	TArray<FYapConversationHandle> ConversationQueue;

	/** The current focused conversation */
	UPROPERTY(Transient)
	FYapConversationHandle ActiveConversation;

	// ----------------------
	
	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Currently, can only store one at a time per fragment; new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/** All registered character components. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/** Helper to ensure that multiple character components are never registered for the same actor. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	static bool bGetGameMaturitySettingWarningIssued;

	UPROPERTY(Transient)
	TSet<FYapSpeechHandle> RunningSpeech;
	
public:
	UPROPERTY(Transient)
	TMap<FYapSpeechHandle, FYapSpeechEvent> SpeechCompleteEvents;
	
	UPROPERTY(Transient)
	TMap<FYapSpeechHandle, FTimerHandle> SpeechTimers;

	// TODO these should probably all be maps for more robust behavior in case multiple things are running!
	
	UPROPERTY(Transient)
	FYapPromptChosen OnPromptChosen;

	UPROPERTY(Transient)
	FYapSpeechEvent OnSpeechSkip;

	UPROPERTY(Transient)
	FYapConversationEvent OnConversationSkip;
	
	/*
	UPROPERTY(BlueprintAssignable, Transient)
	FYapSpeechEvent OnSkipAction;
	*/
	// =========================================
	// PUBLIC API - Your game should use these
	// =========================================
public:
	
	/** Register a conversation handler to a specific type group, or EmptyTag for the default type group. */
	static void RegisterConversationHandler(UObject* NewHandler, FGameplayTag TypeGroup = FGameplayTag::EmptyTag);

	/** Unregister a conversation handler from a specific type group. */
	static void UnregisterConversationHandler(UObject* HandlerToRemove, FGameplayTag TypeGroup = FGameplayTag::EmptyTag);
	
	/** Unregister a conversation handler.*/
	//static void UnregisterConversationHandlerAllTypeGroups(UObject* HandlerToRemove);
	
	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	static void RegisterFreeSpeechHandler(UObject* NewHandler, FGameplayTag TypeGroup = FGameplayTag::EmptyTag);

	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	static void UnregisterFreeSpeechHandler(UObject* HandlerToRemove, FGameplayTag TypeGroup = FGameplayTag::EmptyTag);

	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	//static void UnregisterFreeSpeechHandlerAllTypeGroups(UObject* HandlerToRemove);

	/** Given a character identity tag, find the character component in the world. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	static UYapCharacterComponent* FindCharacterComponent(UWorld* World, FGameplayTag CharacterTag);

	// =========================================
	// YAP API - These are called by Yap classes
	// =========================================
public:

	static UYapSubsystem* Get(UObject* WorldContext)
	{
		if (IsValid(WorldContext))
		{
			return Get(WorldContext->GetWorld());
		}

		return nullptr;
	}
	
	static UYapSubsystem* Get(UWorld* World)
	{
		if (IsValid(World))
		{
			return World->GetSubsystem<UYapSubsystem>();
		}

		return nullptr;
	}

	static bool IsSpeechRunning(UWorld* World, const FYapSpeechHandle& Handle)
	{
		return Get(World)->RunningSpeech.Contains(Handle);
	}
	
public:
	static UYapBroker* GetBroker(UWorld* World);
	
	static EYapMaturitySetting GetCurrentMaturitySetting(UWorld* World);

	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);
	
protected:  // TODO should some of these be public?
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

public:
	// Main open conversation function, and is called by the Open Conversation flow node
	FYapConversation& OpenConversation(const FGameplayTag& ConversationName, UObject* ConversationOwner); // Called by Open Conversation node

	// Main close conversation function
	EYapConversationState CloseConversation(const FYapConversationHandle& Handle);
	
	// Exists primarily to be called by the Close Conversation flow node (with a conversation name)
	EYapConversationState CloseConversation(const FGameplayTag& ConversationName);

	// Exists primarily to be called by the Close Conversation flow node (with an unset conversation name)
	EYapConversationState CloseConversation(const UObject* Owner);
protected:
	// Actually opens a conversation
	void StartOpeningConversation(const FYapConversationHandle& Handle);
	
	// Actually opens a conversation
	bool StartOpeningConversation(FYapConversation& Conversation);

	// Actually closes a conversation
	EYapConversationState StartClosingConversation(const FYapConversationHandle& Handle);










	
	void StartNextQueuedConversation();

	UFUNCTION()
	void OnActiveConversationClosed(UObject* Instigator, FYapConversationHandle Handle);
	
	/**  */
	FYapPromptHandle BroadcastPrompt(const FYapData_PlayerPromptCreated& Data, const FGameplayTag& TypeGroup);

	/**  */
	void OnFinishedBroadcastingPrompts(const FYapData_PlayerPromptsReady& Data, const FGameplayTag& TypeGroup);

public:
	/**  */
	UFUNCTION(BlueprintCallable)
	FYapSpeechHandle RunSpeech(const FYapData_SpeechBegins& SpeechData, const FGameplayTag& TypeGroup, FYapSpeechHandle& Handle);

	// TODO I hate this thing
	static FYapConversation NullConversation;

	// TODO I also hate these things
	/**  */
	static FYapConversation& GetConversation(UObject* WorldContext, UObject* Owner);
	
	/**  */
	static FYapConversation& GetConversation(UObject* WorldContext, FYapConversationHandle Handle);

	/**  */
	static FYapConversation& GetConversation(UObject* WorldContext, const FGameplayTag& ConversationName);

	/**  */
	static FGameplayTag GetActiveConversationName(UWorld* World);

public:
	// TODO should I make a ref struct for FYapPromptHandle too?
	/** The prompt handle will call this function, passing in itself. */
	static void RunPrompt(UWorld* World, const FYapPromptHandle& Handle);

	/** Send a skip or manual advance signal. Returns true if */
	static bool SkipSpeech(UWorld* World, const FYapSpeechHandle& Handle);

	void ConversationSkip(UObject* Instigator, FYapConversationHandle Handle);
	
	/**  */ // TODO: ability to instantly playback/skip through multiple nodes until some sort of target point is hit, maybe a custom node? (imagine skipping an entire cutscene)
	// static bool SkipDialogueTo(???);

	/*
	static FYapRunningFragment& GetFragmentHandle(FYapSpeechHandle HandleRef);
	*/
	
public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	TArray<TObjectPtr<UObject>>& FindOrAddConversationHandlerArray(const FGameplayTag& TypeGroup);

	TArray<TObjectPtr<UObject>>* FindConversationHandlerArray(const FGameplayTag& TypeGroup);
	
	TArray<TObjectPtr<UObject>>& FindOrAddFreeSpeechHandlerArray(const FGameplayTag& TypeGroup);
	
	TArray<TObjectPtr<UObject>>* FindFreeSpeechHandlerArray(const FGameplayTag& TypeGroup);

	void RegisterSpeechHandle(FYapSpeechHandle& Handle);
	
	void UnregisterSpeechHandle(FYapSpeechHandle& Handle);
	
public:
	/**  */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	void Deinitialize() override;

	/**  */
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
protected:
	void OnSpeechComplete(FYapSpeechHandle Handle);

	/**  */
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// Thanks to Blue Man for template help
	template<typename TUInterface, typename TIInterface, auto TFunction, auto TExecFunction, typename... TArgs>
	static void BroadcastEventHandlerFunc(TArray<TObjectPtr<UObject>>* HandlersArray, TArgs&&... Args)
	{
		if (!HandlersArray)
		{
			UE_LOG(LogYap, Error, TEXT("No handlers are currently registered for this type group!"));
			return;
		}
		
		bool bHandled = false;
	
		for (int i = 0; i < HandlersArray->Num(); ++i)
		{
			UObject* HandlerObj = (*HandlersArray)[i];

			if (!IsValid(HandlerObj))
			{
				continue;
			}
		
			if (TIInterface* CppInterface = Cast<TIInterface>(HandlerObj))
			{
				(CppInterface->*TFunction)(Args...);
			}
			else
			{
				// TODO this should throw smarter errors
				check(HandlerObj->Implements<TUInterface>());
				(*TExecFunction)(HandlerObj, Args...);				
			}
		
			bHandled = true;
		}
	
		if (!bHandled)
		{
			UE_LOG(LogYap, Error, TEXT("No Yap Conversation Listeners are currently registered! You must inherit a class from IYapConversationListeners, implement its functions, and register it to the Yap subsystem."));
			return;
		}
	}
};
