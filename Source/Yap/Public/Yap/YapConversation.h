// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Handles/YapPromptHandle.h"
#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/YapRunningFragment.h"

#include "YapConversation.generated.h"

struct FGameplayTag;
struct FYapConversation;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYapConversationEvent, UObject*, Instigator, FYapConversationHandle, Handle);

// ================================================================================================

UENUM()
enum class EYapConversationState : uint8
{
    Undefined,
    Opening,
    Open,
    Closing,
    Closed,
};

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapConversation
{
    GENERATED_BODY()

    FYapConversation();

    FYapConversation(const FGameplayTag& InConversationName, UObject* ConversationOwner, const FYapConversationHandle& InHandle);

    // ==========================================
    // STATE
    // ==========================================
protected:

    // TODO I don't know if conversations should store their own handle. Reconsider?
    UPROPERTY(Transient)
    FYapConversationHandle Handle;
    
    UPROPERTY(Transient)
    FGameplayTag ConversationName;

    UPROPERTY(Transient)
    FGameplayTag TypeGroup;
    
    UPROPERTY(Transient)
    TArray<FYapSpeechHandle> RunningFragments;

    UPROPERTY(Transient)
    TArray<FYapPromptHandle> OpenPrompts;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UObject>> OpeningLocks;   

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UObject>> ClosingLocks;

    UPROPERTY(Transient)
    EYapConversationState State = EYapConversationState::Closed;

    UPROPERTY(Transient)
    bool bWantsToOpen = false;

    UPROPERTY(Transient)
    bool bWantsToClose = false;

    /** What created this conversation? Typically this is going to be a flow graph asset. */
    UPROPERTY(Transient)
    TObjectPtr<UObject> Owner;
    
public:
    UPROPERTY(Transient)
    FYapConversationEvent OnConversationOpening;
    
    UPROPERTY(Transient)
    FYapConversationEvent OnConversationOpened;
    
    UPROPERTY(Transient)
    FYapConversationEvent OnConversationClosing;
    
    UPROPERTY(Transient)
    FYapConversationEvent OnConversationClosed;

    UPROPERTY(Transient)
    FYapPromptHandleChosen OnPromptHandleChosen;
    
    // ==========================================
    // API
    // ==========================================
public:
    const FGameplayTag& GetConversationName() const { return ConversationName; }

    const FGameplayTag& GetTypeGroup() const { return TypeGroup; }
    
    const TArray<FYapSpeechHandle>& GetRunningFragments() const { return RunningFragments; }

    const FYapConversationHandle& GetHandle() const { return Handle; }

    const EYapConversationState GetState() const { return State; }

    const UObject* GetOwner() const { return Owner; }
    
    void AddRunningFragment(FYapSpeechHandle Handle);

    void RemoveRunningFragment(FYapSpeechHandle Handle);
    
    // -----
    
    void StartOpening(UObject* Instigator);
    
    void ApplyOpeningInterlock(UObject* Object);

    void ReleaseOpeningInterlock(UObject* Object);
    
    // -----
    
    void StartClosing(UObject* Instigator);

    void ApplyClosingInterlock(UObject* Object);

    void ReleaseClosingInterlock(UObject* Object);

    // -----
    
    void ExecuteSkip();

    bool IsNull() const;
    
private:
    void FinishOpening(UObject* Instigator);
    
    void FinishClosing(UObject* Instigator);

public:
    bool operator== (const FYapConversation& Other)
    {
        return this->Owner == Other.Owner && this->ConversationName == Other.ConversationName;
    }
};