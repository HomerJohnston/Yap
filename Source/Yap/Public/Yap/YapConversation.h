// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "YapConversationHandle.h"
#include "Yap/YapRunningFragment.h"

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

    FYapConversation(const FGameplayTag& InConversationName);

    // ==========================================
    // STATE
    // ==========================================
protected:

    UPROPERTY(Transient)
    FGameplayTag ConversationName;

    UPROPERTY(Transient)
    TArray<FYapFragmentHandle> RunningFragments;

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
    
public:
    // No point in having an Opening event?
    
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

    const TArray<FYapFragmentHandle>& GetRunningFragments() const { return RunningFragments; }

    const FGuid& GetGuid() const { return Guid; }

    const EYapConversationState GetState() const { return State; }
    
    void AddRunningFragment(FYapFragmentHandle Handle);

    void RemoveRunningFragment(FYapFragmentHandle Handle);

    // -----
    
    void StartOpening();
    
    void AddOpeningLock(UObject* Object);

    void RemoveOpeningLock(UObject* Object);
    
    // -----
    
    void StartClosing();

    void AddClosingLock(UObject* Object);

    void RemoveClosingLock(UObject* Object);

    // -----
    
    void ExecuteSkip();

private:
    void FinishOpening();
    
    void FinishClosing();
};