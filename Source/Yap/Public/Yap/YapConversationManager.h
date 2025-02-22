// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapConversationManager.generated.h"

class UFlowAsset;
struct FGameplayTag;

UCLASS()
class UYapConversationManager : public UObject
{
    GENERATED_BODY()
    
    // =========================================
    // CONSTRUCTION
    // =========================================
public:
   UYapConversationManager();
    
    // =========================================
    // STATE
    // =========================================
private:
    /** The current stack of active conversations. */
    //UPROPERTY(Transient)
    //TArray<FYapConversation> ActiveConversations;

    // =========================================
    // API
    // =========================================
public:

    /**  */
    bool OpenConversation(UFlowAsset* OwningAsset, const FGameplayTag& ConversationName);

    /**  */
    void CloseConversation();

};