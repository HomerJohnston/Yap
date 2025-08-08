// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================
// FYapDialogueHandleRef
// ================================================================================================

FYapSpeechHandle::FYapSpeechHandle()
{
	World = nullptr;
	Guid.Invalidate();
}

FYapSpeechHandle::FYapSpeechHandle(UWorld* InWorld, FGuid InGuid)
{
	check(::IsValid(InWorld));
	World = InWorld;
	Guid = InGuid;
}

FYapSpeechHandle::~FYapSpeechHandle()
{
}

bool FYapSpeechHandle::SkipDialogue()
{
    return UYapSpeechHandleBFL::CancelSpeech(World.Get(), *this);
}

void FYapSpeechHandle::Invalidate()
{
	World = nullptr;
        
	Guid.Invalidate();
	
	bActive = false;
}

// ------------------------------------------------------------------------------------------------

bool FYapSpeechHandle::operator==(const FYapSpeechHandle& Other) const
{
    return Guid == Other.Guid;
}

// ------------------------------------------------------------------------------------------------

void UYapSpeechHandleBFL::BindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: Binding completion delegate %s for handle {%s}"), *Delegate.GetUObject()->GetName(), *Delegate.GetFunctionName().ToString(), *Handle.ToString());
			
		UYapSubsystem::BindToSpeechFinish(WorldContext, Handle, Delegate);
	}
}

void UYapSpeechHandleBFL::UnbindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate)
{
	if (ensureAlwaysMsgf(Handle.IsValid(), TEXT("Null/unset Yap Speech Handle!")))
	{
		UE_LOG(LogYap, VeryVerbose, TEXT("%s: Unbinding completion delegate %s for handle {%s}"), *Delegate.GetUObject()->GetName(), *Delegate.GetFunctionName().ToString(), *Handle.ToString());

		UYapSubsystem::UnbindToSpeechFinish(WorldContext, Handle, Delegate);
	}
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::CancelSpeech(UObject* WorldContext, FYapSpeechHandle& Handle)
{
	if (Handle.IsValid())
	{
		return UYapSubsystem::CancelSpeech(WorldContext, Handle);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempted to cancel an invalid speech handle!"))
	}
	
	return false;
}

bool UYapSpeechHandleBFL::CancelSpeechByOwner(UObject* SpeechOwner)
{
	if (IsValid(SpeechOwner))
	{
		return UYapSubsystem::CancelSpeech(SpeechOwner);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempting to cancel speech for a null speech owner!"));
	}
	
	return false;
}

bool UYapSpeechHandleBFL::AdvanceSpeech(UObject* WorldContext, FYapSpeechHandle& Handle)
{
	if (Handle.IsValid())
	{
		return UYapSubsystem::AdvanceSpeech(WorldContext, Handle);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempted to cancel an invalid speech handle!"))
	}
	
	return false;
}

bool UYapSpeechHandleBFL::AdvanceSpeechByOwner(UObject* SpeechOwner)
{
	if (IsValid(SpeechOwner))
	{
		return UYapSubsystem::AdvanceSpeech(SpeechOwner);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempting to cancel speech for a null speech owner!"));
	}
	
	return false;
}

// ------------------------------------------------------------------------------------------------

bool UYapSpeechHandleBFL::CanSkip(UObject* WorldContext, const FYapSpeechHandle& Handle)
{
	UE_LOG(LogYap, Warning, TEXT("CanSkipCurrently is work in progress and currently just returns true"));
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

void UYapSpeechHandleBFL::Invalidate(FYapSpeechHandle& Handle)
{
	Handle.Invalidate();
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
