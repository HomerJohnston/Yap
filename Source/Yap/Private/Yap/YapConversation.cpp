// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/YapConversation.h"

#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapSubsystem.h"

// ------------------------------------------------------------------------------------------------

FYapConversation::FYapConversation()
{
}

// ------------------------------------------------------------------------------------------------

FYapConversation::FYapConversation(const FGameplayTag& InConversationName, UObject* ConversationOwner)
    : ConversationName(InConversationName)
    , Owner(ConversationOwner)
{
    Guid = FGuid::NewGuid();
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::AddRunningFragment(FYapSpeechHandle FragmentHandle)
{
    check(!RunningFragments.Contains(FragmentHandle));

    RunningFragments.Add(FragmentHandle);
}

// ------------------------------------------------------------------------------------------------

void FYapConversation::RemoveRunningFragment(FYapSpeechHandle FragmentHandle)
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

    OnConversationOpening.Broadcast();
    
    if (OpeningLocks.Num() == 0)
    {
        FinishOpening();
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

