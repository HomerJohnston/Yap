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
	World = nullptr;
	Guid.Invalidate();
}

FYapSpeechHandle::FYapSpeechHandle(UWorld* InWorld, const FGuid& InGuid)
{
	check(::IsValid(InWorld));
	World = InWorld;
	Guid = InGuid;

	UYapSubsystem::Get(InWorld)->RegisterSpeechHandle(*this);
}

FYapSpeechHandle::~FYapSpeechHandle()
{
}

bool FYapSpeechHandle::SkipDialogue()
{
    return UYapSpeechHandleBFL::SkipDialogue(World.Get(), *this);
}

void FYapSpeechHandle::Invalidate()
{
	if (World.IsValid())
	{
		UYapSubsystem::Get(World.Get())->UnregisterSpeechHandle(*this);
	}
        
	Guid.Invalidate();   
}

// ------------------------------------------------------------------------------------------------

bool FYapSpeechHandle::operator==(const FYapSpeechHandle& Other) const
{
    return Guid == Other.Guid;
}

// ------------------------------------------------------------------------------------------------

void UYapSpeechHandleBFL::BindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		FYapSpeechEvent* Event = UYapSubsystem::Get(WorldContext->GetWorld())->SpeechCompleteEvents.Find(Handle);

		if (Event)
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s: Binding delegate %s for handle {%s}"), *Delegate.GetUObject()->GetName(), *Delegate.GetFunctionName().ToString(), *Handle.ToString());
			Event->Add(Delegate);
		}
	}
}

void UYapSpeechHandleBFL::UnbindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		UYapSubsystem* Subsystem = UYapSubsystem::Get(WorldContext->GetWorld());

		FYapSpeechEvent* Event = Subsystem->SpeechCompleteEvents.Find(Handle);
		
		if (Event)
		{
			UE_LOG(LogYap, VeryVerbose, TEXT("%s: Unbinding delegate %s for handle {%s}"), *Delegate.GetUObject()->GetName(), *Delegate.GetFunctionName().ToString(), *Handle.ToString());
			Event->Remove(Delegate);
		}
	}
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::SkipDialogue(UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	if (Handle.IsValid())
	{
		if (!UYapSubsystem::SkipSpeech(WorldContext->GetWorld(), Handle))
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

bool UYapSpeechHandleBFL::CanSkipCurrently(UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	// TODO URGENT - need this for UI development
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

bool UYapSpeechHandleBFL::IsRunning(UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	return UYapSubsystem::IsSpeechRunning(WorldContext->GetWorld(), Handle);
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::EqualEqual_YapSpeechHandle(FYapSpeechHandle A, FYapSpeechHandle B)
{
	return A.GetGuid() == B.GetGuid();
}

FString UYapSpeechHandleBFL::ToString(const FYapSpeechHandle Handle)
{
	return Handle.ToString();
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
