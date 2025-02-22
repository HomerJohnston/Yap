// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapConversation_Open.h"

#include "FlowAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapConversation_Close.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

UFlowNode_YapConversation_Open::UFlowNode_YapConversation_Open()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversation_Open::OnActivate()
{
	EYapOpenConversationResult Result = GetWorld()->GetSubsystem<UYapSubsystem>()->OpenOrEnqueueConversation(Conversation);

	if (Result == EYapOpenConversationResult::Opened)
	{
		OpenConversation();
	}
	if (Result == EYapOpenConversationResult::Queued)
	{
		UE_LOG(LogYap, Verbose, TEXT("Conversation Queued: %s"), *Conversation.GetTagName().ToString());
		UYapSubsystem::Get()->OnOpenConversation.AddDynamic(this, &ThisClass::OnOpenConversationTrigger);
	}
}

void UFlowNode_YapConversation_Open::Finish()
{
	Super::Finish();
	
	UE_LOG(LogYap, Verbose, TEXT("CONVERSATION CLOSING: %s"), *Conversation.GetTagName().ToString());
	
	UYapSubsystem::Get()->OnConversationFinishedOpening.Unbind();
}

void UFlowNode_YapConversation_Open::OnOpenConversationTrigger(const FGameplayTag& TriggeredConversation)
{
	if (Conversation != TriggeredConversation)
	{
		return;
	}

	OpenConversation();
}

void UFlowNode_YapConversation_Open::OnConversationFinishedOpening()
{
	UYapSubsystem::Get()->OnConversationFinishedOpening.Unbind();
	TriggerFirstOutput(true);
}

void UFlowNode_YapConversation_Open::OpenConversation()
{
	UE_LOG(LogYap, Verbose, TEXT("CONVERSATION OPENING: %s"), *Conversation.GetTagName().ToString());

	UYapSubsystem::Get()->OnOpenConversation.RemoveDynamic(this, &ThisClass::OnOpenConversationTrigger);

	// This conversation is becoming active. However, we don't actually open yet in case there is a lock.
	// We will let the subsystem or conversation struct inform us when to actually progress the flow graph.
	UYapSubsystem::Get()->OnConversationFinishedOpening.BindDynamic(this, &ThisClass::OnConversationFinishedOpening);
}

#if WITH_EDITOR
FText UFlowNode_YapConversation_Open::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation - Open");
	}

	return FText::FromString("Open Convo.");
}

void UFlowNode_YapConversation_Open::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE