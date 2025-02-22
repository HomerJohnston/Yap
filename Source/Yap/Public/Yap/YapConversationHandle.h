// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapConversationHandle.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FYapOnConversationClosed);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FYapOnConversationClosed_Multi);

USTRUCT(BlueprintType)
struct FYapConversationHandle
{
    GENERATED_BODY()

    FYapConversationHandle();

    FYapConversationHandle(const FGuid& InGuid);
    
    UPROPERTY(Transient)
    FGuid Guid;
};

UCLASS()
class UYapConversationHandleBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable)
    static void BindToConversationClose(FYapConversationHandle Handle, FYapOnConversationClosed Delegate);
};