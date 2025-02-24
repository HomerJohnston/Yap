// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/YapConversation.h"
#include "Yap/YapSubsystem.h"

FYapConversationHandle::FYapConversationHandle()
{
    Guid = FGuid::NewGuid();
}

FYapConversationHandle::FYapConversationHandle(const FGuid& InGuid)
    : Guid(InGuid)
{
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpening(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationOpening.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpened(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationOpened.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosing(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosing.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosed(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosed.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::ApplyOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ApplyOpeningInterlock(LockObject);
}

void UYapConversationHandleBlueprintFunctionLibrary::ReleaseOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ReleaseOpeningInterlock(LockObject);
}

void UYapConversationHandleBlueprintFunctionLibrary::ApplyClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ApplyClosingInterlock(LockObject);
}

void UYapConversationHandleBlueprintFunctionLibrary::ReleaseClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ReleaseClosingInterlock(LockObject);
}

