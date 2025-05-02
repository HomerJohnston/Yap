// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapConversationHandle.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FYapConversationEventDelegate);

USTRUCT(BlueprintType)
struct FYapConversationHandle
{
    GENERATED_BODY()

    // ==========================================
    // CONSTRUCTION
    // ==========================================
    
public:
	
    FYapConversationHandle();

    FYapConversationHandle(const FGuid& InGuid);

    // ==========================================
    // STATE
    // ==========================================
    
private:
    
    UPROPERTY(Transient)
    FGuid Guid;

    // ==========================================
    // API
    // ==========================================
    
public:

    bool IsValid() const { return Guid.IsValid(); }
    
    const FGuid& GetGuid() const { return Guid; }
    
    void Invalidate() { Guid.Invalidate(); }
    
    bool operator== (const FYapConversationHandle& Other) const;

    FString ToString() const
    {
        return Guid.ToString();
    }
};

FORCEINLINE uint32 GetTypeHash(const FYapConversationHandle& Struct)
{
    return GetTypeHash(Struct.GetGuid());
}

UCLASS()
class UYapConversationHandleBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = Yap, meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationOpening(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationOpened(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationClosing(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    UFUNCTION(BlueprintCallable, Category = Yap, meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationClosed(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    /**
     * Apply an interlock once a conversation starts opening to prevent the conversation from actually opening.
     * Use this to play animations or await other conditions before actually entering the conversation.
     */
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ApplyOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject);

    /**
     * Allow this conversation to open. This causes the conversation to open immediately, at this function call.
     */
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ReleaseOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject);

    /**
     * Apply an interlock once a conversation starts closing to prevent the conversation from actually closing.
     * Use this to play animations or await other conditions before actually closing the conversation.
     */
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ApplyClosingInterlock(FYapConversationHandle Handle, UObject* LockObject);
    
    /**
     * Allow this conversation to close. This causes the conversation to close immediately, at this function call.
     * If there is another queued conversation, it will open immediately; do not run additional closing logic for your UI pane after releasing an interlock!
     */
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ReleaseClosingInterlock(FYapConversationHandle Handle, UObject* LockObject);

    /**
     * Sends a skip signal to the current running conversation.
     */
    UFUNCTION(BlueprintCallable, Category = Yap, meta = (WorldContext = "Instigator"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle SkipDialogue(UObject* Instigator, FYapConversationHandle Handle);
};
