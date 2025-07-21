// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/Handles/YapConversationHandle.h"

#include "FlowNode_YapConversation_Close.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Yap Conversation End", Keywords = "yap"))
class YAP_API UFlowNode_YapConversation_Close : public UFlowNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Default")
	FGameplayTag Conversation;
	
public:
	UFlowNode_YapConversation_Close();

	void ExecuteInput(const FName& PinName) override;

	UFUNCTION()
	void FinishNode(UObject* Instigator, FYapConversationHandle Handle);

	void FinishNode_Internal();

#if WITH_EDITOR
	FText GetNodeTitle() const override;

	void UpdateNodeConfigText_Implementation() override;
#endif
};