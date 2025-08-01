// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapPromptHandle.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

FYapPromptHandle::FYapPromptHandle()
{
	Guid = FGuid::NewGuid();
	this->NodeType = nullptr;
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle::FYapPromptHandle(TSubclassOf<UFlowNode_YapDialogue> NodeType)
{
	Guid = FGuid::NewGuid();
	this->NodeType = NodeType;
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::Invalidate()
{
	Guid.Invalidate();
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::RunPrompt(UObject* WorldContext)
{
	UYapSubsystem::RunPrompt(WorldContext, *this);
}

// ------------------------------------------------------------------------------------------------

void UYapPromptHandleBFL::RunPrompt(UObject* WorldContext, const FYapPromptHandle& Handle)
{
	UYapSubsystem::RunPrompt(WorldContext, Handle);
}

/*
bool UYapPromptHandleBFL::Subscribe(const FYapPromptHandle& Handle, FYapPromptHandleChosen Delegate)
{
	
}
*/

#undef LOCTEXT_NAMESPACE
