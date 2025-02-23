// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapPromptHandle.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapPromptHandle::FYapPromptHandle()
{
	DialogueNode = nullptr;
	FragmentIndex = 0;
}

FYapPromptHandle::FYapPromptHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;
}

void FYapPromptHandle::Invalidate()
{
	DialogueNode = nullptr;
	FragmentIndex = INDEX_NONE;
	Guid.Invalidate();
}

void FYapPromptHandle::RunPrompt(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return;
	}
	
	UYapSubsystem* Subsystem = World->GetSubsystem<UYapSubsystem>();

	Subsystem->RunPrompt(*this);
}

#undef LOCTEXT_NAMESPACE