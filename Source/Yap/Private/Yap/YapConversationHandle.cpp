// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapConversationHandle.h"

#include "Yap/YapSubsystem.h"

FYapConversationHandle::FYapConversationHandle()
{
    Guid = FGuid::NewGuid();
}

FYapConversationHandle::FYapConversationHandle(const FGuid& InGuid)
    : Guid(InGuid)
{
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClose(FYapConversationHandle Handle, FYapOnConversationClosed Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);

    Conversation.OnConversationClosed.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::SetOpenLock(FYapConversationHandle Handle)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);

    Conversation.SetOpenLock();
}

void UYapConversationHandleBlueprintFunctionLibrary::RemoveOpenLock(FYapConversationHandle Handle)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);

    Conversation.RemoveOpenLockAndAdvance();
}
