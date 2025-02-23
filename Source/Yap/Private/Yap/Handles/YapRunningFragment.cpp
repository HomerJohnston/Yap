// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapRunningFragment.h"

#include "Yap/YapBlueprintFunctionLibrary.h"
#include "Yap/Interfaces/IYapHandleReactor.h"
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
	/*
	for (TWeakObjectPtr<UObject> Reactor : Reactors)
	{
		if (Reactor.IsValid())
		{
			IYapHandleReactor::Execute_K2_OnSpeakingEnds(Reactor.Get());
		}
	}
	*/
}

// ------------------------------------------------------------------------------------------------

void FYapRunningFragment::Invalidate()
{
	Guid.Invalidate();
	
	/*
	for (TWeakObjectPtr<UObject> Reactor : Reactors)
	{
		if (Reactor.IsValid())
		{
			IYapHandleReactor::Execute_K2_OnHandleInvalidated(Reactor.Get());
		}
	}
	
	Reactors.Empty();
	*/
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

/*
void FYapRunningFragment::AddReactor(UObject* Reactor)
{
	if (ensureMsgf(Reactor->Implements<UYapHandleReactor>(), TEXT("FYapDialogueHandle::AddReactor(...) failed: object does not implement IYapHandleReactor! [%s]"), *Reactor->GetName()))
	{
		Reactors.Add(Reactor);
	}
}
*/

// ------------------------------------------------------------------------------------------------

bool FYapRunningFragment::operator==(const FYapRunningFragment& Other) const
{
	return Guid == Other.Guid;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
