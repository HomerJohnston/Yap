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
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::Invalidate()
{
	Guid.Invalidate();
}

// ------------------------------------------------------------------------------------------------

void FYapPromptHandle::RunPrompt(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return;
	}
	
	UYapSubsystem* Subsystem = World->GetSubsystem<UYapSubsystem>();
	
	UYapSubsystem::RunPrompt(*this);
}

// ------------------------------------------------------------------------------------------------

bool UYapPromptHandleBFL::RunPrompt(const FYapPromptHandle& Handle)
{
	return UYapSubsystem::RunPrompt(Handle);
}

bool UYapPromptHandleBFL::Subscribe(const FYapPromptHandle& Handle, FYapPromptHandleChosen Delegate)
{
	return true;
}

#undef LOCTEXT_NAMESPACE
