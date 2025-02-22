// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"

#include "YapRunningFragment.generated.h"

struct FYapFragment;
struct FInstancedStruct;
class UObject;
class UFlowNode_YapDialogue;

DECLARE_MULTICAST_DELEGATE(FOnFinish);

// ================================================================================================

/** Since I can't store handles in structs by ref, I pass around a simpler version of it containing only the GUID and look up the actual handle (which could become "large" with more data) from the subsystem as needed. */
USTRUCT(BlueprintType)
struct YAP_API FYapFragmentHandle
{
	GENERATED_BODY()

	// ==========================================
	// CONSTRUCTION
	// ==========================================
public:
	
	FYapFragmentHandle();

	FYapFragmentHandle(const FYapRunningFragment& RunningFragment);

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
	
	const TArray<FInstancedStruct>& GetFragmentData();
	
	bool operator== (const FYapFragmentHandle& Other) const;
};

FORCEINLINE uint32 GetTypeHash(const FYapFragmentHandle& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}

// ================================================================================================

/** Handle for a running fragment. Can be used to instruct the Yap Subsystem to interrupt a piece of dialogue, unless it is marked not skippable. */
USTRUCT(BlueprintType)
struct YAP_API FYapRunningFragment
{
	GENERATED_BODY()

	// ==========================================
	// CONSTRUCTION
	// ==========================================
public:
	
	FYapRunningFragment();

	~FYapRunningFragment();

	static FYapRunningFragment& InvalidHandle() { return _InvalidHandle; }

	// ==========================================
	// STATE
	// ==========================================
private:
	UPROPERTY(Transient)
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(Transient)
	uint8 FragmentIndex = 0;
	
	static FYapRunningFragment _InvalidHandle;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess, IgnoreForMemberInitializationTest))
	FGuid Guid;

	UPROPERTY(Transient)
	FTimerHandle SpeechTimerHandle;
	
	UPROPERTY(Transient)
	FTimerHandle FragmentTimerHandle;
	
public:
	/**  */
	TMulticastDelegate<void()> OnFinish;
	
	// ==========================================
	// API
	// ==========================================
public:
	const FGuid& GetGuid() const { return Guid; }

	const UFlowNode_YapDialogue* GetDialogueNode() const { return DialogueNode; }

	uint8 GetFragmentIndex() const { return FragmentIndex; }

	const FYapFragment& GetFragment() const;
	
	void OnSpeakingEnds() const;
	
	void Invalidate();

	void SetSpeechTimerHandle(FTimerHandle InSpeechTimerHandle);

	void SetFragmentTimerHandle(FTimerHandle InFragmentTimerHandle);
	
	bool IsValid() const { return Guid.IsValid(); }

	bool operator== (const FYapRunningFragment& Other) const;
};

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapRunningSpeech
{
	GENERATED_BODY()
	
};

// ================================================================================================

FORCEINLINE uint32 GetTypeHash(const FYapRunningFragment& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}