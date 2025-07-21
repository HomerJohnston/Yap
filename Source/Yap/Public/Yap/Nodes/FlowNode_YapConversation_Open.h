// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversation_Open.generated.h"

struct FYapConversation;

/** Opens a conversation for this running flow asset. IMPORTANT: All entered dialogue nodes in this flow asset will broadcast inside of this conversation while the conversation is open! */
UCLASS(NotBlueprintable, meta = (DisplayName = "Yap Conversation Start", Keywords = "yap"))
class YAP_API UFlowNode_YapConversation_Open : public UFlowNode
{
	GENERATED_BODY()
public:
	UFlowNode_YapConversation_Open();
	
	// ==========================================
	// SETTINGS
	// ==========================================
protected:

	/** Optional name for this conversation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	FGameplayTag ConversationName;

	// ==========================================
	// API
	// ==========================================
public:

	void ExecuteInput(const FName& PinName) override;

protected:
	UFUNCTION()
	void FinishNode(UObject* Instigator, FYapConversationHandle Handle);

	void FinishNode_Internal();
	
#if WITH_EDITOR
public:
	FText GetNodeTitle() const override;
	
	void UpdateNodeConfigText_Implementation() override;
#endif
};
