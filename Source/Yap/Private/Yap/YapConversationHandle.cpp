// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/YapConversation.h"
#include "Yap/YapSubsystem.h"

FYapConversationHandle::FYapConversationHandle()
{
//    UE_LOG(LogTemp, Display, TEXT("Default convo handle"));
}

FYapConversationHandle::FYapConversationHandle(const FGuid& InGuid)
    : Guid(InGuid)
{
}

FYapConversationHandle::FYapConversationHandle(UObject* Owner, FName Name)
{
    Guid = FGuid(GetTypeHash(Owner->GetName()), GetTypeHash(Name), 0, 0);
}

bool FYapConversationHandle::operator==(const FYapConversationHandle& Other) const
{
    return Guid == Other.Guid;
}

FYapConversationHandle UYapConversationHandleBFL::BindToConversationOpening(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(WorldContext, Handle))
    {
        Conversation->OnConversationOpening.Add(Delegate);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::BindToConversationOpened(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(WorldContext, Handle))
    {
        Conversation->OnConversationOpened.Add(Delegate);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::BindToConversationClosing(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(WorldContext, Handle))
    {
        Conversation->OnConversationClosing.Add(Delegate);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::BindToConversationClosed(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(WorldContext, Handle))
    {
        Conversation->OnConversationClosed.Add(Delegate);
    }
    
    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::AdvanceConversation(UObject* WorldContext, FYapConversationHandle Handle)
{
    UYapSubsystem::AdvanceConversation(WorldContext, Handle);
    
    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::ApplyOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(LockObject, Handle))
    {
        Conversation->ApplyOpeningInterlock(LockObject);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::ReleaseOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(LockObject, Handle))
    {
        Conversation->ReleaseOpeningInterlock(LockObject);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::ApplyClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(LockObject, Handle))
    {
        Conversation->ApplyClosingInterlock(LockObject);
    }

    return Handle;
}

FYapConversationHandle UYapConversationHandleBFL::ReleaseClosingInterlock(FYapConversationHandle Handle, UObject* LockObject)
{
    if (FYapConversation* Conversation = UYapSubsystem::GetConversationByHandle(LockObject, Handle))
    {
        Conversation->ReleaseClosingInterlock(LockObject);        
    }

    return Handle;
}

// TODO error logging?