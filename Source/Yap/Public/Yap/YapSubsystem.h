// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "YapConversation.h"
#include "YapCharacterComponent.h"
#include "YapBroker.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Enums/YapMaturitySetting.h"
#include "Yap/YapRunningFragment.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapDataStructures.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/TimerHandle.h"
#include "Engine/World.h"
#include "UObject/ObjectKey.h"

#include "YapSubsystem.generated.h"

class UYapCharacterManager;
class UYapConversationHandler;
class UYapBroker;
struct FYapPromptHandle;
class IYapConversationHandler;
struct FYapBit;
class UYapCharacterComponent;
class UYapSquirrel;
enum class EYapMaturitySetting : uint8;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYapPromptChosen, UObject*, Instigator, FYapPromptHandle, Handle);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FYapSpeechEventHandler, UObject*, Instigator, FYapSpeechHandle, Handle, EYapSpeechCompleteResult, Result);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FYapSpeechEvent, UObject*, Instigator, FYapSpeechHandle, Handle, EYapSpeechCompleteResult, Result);

USTRUCT()
struct FYapSpeechFinishDelegateContainer
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FYapSpeechEventHandler OnCompleted;

	UPROPERTY(Transient)
	FYapSpeechEventHandler OnCancelled;
};

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

	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Array;
};

// ================================================================================================

USTRUCT()
struct FYapSpeechHandlesArray
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<FYapSpeechHandle> Handles;
};

USTRUCT()
struct FYap__ActiveSpeechContainer
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	TObjectPtr<UObject> SpeechOwner = nullptr;
	
	UPROPERTY(Transient)
	FName SpeakerID;

	UPROPERTY(Transient)
	FYapSpeechEvent OnSpeechFinish;

	UPROPERTY(Transient)
	FTimerHandle SpeechTimerHandle;

	UPROPERTY(Transient)
	FYapConversationHandle ConversationHandle;
};

/*
USTRUCT()
struct FYapConversationTestMap
{
	GENERATED_BODY()

	// ----------------------------------------------

	TMap<FYapConversationHandle, FYapConversation> Conversations;

	TMap<TObjectPtr<UObject>, FYapConversationHandle> HandlesByOwner;

	TMap<FName, FYapConversationHandle> HandlesByName;

	// ----------------------------------------------

	const FYapConversationHandle& AddConversation(UObject* Owner, FName Name);

	FYapConversation* FindConversation(UObject* Owner, FName Name);
};
*/

USTRUCT()
struct FYap__ActiveSpeechMap
{
	GENERATED_BODY()

// ----------------------------------------------
// ----------------------------------------------
private:
	UPROPERTY(Transient)
	TMap<FYapSpeechHandle, FYap__ActiveSpeechContainer> AllSpeech;
	
	UPROPERTY(Transient)
	TMap<TObjectPtr<UObject>, FYapSpeechHandlesArray> ContainersByOwner;
	
	UPROPERTY(Transient)
	TMap<FName, FYapSpeechHandlesArray> ContainersBySpeakerID;

// ----------
public:
	FYap__ActiveSpeechContainer* AddSpeech(FYapSpeechHandle Handle, FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner);

	void RemoveSpeech(const FYapSpeechHandle& Handle);

	void BindToSpeechFinish(const FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate);
	
	void UnbindToSpeechFinish(const FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate);

	void SetTimer(const FYapSpeechHandle& Handle, FTimerHandle TimerHandle);
	
	TArray<FYapSpeechHandle> GetHandles(FName SpeakerID);
	
	TArray<FYapSpeechHandle> GetHandles(UObject* SpeechOwner);

	TArray<FYapSpeechHandle> GetHandles(const FYapConversationHandle& ConversationHandle);
	
	FName FindSpeakerID(const FYapSpeechHandle& Handle);

	FTimerHandle FindTimerHandle(const FYapSpeechHandle& Handle);

	FYapSpeechEvent FindSpeechFinishedEvent(const FYapSpeechHandle& Handle);

	FYapConversationHandle FindSpeechConversationHandle(const FYapSpeechHandle& Handle);

	bool IsSpeechRunning(const FYapSpeechHandle& Handle);
	
// ----------------------------------------------
// ----------------------------------------------
private:
	UPROPERTY(Transient)
	TMap<FYapConversationHandle, FYapConversation> Conversations;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UObject>, FYapConversationHandle> ConversationsByOwner;
	
// ----------
public:
	FYapConversation& AddConversation(FName ConversationName, UObject* ConversationOwner, FYapConversationHandle& ConversationHandle);

	void RemoveConversation(FYapConversationHandle ConversationHandle);
	
	FYapConversation* FindConversationByOwner(const UObject* Owner);

	FYapConversationHandle* FindConversationHandleByOwner(const UObject* Owner);
	
	FYapConversation* FindConversation(const FYapConversationHandle& ConversationHandle);
};

// ================================================================================================

// Alias for TSubclassOf<UFlowNode_YapDialogue>.
// This is a wrapper to auto-convert a null type to the default Yap dialogue node type.
USTRUCT(BlueprintType)
struct FYapDialogueNodeClassType
{
	GENERATED_BODY()

	FYapDialogueNodeClassType()
	{
		NodeType = UFlowNode_YapDialogue::StaticClass();
	}
	
	FYapDialogueNodeClassType(UClass* InNodeType)
	{
		if (InNodeType && InNodeType->IsChildOf(UFlowNode_YapDialogue::StaticClass()))
		{
			NodeType = InNodeType;
		}
		else
		{
			NodeType = UFlowNode_YapDialogue::StaticClass();
		}
	}

	FYapDialogueNodeClassType(TSubclassOf<UFlowNode_YapDialogue> InNodeType)
	{
		if (InNodeType && InNodeType->IsChildOf(UFlowNode_YapDialogue::StaticClass()))
		{
			NodeType = InNodeType;
		}
		else
		{
			NodeType = UFlowNode_YapDialogue::StaticClass();
		}
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UFlowNode_YapDialogue> NodeType;

	// Implicit conversion... usually works but not everywhere?
	operator TSubclassOf<UFlowNode_YapDialogue>() const
	{
		return NodeType;
	}

	// Explicit getter
	TSubclassOf<UFlowNode_YapDialogue> Get() const
	{
		return NodeType;
	}
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

	// -----------------------------------------
	// STATE
	// -----------------------------------------
protected:
	/** All registered conversation handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TMap<TSubclassOf<UFlowNode_YapDialogue>, FYapHandlersArray> ConversationHandlers;

	/** All registered free speech handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. Calling order will be preserved in order of registration. */
	UPROPERTY(Transient)
	TMap<TSubclassOf<UFlowNode_YapDialogue>, FYapHandlersArray> FreeSpeechHandlers;

	/** The broker object. Active only during play. Editor work uses the CDO instead. */
	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;

	/** Queue of conversations. The top one is always going to be "active". If two "Open Conversation" nodes run, the second one will wait in this queue until the first one closes. */
	UPROPERTY(Transient)
	TArray<FYapConversationHandle> ConversationQueue;

	///** Stores which conversation a given speech is a part of */
	//UPROPERTY(Transient)
	//TMap<FYapSpeechHandle, FYapConversationHandle> SpeechConversationMapping;
	
	/** Stores which conversation a given prompt is a part of */
	UPROPERTY(Transient)
	TMap<FYapPromptHandle, FYapConversationHandle> PromptHandleConversationTags;

	// TODO dialogue node FName map?
	// TODO change this up, should instead use Flow asset instance + flow node instance + fragment FName 
	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Currently, can only store one at a time per fragment; new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/** All registered character components. */
	UPROPERTY(Transient)
	TMap<FName, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/** Helper to ensure that multiple character components are never registered for the same actor. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	static bool bGetGameMaturitySettingWarningIssued;

public:
	static void BindToSpeechFinish(UObject* WorldContextObject, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);
	
	static void UnbindToSpeechFinish(UObject* WorldContextObject, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);

	UPROPERTY(Transient)
	FYap__ActiveSpeechMap ActiveSpeechMap;
	
	// TODO these should probably all be maps for more robust behavior in case multiple things are running!
	
	UPROPERTY(Transient)
	FYapPromptChosen OnPromptChosen;

	UPROPERTY(Transient)
	FYapConversationEvent OnAdvanceConversationDelegate;
	
	UPROPERTY(Transient, Instanced)
	TObjectPtr<UYapSquirrel> NoiseGenerator;

	UPROPERTY(Transient, Instanced)
	TObjectPtr<UYapCharacterManager> CharacterManager;
	
public:
	UYapSquirrel& GetNoiseGenerator() { return *NoiseGenerator; }

	static UYapCharacterManager& GetCharacterManager(UObject* WorldContextObject);
	
	/*
	UPROPERTY(BlueprintAssignable, Transient)
	FYapSpeechEvent OnSkipAction;
	*/
	// =========================================
	// PUBLIC API - Your game should use these
	// =========================================
public:
	
	/** Register a conversation handler for a node type. Nullptr will use the default yap node. */
	static void RegisterConversationHandler(UObject* NewHandler, FYapDialogueNodeClassType NodeType);

	/** Unregister a conversation handler for a node type. Nullptr will use the default yap node. */
	static void UnregisterConversationHandler(UObject* HandlerToRemove, FYapDialogueNodeClassType NodeType);
	
	/**  */
	//static void UnregisterConversationHandlerAllTypes(UObject* HandlerToRemove);
	
	/** Register a free speech handler for a node type. Nullptr will use the default yap node. */
	static void RegisterFreeSpeechHandler(UObject* NewHandler, FYapDialogueNodeClassType NodeType);

	/** Unregister a free speech handler for a node type. Nullptr will use the default yap node.*/
	static void UnregisterFreeSpeechHandler(UObject* HandlerToRemove, FYapDialogueNodeClassType NodeType);

	/**  */
	//static void UnregisterFreeSpeechHandlerAllTypes(UObject* HandlerToRemove);

	/** Given a character identity tag, attempt to find the character component in the world. */
	static UYapCharacterComponent* FindCharacterComponent(UWorld* World, FName CharacterName);

	// =========================================
	// YAP API - These are called by Yap classes
	// =========================================
public:

	static UYapSubsystem* Get(const UObject* WorldContext)
	{
		if (IsValid(WorldContext))
		{
			return Get(WorldContext->GetWorld());
		}

		return nullptr;
	}
	
	static UYapSubsystem* Get(const UWorld* World)
	{
		if (IsValid(World))
		{
			return World->GetSubsystem<UYapSubsystem>();
		}

		return nullptr;
	}

	static bool IsNodeInConversation(const UFlowNode_YapDialogue* DialogueNode);
	
	static bool IsSpeechInConversation(const UObject* WorldContext, const FYapSpeechHandle& Handle);

public:
#if WITH_EDITOR
	static const UYapBroker& GetBroker_Editor();
#endif
	
	static UYapBroker& GetBroker(UObject* WorldContext);
	
	static EYapMaturitySetting GetCurrentMaturitySetting(UWorld* World);

	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);
	
protected:  // TODO should some of these be public?
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

public:
	// Main open conversation function, and is called by the Open Conversation flow node
	FYapConversation& OpenConversation(FName ConversationName, UObject* ConversationOwner); // Called by Open Conversation node

	// Main close conversation function
	EYapConversationState CloseConversation(FYapConversationHandle& Handle);
	
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
	EYapConversationState StartClosingConversation(FYapConversationHandle& Handle);
	
	/** The current focused conversation; same as Con */
	const FYapConversationHandle& GetActiveConversation();;
	
	void StartNextQueuedConversation();

	UFUNCTION()
	void OnActiveConversationClosed(UObject* Instigator, FYapConversationHandle Handle);
	
	/**  */
	FYapPromptHandle BroadcastPrompt(const FYapData_PlayerPromptCreated& Data, FYapDialogueNodeClassType NodeType);

	/**  */
	void OnFinishedBroadcastingPrompts(const FYapData_PlayerPromptsReady& Data, FYapDialogueNodeClassType NodeType);

public:
	void RunSpeech(const FYapData_SpeechBegins& SpeechData, FYapDialogueNodeClassType NodeType, const FYapSpeechHandle& SpeechHandle);

	/** This is a bit ghetto. Normally Yap permits speech to overlap (negative padding or Talk And Advance node usage), but sometimes we don't want that. This tells the subsystem to cancel this speech event if another one starts up. */
	void MarkConversationSpeechAsFragile(const FYapSpeechHandle& Handle);

	UPROPERTY(Transient)
	TMap<FYapConversationHandle, FYapSpeechHandlesArray> FragileSpeechHandles;
	
	// TODO I hate this thing
	// static FYapConversation NullConversation;

	static FName Yap_UnnamedConvo;
	
	// TODO I also hate these things
	/**  */
	static FYapConversation* GetConversationByOwner(UObject* WorldContext, UObject* Owner);
	
	/**  */
	static FYapConversation* GetConversationByHandle(UObject* WorldContext, const FYapConversationHandle& Handle);

	/**  
	static FGameplayTag GetActiveConversationName(UWorld* World);
	*/
	
public:
	// TODO should I make a ref struct for FYapPromptHandle too?
	/** The prompt handle will call this function, passing in itself. */
	static void RunPrompt(UObject* WorldContext, const FYapPromptHandle& Handle);

	/** Used to complete a single speech early; supposed to be used for free world speech */
	static bool CancelSpeech(UObject* WorldContext, FYapSpeechHandle& Handle);

	/** Used to complete a single speech early; supposed to be used for free world speech */
	static bool CancelSpeech(UObject* SpeechOwner);

	/** Used to complete all running speech within a given conversation; supposed to be used for "skip" or "continue/advance" type buttons */
	static void AdvanceConversation(UObject* Instigator, const FYapConversationHandle& ConversationHandle);
	
	// TODO: ability to instantly playback/skip through multiple nodes until some sort of target point is hit, maybe a custom node? (imagine skipping an entire cutscene)
	// static bool SkipConversationTo(???);

	bool EmitSpeechResult(const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result);
	
public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	TArray<TObjectPtr<UObject>>& FindOrAddConversationHandlerArray(FYapDialogueNodeClassType NodeType);

	TArray<TObjectPtr<UObject>>* FindConversationHandlerArray(FYapDialogueNodeClassType NodeType);
	
	TArray<TObjectPtr<UObject>>& FindOrAddFreeSpeechHandlerArray(FYapDialogueNodeClassType NodeType);
	
	TArray<TObjectPtr<UObject>>* FindFreeSpeechHandlerArray(FYapDialogueNodeClassType NodeType);

	FYapSpeechHandle GetNewSpeechHandle(FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner);
	
	FYapSpeechHandle GetNewSpeechHandle(FGuid Guid, FName SpeakerID, UObject* SpeechOwner, UObject* ConversationOwner);
	
public:
	/**  */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	void Deinitialize() override;

	/**  */
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
protected:
	void OnSpeechComplete(FYapSpeechHandle Handle, bool bBroadcast, EYapSpeechCompleteResult SpeechResult = EYapSpeechCompleteResult::Undefined);

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
