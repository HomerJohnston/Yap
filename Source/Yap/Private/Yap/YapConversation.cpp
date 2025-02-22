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

FYapConversation::~FYapConversation()
{
    OnConversationClosed.Broadcast();
}

void FYapConversation::AddRunningFragment(FYapFragmentHandle FragmentHandle)
{
    check(!RunningFragments.Contains(FragmentHandle));

    RunningFragments.Add(FragmentHandle);
}

void FYapConversation::RemoveRunningFragment(FYapFragmentHandle FragmentHandle)
{
    check(RunningFragments.Contains(FragmentHandle));

    RunningFragments.Remove(FragmentHandle);
}

void FYapConversation::SetOpenLock()
{
    bOpenLock = true;
}

void FYapConversation::RemoveOpenLockAndAdvance()
{
    bOpenLock = false;
    (void) UYapSubsystem::Get()->OnConversationFinishedOpening.ExecuteIfBound();
}

void FYapConversation::ExecuteSkip()
{
    if (RunningFragments.Num() == 0)
    {
        return;
    }
}

// ------------------------------------------------------------------------------------------------
