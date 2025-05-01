// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/Handles/YapRunningFragment.h"
#include "YapSpeechHandle.generated.h"

/** Since I can't store handles in structs by ref, I pass around a simpler version of it containing only the GUID and look up the actual handle (which could become "large" with more data) from the subsystem as needed. */
USTRUCT(BlueprintType)
struct YAP_API FYapSpeechHandle
{
    GENERATED_BODY()

    // ==========================================
    // CONSTRUCTION
    // ==========================================
public:
	
    FYapSpeechHandle();

    FYapSpeechHandle(UWorld* InWorld, const FGuid& InGuid);

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

    // ==========================================
    // API
    // ==========================================
public:

    bool IsValid() const { return Guid.IsValid() && World.IsValid(); }

    const FGuid& GetGuid() const { return Guid; }

    bool SkipDialogue();

   // const TArray<FInstancedStruct>& GetFragmentData();

    void Invalidate();

    bool operator== (const FYapSpeechHandle& Other) const;

    // TODO I would prefer to use these instead of calling the BPFL functions below, but circular reference. Solve later. Use the BPFL funcs below for now instead. 
    //void BindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const;

    //void UnbindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const;

    //void BindToOnFragmentComplete(FYapSpeechEventDelegate Delegate) const;

    //void UnbindToOnFragmentComplete(FYapSpeechEventDelegate Delegate) const;

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
UCLASS()
class YAP_API UYapSpeechHandleBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle", meta = (WorldContext = "WorldContext"))
    static void BindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);

    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle", meta = (WorldContext = "WorldContext"))
    static void UnbindToOnSpeechComplete(UObject* WorldContext, FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool SkipDialogue(UObject* WorldContext, const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool CanSkipCurrently(UObject* WorldContext, const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle", meta = (WorldContext = "WorldContext"))
    static bool IsRunning(UObject* WorldContext, const FYapSpeechHandle& Handle);
    
    /*
    UFUNCTION(BlueprintCallable, Category = "Yap")
    static const TArray<FInstancedStruct>& GetFragmentData(const FYapSpeechHandle& HandleRef);
    */

    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle")
    static void Invalidate(UPARAM(ref) FYapSpeechHandle& Handle)
    {
        Handle.Invalidate();
    };
    
    /** Returns true if the values are equal (A == B) */
    UFUNCTION(BlueprintPure, meta=(DisplayName="Equal (YapSpeechHandle)", CompactNodeTitle="==", BlueprintThreadSafe), Category="Yap")
    static bool EqualEqual_YapSpeechHandle(FYapSpeechHandle A, FYapSpeechHandle B);

    UFUNCTION(BlueprintCallable, Category = "Yap Speech Handle")
    static FString ToString(const FYapSpeechHandle Handle);
};

