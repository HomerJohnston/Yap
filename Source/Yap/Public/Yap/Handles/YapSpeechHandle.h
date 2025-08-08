// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "YapSpeechHandle.generated.h"

// ================================================================================================

UENUM(BlueprintType)
enum class EYapSpeechCompleteResult : uint8
{
    Undefined UMETA(Hidden),
    Normal,
    Cancelled,
    Advanced,
};

/**
 * When a fragment (speech) begins running, you will be given a handle. You can use this handle to bind to events,
 * get fragment data, cancel the running speech, etc. using the function library.
 **/
USTRUCT(BlueprintType)
struct YAP_API FYapSpeechHandle
{
    GENERATED_BODY()

  //  friend class UYapSpeechHandleBFL;
    
    // ------------------------------------------
    // CONSTRUCTION
    // ------------------------------------------
public:
	
    FYapSpeechHandle();

    FYapSpeechHandle(UWorld* InWorld, FGuid InGuid);

    ~FYapSpeechHandle();
    
    //FYapSpeechHandle(const FYapRunningFragment& RunningFragment);

    // ------------------------------------------
    // STATE
    // ------------------------------------------
private:
    UPROPERTY(Transient, meta = (IgnoreForMemberInitializationTest))
    FGuid Guid;

    UPROPERTY(Transient)
    TWeakObjectPtr<UWorld> World;

    UPROPERTY(Transient)
    bool bActive = true;
    
    // ------------------------------------------
    // API
    // ------------------------------------------
public:

    bool IsValid() const { return bActive && Guid.IsValid() && World.IsValid(); }

    const FGuid& GetGuid() const { return Guid; }

    bool SkipDialogue();

    void Invalidate();

    bool operator== (const FYapSpeechHandle& Other) const;

    FString ToString() const
    {
        return Guid.ToString();
    }
};

FORCEINLINE uint32 GetTypeHash(const FYapSpeechHandle& Struct)
{
    return GetTypeHash(Struct.GetGuid());
}


UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FYapSpeechEventDelegate, UObject*, Broadcaster, const FYapSpeechHandle&, Handle, EYapSpeechCompleteResult, Result);

/**
 * Function library for speech handles.
 */
UCLASS(DisplayName = "Yap Speech Handle Function Library")
class YAP_API UYapSpeechHandleBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Bind a delegate to run when speech completes. This is when actual talking finishes not when any fragment padding finishes. */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (WorldContext = "WorldContext"))
    static void BindToOnSpeechComplete(UObject* WorldContext, UPARAM(ref) FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate);

    /** Unind a delegate for when speech completes (i.e. if you no longer want a delegate to run). */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (WorldContext = "WorldContext"))
    static void UnbindToOnSpeechComplete(UObject* WorldContext, UPARAM(ref) FYapSpeechHandle& Handle, FYapSpeechEventDelegate Delegate);

    /** Stops a running speech. This is intended to be used to halt free speech; to advance conversations, use the conversation handle. */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (WorldContext = "WorldContext"))
    static bool CancelSpeech(UObject* WorldContext, UPARAM(ref) FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (DefaultToSelf = "SpeechOwner"))
    static bool CancelSpeechByOwner(UObject* SpeechOwner);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (WorldContext = "WorldContext"))
    static bool AdvanceSpeech(UObject* WorldContext, UPARAM(ref) FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech", meta = (DefaultToSelf = "SpeechOwner"))
    static bool AdvanceSpeechByOwner(UObject* SpeechOwner);
    
    /** If speech is set as forced duration (unskippable) this will return false. This is intended to be used to help show/hide "Continue" style buttons. */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool CanSkip(UObject* WorldContext, const FYapSpeechHandle& Handle);

    /*
     * TODO URGENT
    UFUNCTION(BlueprintCallable, Category = "Yap")
    static const TArray<FInstancedStruct>& GetFragmentData(const FYapSpeechHandle& HandleRef);
    */

    /** Invalidates the speech handle. */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle")
    static void Invalidate(UPARAM(ref) FYapSpeechHandle& Handle);
    
    /** Returns true if the values are equal (A == B) */
    UFUNCTION(BlueprintPure, Category = "Yap|Speech Handle", meta=(DisplayName="Equal (YapSpeechHandle)", CompactNodeTitle="==", BlueprintThreadSafe))
    static bool EqualEqual_YapSpeechHandle(FYapSpeechHandle A, FYapSpeechHandle B);

    /** Returns the GUID of the handle. */
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle")
    static FString ToString(const FYapSpeechHandle Handle);
};

