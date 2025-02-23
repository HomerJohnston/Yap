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

FYapConversation::FYapConversation(const FGameplayTag& InConversationName)
    : ConversationName(InConversationName)
{
    Guid = FGuid::NewGuid();
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::AddRunningFragment(FYapFragmentHandle FragmentHandle)
{
    check(!RunningFragments.Contains(FragmentHandle));

    RunningFragments.Add(FragmentHandle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::RemoveRunningFragment(FYapFragmentHandle FragmentHandle)
{
    check(RunningFragments.Contains(FragmentHandle));

    RunningFragments.Remove(FragmentHandle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::StartOpening()
{
    bWantsToOpen = true;
    bWantsToClose = false;
    State = EYapConversationState::Opening;

    if (OpeningLocks.Num() == 0)
    {
        FinishOpening();
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::AddOpeningLock(UObject* LockingObject)
{
    OpeningLocks.AddUnique(LockingObject);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::RemoveOpeningLock(UObject* Object)
{
    OpeningLocks.Remove(Object);

    if (bWantsToOpen && OpeningLocks.Num() == 0)
    {
        FinishOpening();
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::StartClosing()
{
    State = EYapConversationState::Closing;
    bWantsToOpen = false;
    bWantsToClose = true;
    
    OnConversationClosing.Broadcast();

    if (ClosingLocks.Num() == 0)
    {
        FinishClosing();
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::AddClosingLock(UObject* LockingObject)
{
    ClosingLocks.AddUnique(LockingObject);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::RemoveClosingLock(UObject* Object)
{
    ClosingLocks.Remove(Object);

    if (bWantsToClose && ClosingLocks.Num() == 0)
    {
        FinishClosing();
    }
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::FinishOpening()
{
    bWantsToOpen = false;
    bWantsToClose = false;
    
    State = EYapConversationState::Open;
    
    OnConversationOpened.Broadcast();
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::FinishClosing()
{
    bWantsToOpen = false;
    bWantsToClose = false;
    
    State = EYapConversationState::Closed;
    
    OnConversationClosed.Broadcast();
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::ExecuteSkip()
{
    if (RunningFragments.Num() == 0)
    {
        return;
    }
}

// ------------------------------------------------------------------------------------------------

