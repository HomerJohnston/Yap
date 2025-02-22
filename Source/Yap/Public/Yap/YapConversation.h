// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "YapConversationHandle.h"
#include "Yap/YapRunningFragment.h"

#include "YapConversation.generated.h"

class UFlowAsset;
struct FGameplayTag;

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapConversation
{
    GENERATED_BODY()

    FYapConversation();

    FYapConversation(UFlowAsset* InFlowAsset, const FGameplayTag& InConversationName);

    ~FYapConversation();
    
    // ==========================================
    // STATE
    // ==========================================
protected:

    UPROPERTY(Transient)
    TObjectPtr<UFlowAsset> FlowAsset = nullptr;

    UPROPERTY(Transient)
    FGameplayTag ConversationName;

    UPROPERTY(Transient)
    TArray<FYapFragmentHandle> RunningFragments;

    UPROPERTY(Transient)
    FGuid Guid;
    
public:
    UPROPERTY(Transient)
    FYapOnConversationClosed_Multi OnConversationClosed;


    // ==========================================
    // API
    // ==========================================
public:
    const UFlowAsset* GetFlowAsset() const { return FlowAsset; }

    const FGameplayTag& GetConversationName() const { return ConversationName; }

    const TArray<FYapFragmentHandle>& GetRunningFragments() const { return RunningFragments; }

    const FGuid& GetGuid() const { return Guid; }
    
    void AddRunningFragment(FYapFragmentHandle Handle);

    void RemoveRunningFragment(FYapFragmentHandle Handle);

    void ExecuteSkip();
};