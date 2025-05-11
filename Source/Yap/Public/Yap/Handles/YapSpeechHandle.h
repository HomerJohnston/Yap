// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapSpeechHandle.generated.h"

/** Since I can't store handles in structs by ref, I pass around a simpler version of it containing only the GUID and look up the actual handle (which could become "large" with more data) from the subsystem as needed. */
USTRUCT(BlueprintType)
struct YAP_API FYapSpeechHandle
{
    GENERATED_BODY()

    friend class UYapSpeechHandleBFL;
    
    // ==========================================
    // CONSTRUCTION
    // ==========================================
public:
	
    FYapSpeechHandle();

    FYapSpeechHandle(UWorld* InWorld, FGuid InGuid);

    ~FYapSpeechHandle();
    
    //FYapSpeechHandle(const FYapRunningFragment& RunningFragment);

    // ==========================================
    // STATE
    // ==========================================
private:
	
    UPROPERTY(Transient, BlueprintReadOnly, meta = (IgnoreForMemberInitializationTest, AllowPrivateAccess))
    FGuid Guid;

    UPROPERTY(Transient)
    TWeakObjectPtr<UWorld> World;

    UPROPERTY(Transient)
    bool bActive = true;
    
    // ==========================================
    // API
    // ==========================================
public:

    bool IsValid() const { return bActive && Guid.IsValid() && World.IsValid(); }

    const FGuid& GetGuid() const { return Guid; }

    bool SkipDialogue();

   // const TArray<FInstancedStruct>& GetFragmentData();

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
DECLARE_DYNAMIC_DELEGATE_TwoParams(FYapSpeechEventDelegate, UObject*, Broadcaster, FYapSpeechHandle, Handle);


/**
 * 
 */
UCLASS(DisplayName = "Yap Speech Handle Function Library")
class YAP_API UYapSpeechHandleBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static void BindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static void UnbindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool CancelSpeech(UObject* WorldContext, const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool CanSkipCurrently(UObject* WorldContext, const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool IsRunning(UObject* WorldContext, const FYapSpeechHandle& Handle);
    
    /*
    UFUNCTION(BlueprintCallable, Category = "Yap")
    static const TArray<FInstancedStruct>& GetFragmentData(const FYapSpeechHandle& HandleRef);
    */

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle")
    static void Invalidate(UPARAM(ref) FYapSpeechHandle& Handle)
    {
        Handle.bActive = false;
    };
    
    /** Returns true if the values are equal (A == B) */
    UFUNCTION(BlueprintPure, Category = "Yap|Speech Handle", meta=(DisplayName="Equal (YapSpeechHandle)", CompactNodeTitle="==", BlueprintThreadSafe))
    static bool EqualEqual_YapSpeechHandle(FYapSpeechHandle A, FYapSpeechHandle B);

    UFUNCTION(BlueprintCallable, Category = "Yap|Speech Handle")
    static FString ToString(const FYapSpeechHandle Handle);
};

