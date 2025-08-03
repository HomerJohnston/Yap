// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/YapConversation.h"

#include "Yap/YapRunningFragment.h"
#include "Yap/YapSubsystem.h"

// ------------------------------------------------------------------------------------------------

FYapConversation::FYapConversation()
{
}

// ------------------------------------------------------------------------------------------------

FYapConversation::FYapConversation(const FGameplayTag& InConversationName, UObject* ConversationOwner, const FYapConversationHandle& InHandle)
    : ConversationName(InConversationName)
    , Owner(ConversationOwner)
{
    Handle = InHandle;
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::AddRunningFragment(FYapSpeechHandle FragmentHandle)
{
    check(!RunningSpeech.Contains(FragmentHandle));

    RunningSpeech.Add(FragmentHandle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::RemoveRunningFragment(FYapSpeechHandle FragmentHandle)
{
    check(RunningSpeech.Contains(FragmentHandle));

    RunningSpeech.Remove(FragmentHandle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::StartOpening(UObject* Instigator)
{
    bWantsToOpen = true;
    bWantsToClose = false;
    State = EYapConversationState::Opening;

    OnConversationOpening.Broadcast(Instigator, Handle);
    
    if (OpeningLocks.Num() == 0)
    {
        FinishOpening(Instigator);
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ApplyOpeningInterlock(UObject* LockingObject)
{
    OpeningLocks.AddUnique(LockingObject);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ReleaseOpeningInterlock(UObject* Object)
{
    OpeningLocks.Remove(Object);

    if (bWantsToOpen && OpeningLocks.Num() == 0)
    {
        FinishOpening(Object);
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::StartClosing(UObject* Instigator)
{
    State = EYapConversationState::Closing;
    bWantsToOpen = false;
    bWantsToClose = true;
    
    OnConversationClosing.Broadcast(Instigator, Handle);

    if (ClosingLocks.Num() == 0)
    {
        FinishClosing(Instigator);
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ApplyClosingInterlock(UObject* LockingObject)
{
    ClosingLocks.AddUnique(LockingObject);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ReleaseClosingInterlock(UObject* Object)
{
    ClosingLocks.Remove(Object);

    if (bWantsToClose && ClosingLocks.Num() == 0)
    {
        FinishClosing(Object);
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::FinishOpening(UObject* Instigator)
{
    bWantsToOpen = false;
    bWantsToClose = false;
    
    State = EYapConversationState::Open;
    
    OnConversationOpened.Broadcast(Instigator, Handle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::FinishClosing(UObject* Instigator)
{
    bWantsToOpen = false;
    bWantsToClose = false;
    
    State = EYapConversationState::Closed;
    
    OnConversationClosed.Broadcast(Instigator, Handle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ExecuteSkip()
{
    if (RunningSpeech.Num() == 0)
    {
        return;
    }
}

bool FYapConversation::IsNull() const
{
    return this == &UYapSubsystem::NullConversation;
}

// ------------------------------------------------------------------------------------------------

