// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/Handles/YapRunningFragment.h"

#include "YapConversation.generated.h"

struct FGameplayTag;
struct FYapConversation;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FYapConversationEvent);

// ================================================================================================

UENUM()
enum class EYapConversationState : uint8
{
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

    FYapConversation(const FGameplayTag& InConversationName, UObject* ConversationOwner);

    // ==========================================
    // STATE
    // ==========================================
protected:

    UPROPERTY(Transient)
    FGameplayTag ConversationName;

    UPROPERTY(Transient)
    TArray<FYapSpeechHandle> RunningFragments;

    UPROPERTY(Transient)
    FGuid Guid;

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
    
    // ==========================================
    // API
    // ==========================================
public:
    const FGameplayTag& GetConversationName() const { return ConversationName; }

    const TArray<FYapSpeechHandle>& GetRunningFragments() const { return RunningFragments; }

    const FGuid& GetGuid() const { return Guid; }

    const EYapConversationState GetState() const { return State; }

    const UObject* GetOwner() const { return Owner; }
    
    void AddRunningFragment(FYapSpeechHandle Handle);

    void RemoveRunningFragment(FYapSpeechHandle Handle);

    // -----
    
    void StartOpening();
    
    void ApplyOpeningInterlock(UObject* Object);

    void ReleaseOpeningInterlock(UObject* Object);
    
    // -----
    
    void StartClosing();

    void ApplyClosingInterlock(UObject* Object);

    void ReleaseClosingInterlock(UObject* Object);

    // -----
    
    void ExecuteSkip();

private:
    void FinishOpening();
    
    void FinishClosing();
};