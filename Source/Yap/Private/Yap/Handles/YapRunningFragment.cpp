// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapRunningFragment.h" 

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

FYapRunningFragment FYapRunningFragment::_InvalidHandle;


FYapRunningFragment::FYapRunningFragment()
{
	Guid = FGuid::NewGuid();
}

FYapRunningFragment::~FYapRunningFragment()
{
}

// ================================================================================================
// FYapDialogueHandle
// ================================================================================================


// ------------------------------------------------------------------------------------------------

const FYapFragment& FYapRunningFragment::GetFragment() const
{
	return DialogueNode->GetFragment(FragmentIndex);
}

void FYapRunningFragment::OnSpeakingEnds() const
{
}

// ------------------------------------------------------------------------------------------------

void FYapRunningFragment::Invalidate()
{
	Guid.Invalidate();
}

void FYapRunningFragment::SetSpeechTimerHandle(FTimerHandle InSpeechTimerHandle)
{
	SpeechTimerHandle = InSpeechTimerHandle;
}

void FYapRunningFragment::SetFragmentTimerHandle(FTimerHandle InFragmentTimerHandle)
{
	FragmentTimerHandle = InFragmentTimerHandle;
}

// ------------------------------------------------------------------------------------------------

bool FYapRunningFragment::operator==(const FYapRunningFragment& Other) const
{
	return Guid == Other.Guid;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
