// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapConversationHandle.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FYapConversationDelegate);

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
    static void BindToConversationOpening(FYapConversationHandle Handle, FYapConversationDelegate Delegate);
    
    UFUNCTION(BlueprintCallable)
    static void BindToConversationOpened(FYapConversationHandle Handle, FYapConversationDelegate Delegate);
    
    UFUNCTION(BlueprintCallable)
    static void BindToConversationClosing(FYapConversationHandle Handle, FYapConversationDelegate Delegate);

    UFUNCTION(BlueprintCallable)
    static void BindToConversationClosed(FYapConversationHandle Handle, FYapConversationDelegate Delegate);

    UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Lock"))
    static void AddOpeningLock(FYapConversationHandle Handle, UObject* Lock);

    UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Lock"))
    static void RemoveOpenLock(FYapConversationHandle Handle, UObject* Lock);

    UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Lock"))
    static void SetClosingLock(FYapConversationHandle Handle, UObject* Lock);
    
    UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Lock"))
    static void RemoveClosingLock(FYapConversationHandle Handle, UObject* Lock);
};
