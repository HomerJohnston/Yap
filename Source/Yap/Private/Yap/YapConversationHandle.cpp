// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapConversationHandle.h"
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

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpening(FYapConversationHandle Handle, FYapConversationDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpened(FYapConversationHandle Handle, FYapConversationDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationOpened.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosing(FYapConversationHandle Handle, FYapConversationDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosing.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosed(FYapConversationHandle Handle, FYapConversationDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosed.Add(Delegate);
}

void UYapConversationHandleBlueprintFunctionLibrary::AddOpeningLock(FYapConversationHandle Handle, UObject* Lock)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.AddOpeningLock(Lock);
}

void UYapConversationHandleBlueprintFunctionLibrary::RemoveOpenLock(FYapConversationHandle Handle, UObject* Lock)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.RemoveOpeningLock(Lock);
}

void UYapConversationHandleBlueprintFunctionLibrary::SetClosingLock(FYapConversationHandle Handle, UObject* Lock)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.AddClosingLock(Lock);
}

void UYapConversationHandleBlueprintFunctionLibrary::RemoveClosingLock(FYapConversationHandle Handle, UObject* Lock)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.RemoveClosingLock(Lock);
}

