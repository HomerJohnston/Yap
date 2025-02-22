// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversation_Open.generated.h"

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
public:

	/** Optional name for this conversation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Conversation;
	
	// ==========================================
	// STATE
	// ==========================================
protected:

	// ==========================================
	// API
	// ==========================================
public:
	void OnActivate() override;

	void Finish() override;

protected:
	UFUNCTION()
	void OnOpenConversationTrigger(const FGameplayTag& TriggeredConversation);
	
	UFUNCTION()
	void OnConversationFinishedOpening();

protected:
	void OpenConversation();
	
#if WITH_EDITOR
public:
	FText GetNodeTitle() const override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
