// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapConversation_Close.h"

#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

UFlowNode_YapConversation_Close::UFlowNode_YapConversation_Close()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversation_Close::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	
	FGameplayTag ConversationToClose = Conversation.IsValid() ? Conversation : UYapSubsystem::GetActiveConversation();
	
	EYapConversationState Result = UYapSubsystem::Get()->RequestCloseConversation(ConversationToClose);

	if (Result == EYapConversationState::Closed)
	{
		UE_LOG(LogYap, Verbose, TEXT("Conversation closed: %s"), *ConversationToClose.GetTagName().ToString());
		FinishNode();
	}
	else
	{
		FYapConversation& ExistingConversation = UYapSubsystem::GetConversation(ConversationToClose);
		ExistingConversation.OnConversationClosed.AddDynamic(this, &ThisClass::FinishNode);
	}
}

void UFlowNode_YapConversation_Close::FinishNode()
{
	UE_LOG(LogYap, Verbose, TEXT("CONVERSATION CLOSING: %s"), *Conversation.GetTagName().ToString());

	UYapSubsystem::GetConversation(Conversation).OnConversationClosed.RemoveAll(this);
	
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FText UFlowNode_YapConversation_Close::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation - Close");
	}

	return FText::FromString("Close Convo.");
}
#endif

#undef LOCTEXT_NAMESPACE
