// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/Handles/YapRunningFragment.h"
#include "YapSpeechHandle.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FYapSpeechEventDelegate);

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

    FYapSpeechHandle(const FYapRunningFragment& RunningFragment);

    // ==========================================
    // STATE
    // ==========================================
private:
	
    UPROPERTY(Transient, meta = (IgnoreForMemberInitializationTest))
    FGuid Guid;

    // ==========================================
    // API
    // ==========================================
public:

    bool IsValid() const { return Guid.IsValid(); }

    const FGuid& GetGuid() const { return Guid; }

    bool SkipDialogue();

    void AddReactor(UObject* Reactor);
	
   // const TArray<FInstancedStruct>& GetFragmentData();
	
    bool operator== (const FYapSpeechHandle& Other) const;

    void BindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const;

    void UnbindToOnSpeechComplete(FYapSpeechEventDelegate Delegate) const;
};

FORCEINLINE uint32 GetTypeHash(const FYapSpeechHandle& Struct)
{
    return GetTypeHash(Struct.GetGuid());
}


/**
 * 
 */
UCLASS()
class YAP_API UYapSpeechHandleBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable)
    static void BindToOnSpeechComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);

    UFUNCTION(BlueprintCallable)
    static void UnbindToOnSpeechComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable)
    static void BindToOnFragmentComplete(FYapSpeechHandle Handle, FYapSpeechEventDelegate Delegate);
    
    UFUNCTION(BlueprintCallable, Category = "Yap")
    static bool SkipDialogue(const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap")
    static bool CanSkipCurrently(const FYapSpeechHandle& Handle);

    UFUNCTION(BlueprintCallable, Category = "Yap")
    static void AddReactor(UPARAM(ref) FYapSpeechHandle& HandleRef, UObject* Reactor);

    /*
    UFUNCTION(BlueprintCallable, Category = "Yap")
    static const TArray<FInstancedStruct>& GetFragmentData(const FYapSpeechHandle& HandleRef);
    */
    
    /** Returns true if the values are equal (A == B) */
    UFUNCTION(BlueprintPure, meta=(DisplayName="Equal (YapSpeechHandle)", CompactNodeTitle="==", BlueprintThreadSafe), Category="Yap")
    static bool EqualEqual_YapSpeechHandle( FYapSpeechHandle A, FYapSpeechHandle B );
};

