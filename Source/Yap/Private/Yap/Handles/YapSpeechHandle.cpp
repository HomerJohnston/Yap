// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================
// FYapDialogueHandleRef
// ================================================================================================

FYapSpeechHandle::FYapSpeechHandle()
{
}

FYapSpeechHandle::FYapSpeechHandle(const FYapRunningFragment& RunningFragment)
{
    Guid = RunningFragment.GetGuid();
}

bool FYapSpeechHandle::SkipDialogue()
{
    return UYapSpeechHandleBFL::SkipDialogue(*this);
}

// ------------------------------------------------------------------------------------------------

/*
const TArray<FInstancedStruct>& FYapSpeechHandle::GetFragmentData()
{
    return UYapSpeechHandleBFL::GetFragmentData(*this);
}
*/

// ------------------------------------------------------------------------------------------------

bool FYapSpeechHandle::operator==(const FYapSpeechHandle& Other) const
{
    return Guid == Other.Guid;
}

void FYapSpeechHandle::BindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const
{
	FYapSpeechEvent* Event = UYapSubsystem::Get()->SpeechCompleteEvents.Find(*this);

	if (Event)
	{
		(*Event).Add(Delegate);
	}
}

void FYapSpeechHandle::UnbindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const
{
	FYapSpeechEvent* Event = UYapSubsystem::Get()->SpeechCompleteEvents.Find(*this);

	if (Event)
	{
		(*Event).Remove(Delegate);
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSpeechHandleBFL::BindToOnSpeechComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		Handle.BindToOnSpeechComplete(Delegate);
	}
}

void UYapSpeechHandleBFL::UnbindToOnSpeechComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		Handle.UnbindToOnSpeechComplete(Delegate);
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSpeechHandleBFL::BindToOnFragmentComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate)
{
	//UYapSubsystem::Get()->OnFragmentCompleteEvent.Add(Delegate);
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::SkipDialogue(const FYapSpeechHandle& Handle)
{
	if (Handle.IsValid())
	{
		if (!UYapSubsystem::SkipSpeech(Handle))
		{
			UE_LOG(LogYap, Display, TEXT("Failed to skip dialogue!"))
		}
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempted to skip with invalid handle!"))
	}
	
	return false;
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::CanSkipCurrently(const FYapSpeechHandle& Handle)
{
	return true;
	/*
	if (!Handle.IsValid())
	{
		return false;
	}

	UFlowNode_YapDialogue* DialogueNode = UYapSubsystem::GetDialogueHandle(Handle).GetDialogueNode();

	if (DialogueNode)
	{
		return DialogueNode->CanSkip();
	}

	return false;
	*/
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::EqualEqual_YapSpeechHandle(FYapSpeechHandle A, FYapSpeechHandle B)
{
	return A.GetGuid() == B.GetGuid();
}

/*
const TArray<FInstancedStruct>& UYapSpeechHandleBFL::GetFragmentData(const FYapSpeechHandle& HandleRef)
{
	const FYapRunningFragment& Handle = UYapSubsystem::GetFragmentHandle(HandleRef);

	const UFlowNode_YapDialogue* DialogueNode = Handle.GetDialogueNode();

	const FYapFragment& Fragment = DialogueNode->GetFragments()[Handle.GetFragmentIndex()];
	
	return Fragment.GetData();
}
*/
	
// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
