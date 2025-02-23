// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapSpeechHandle.h"

#include "Yap/YapBlueprintFunctionLibrary.h"


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
    return UYapBlueprintFunctionLibrary::SkipDialogue(*this);
}

// ------------------------------------------------------------------------------------------------

void FYapSpeechHandle::AddReactor(UObject* Reactor)
{
    UYapBlueprintFunctionLibrary::AddReactor(*this, Reactor);
}

// ------------------------------------------------------------------------------------------------

const TArray<FInstancedStruct>& FYapSpeechHandle::GetFragmentData()
{
    return UYapBlueprintFunctionLibrary::GetFragmentData(*this);
}

// ------------------------------------------------------------------------------------------------

bool FYapSpeechHandle::operator==(const FYapSpeechHandle& Other) const
{
    return Guid == Other.Guid;
}