// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapRunningFragment.generated.h"

struct FYapFragment;
struct FInstancedStruct;
class UObject;
class UFlowNode_YapDialogue;

DECLARE_MULTICAST_DELEGATE(FOnFinish);

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