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

	UYapSubsystem* Subsystem = UYapSubsystem::Get(this);

	FYapConversation& ConversationInst = (Conversation.IsValid()) ? Subsystem->GetConversationByName(Conversation, GetFlowAsset()) : Subsystem->GetConversationByOwner(this, GetFlowAsset());

	if (ConversationInst.IsNull())
	{
		UE_LOG(LogYap, Warning, TEXT("Found no conversation to close! Conversation name: %s, owner: %s"), *Conversation.ToString(), *GetName());
		return;
	}
	
	EYapConversationState Result = Subsystem->CloseConversation(ConversationInst.GetHandle());

	switch (Result)
	{
		case EYapConversationState::Closed:
		{
			FinishNode_Internal();
			break;
		}
		case EYapConversationState::Closing:
		{
			ConversationInst.OnConversationClosed.AddDynamic(this, &ThisClass::FinishNode);
			break;
		}
		default:
		{
			UE_LOG(LogYap, Warning, TEXT("Failed to close conversation - unknown error! Conversation name: %s, owner: %s"), *Conversation.ToString(), *GetName());
			break;
		}
	}
}

void UFlowNode_YapConversation_Close::FinishNode(UObject* Instigator, FYapConversationHandle)
{
	FinishNode_Internal();
}

void UFlowNode_YapConversation_Close::FinishNode_Internal()
{
	UE_LOG(LogYap, Verbose, TEXT("Conversation closed: %s for owner: %s"), *Conversation.GetTagName().ToString(), *GetFlowAsset()->GetName());

	UYapSubsystem::GetConversationByName(Conversation, GetFlowAsset()).OnConversationClosed.RemoveAll(this);
	
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
