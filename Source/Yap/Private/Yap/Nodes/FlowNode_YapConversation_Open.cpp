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

void UFlowNode_YapConversation_Open::InitializeInstance()
{
	Super::InitializeInstance();

	TArray<UFlowNode*> ConnectedNodes;
}

void UFlowNode_YapConversation_Open::OnActivate()
{
	if (bTriggerFlop)
	{
		return;
	}
	
	GetWorld()->GetSubsystem<UYapSubsystem>()->EnqueueConversation(GetFlowAsset(), Conversation);
	
	if (UYapProjectSettings::GetConversationsRequireAdvanceTriggerToStart())
	{
		UYapSubsystem::Get()->AdvanceIntoConversation.AddUObject(this, &ThisClass::OnConversationOpenTrigger);
		bTriggerFlop = true;
	}
}

void UFlowNode_YapConversation_Open::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	if (!bTriggerFlop)
	{
		TriggerFirstOutput(true);
	}
}

void UFlowNode_YapConversation_Open::OnConversationOpenTrigger()
{
	if (!bTriggerFlop)
	{
		return;
	}
	
	TriggerFirstOutput(true);
	bTriggerFlop = false;
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