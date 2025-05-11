// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapConversationHandle.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FYapConversationEventDelegate);

// ================================================================================================

/**
 * A conversation handle will be given to you when a conversation is opened and is used to interact with the conversation (advance it).
 * Use the blueprint function library functions to make use of the conversation handle.
 */
USTRUCT(BlueprintType)
struct YAP_API FYapConversationHandle
{
    GENERATED_BODY()

    // ------------------------------------------
    // CONSTRUCTION
    // ------------------------------------------
public:
	
    FYapConversationHandle();

    FYapConversationHandle(const FGuid& InGuid);

    // ------------------------------------------
    // STATE
    // ------------------------------------------
private:
    
    UPROPERTY(Transient, meta = (IgnoreForMemberInitializationTest))
    FGuid Guid;

    // ------------------------------------------
    // API
    // ------------------------------------------
public:

    bool IsValid() const { return Guid.IsValid(); }
    
    const FGuid& GetGuid() const { return Guid; }
    
    void Invalidate() { Guid.Invalidate(); }
    
    bool operator== (const FYapConversationHandle& Other) const;

    FString ToString() const
    {
        return Guid.ToString();
    }
    
    static const FYapConversationHandle& GetNullHandle()
    {
        static const FYapConversationHandle NullConversationHandle = FYapConversationHandle(FGuid(0, 0, 0, 0));
        return NullConversationHandle;
    }
};

FORCEINLINE uint32 GetTypeHash(const FYapConversationHandle& Struct)
{
    return GetTypeHash(Struct.GetGuid());
}

// ================================================================================================

/**
 * Function library for conversation handles.
 */
UCLASS(DisplayName = "Yap Conversation Handle Function Library")
class YAP_API UYapConversationHandleBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    /** Bind a delegate to a conversation opening event. This will be called when a "Open Conversation" flow node is entered. Use it to start opening your conversation UI. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationOpening(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    /** Bind a delegate to a conversation opened event. This will be called when a "Open Conversation" flow node is finished and triggering its output pin. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationOpened(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    /** Bind a delegate to a conversation closing event. This will be called when a "Close Conversation" flow node is entered. Use it to start closing your conversation UI. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationClosing(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    /** Bind a delegate to a conversation closed event. This will be called when a "Close Conversation" flow node is finished and triggering its output pin. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle BindToConversationClosed(UObject* WorldContext, FYapConversationHandle Handle, FYapConversationEventDelegate Delegate);

    /** Call this function to either skip running dialogue or to manually advance stepped dialogue. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (WorldContext = "WorldContext"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle AdvanceConversation(UObject* WorldContext, FYapConversationHandle Handle);
    
    /** Apply an interlock on Conversation Opening to prevent the "Open Conversation" flow node from finishing. Play animations or await other things before finally finishing the Open Conversation node. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ApplyOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject);

    /** Remove an interlock to allow an "Open Conversation" flow node to finish. When all interlocks are removed it will progress immediately. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ReleaseOpeningInterlock(FYapConversationHandle Handle, UObject* LockObject);

    /** Apply an interlock on Conversation Closing to prevent the "Close Conversation" flow node from finishing. Play animations or await other things before finally finishing the Close Conversation node. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ApplyClosingInterlock(FYapConversationHandle Handle, UObject* LockObject);
    
    /** Remove an interlock to allow a "Close Conversation" flow node to finish. When all interlocks are removed it will progress immediately. */
    UFUNCTION(BlueprintCallable, Category = "Yap|ConversationHandle", meta = (DefaultToSelf = "LockObject"))
    static UPARAM(DisplayName = Handle) FYapConversationHandle ReleaseClosingInterlock(FYapConversationHandle Handle, UObject* LockObject);
};
