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

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpening(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationOpening.Add(Delegate);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::BindToConversationOpened(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationOpened.Add(Delegate);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosing(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosing.Add(Delegate);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::BindToConversationClosed(FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.OnConversationClosed.Add(Delegate);
    
    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::ApplyOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ApplyOpeningInterlock(LockObject);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::ReleaseOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ReleaseOpeningInterlock(LockObject);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::ApplyClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ApplyClosingInterlock(LockObject);

    return Handle;
}

FYapConversationHandle UYapConversationHandleBlueprintFunctionLibrary::ReleaseClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    FYapConversation& Conversation = UYapSubsystem::GetConversation(Handle);
    Conversation.ReleaseClosingInterlock(LockObject);

    return Handle;
}

