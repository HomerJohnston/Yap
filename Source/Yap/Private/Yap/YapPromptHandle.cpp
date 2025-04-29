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
	TypeGroup = FGameplayTag::EmptyTag;
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle::FYapPromptHandle(const FGameplayTag& InTypeGroup)
{
	Guid = FGuid::NewGuid();
	TypeGroup = InTypeGroup;
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::Invalidate()
{
	Guid.Invalidate();
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::RunPrompt(UObject* WorldContext)
{
	UWorld* World = WorldContext->GetWorld();

	if (!World)
	{
		return;
	}
	
	UYapSubsystem::RunPrompt(WorldContext->GetWorld(), *this);
}

// ------------------------------------------------------------------------------------------------

void UYapPromptHandleBFL::RunPrompt(UObject* WorldContext, const FYapPromptHandle& Handle)
{
	UYapSubsystem::RunPrompt(WorldContext->GetWorld(), Handle);
}

bool UYapPromptHandleBFL::Subscribe(const FYapPromptHandle& Handle, FYapPromptHandleChosen Delegate)
{
	// TODO URGENT C++ implementation required!
	return true;
}

#undef LOCTEXT_NAMESPACE
