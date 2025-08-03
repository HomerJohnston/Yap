// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "YapBit.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
	#include "InstancedStruct.h"
#else
	#include "StructUtils/InstancedStruct.h"
#endif

#include "Nodes/FlowPin.h"

#include "YapFragment.generated.h"

enum class EYapAutoAdvanceFlags : uint8;
enum class EYapInterruptibleFlags : uint8;
class IYapCharacterInterface;
enum class EYapLoadContext : uint8;
class UYapCharacterAsset;
class UYapCondition;
class UFlowNode_YapDialogue;
struct FFlowPin;
enum class EYapMaturitySetting : uint8;

// ================================================================================================

UENUM()
enum class EYapFragmentRunState : uint8
{
	Idle		= 0,
	Running		= 1,
	//InPadding	= 2,
};

// ================================================================================================

UENUM()
enum class EYapFragmentEntryStateFlags : uint8
{
	NeverRan =	0,
	Failed =	1 << 0,
	Success =	1 << 1,
	Skipped =	1 << 2,
};

inline EYapFragmentEntryStateFlags operator|(EYapFragmentEntryStateFlags Left, EYapFragmentEntryStateFlags Right)
{
	return static_cast<EYapFragmentEntryStateFlags>(static_cast<uint8>(Left) | static_cast<uint8>(Right));
}

// ================================================================================================

/**
 * Fragments contain all of the actual data and settings required for a segment of speech to run.
 * 
 * Fragment settings override any defaults provided by the parent node.
 */
USTRUCT(NotBlueprintType)
struct YAP_API FYapFragment
{
	GENERATED_BODY()

public:
	FYapFragment();

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class UFlowGraphNode_YapDialogue;
	friend class SYapDialogueEditor;

	// TODO this should be removed eventually (probably early 2026). I am only putting this in to allow deprecation of SpeakerAsset and DirectedAtAsset.
	friend class UFlowNode_YapDialogue;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY()
	TArray<TObjectPtr<UYapCondition>> Conditions;

	UPROPERTY()
	TSoftObjectPtr<UObject> SpeakerAsset;

	UPROPERTY()
	FGameplayTag Speaker;
	
	UPROPERTY()
	TSoftObjectPtr<UObject> DirectedAtAsset;

	UPROPERTY()
	FGameplayTag DirectedAt;
	
	UPROPERTY()
	FYapBit MatureBit;

	UPROPERTY()
	FYapBit ChildSafeBit;
	
	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY()
	int32 ActivationLimit = 0;

	/**  */
	UPROPERTY()
	FName FragmentID;

	/**  */
	UPROPERTY()
	FName AudioID;
	
	/** Padding is idle time to wait after the fragment finishes running. An unset value will use project defaults. */
	UPROPERTY()
	TOptional<float> Padding;
	
	/**  */
	UPROPERTY()
	TOptional<EYapInterruptibleFlags> InterruptibleFlags;
	
	/**  */
	UPROPERTY()
	TOptional<EYapAutoAdvanceFlags> AutoAdvanceFlags;
	
	/** Indicates whether child-safe data is available in this bit or not */
	UPROPERTY()
	bool bEnableChildSafe = false;
	
	UPROPERTY()
	bool bShowOnStartPin = false;

	UPROPERTY()
	bool bShowOnEndPin = false;
	
	/**  */
	UPROPERTY()
	FGameplayTag MoodTag;

    UPROPERTY(EditAnywhere, Category = "Default")
	TArray<FInstancedStruct> Data;
	
	/**  */
	UPROPERTY()
	EYapTimeMode TimeMode;
	
	// ==========================================
	// STATE
protected:
    UPROPERTY(VisibleAnywhere, meta=(IgnoreForMemberInitializationTest), Category = "Default")
	FGuid Guid;
	
	// TODO should this be serialized or transient
	UPROPERTY(Transient)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(Transient)
	int32 ActivationCount = 0;
	
	UPROPERTY()
	FFlowPin PromptPin;

	UPROPERTY()
	FFlowPin StartPin;

	UPROPERTY()
	FFlowPin EndPin;

	UPROPERTY(Transient)
	EYapFragmentRunState RunState = EYapFragmentRunState::Idle;

	UPROPERTY(Transient)
	EYapFragmentEntryStateFlags LastEntryState = EYapFragmentEntryStateFlags::NeverRan;

	/** When was the current running fragment started? */
	UPROPERTY(Transient)
	double StartTime = -1;

	/** When did the most recently ran fragment finish? */
	UPROPERTY(Transient)
	double EndTime = -1;

	/**  */
	UPROPERTY(Transient)
	bool bRunning = false;
	
	/**  */
	UPROPERTY(Transient)
	bool bFragmentAwaitingManualAdvance = false;
	
public:
	
	/**  */
	UPROPERTY(Transient)
	FTimerHandle PaddingTimerHandle;
	
	// ASSET LOADING
protected:
	
	/**
	 * 
	 */
	TSharedPtr<FStreamableHandle> SpeakerHandle;
	
	TSharedPtr<FStreamableHandle> DirectedAtHandle;

	// EDITOR
#if WITH_EDITOR

#endif
	
	// ==========================================
	// API
public:
	bool CanRun() const;
	
	bool CheckConditions() const;
	
	void ResetOptionalPins();
	
	void PreloadContent(UWorld* World, EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext);
	
	const FGameplayTag& GetSpeakerTag() const;
	
	const FGameplayTag& GetDirectedAtTag() const;

	const TScriptInterface<IYapCharacterInterface> GetSpeakerCharacter(UWorld* World, EYapLoadContext LoadContext); // Non-const because of async loading handle

	bool HasDeprecatedSpeakerAsset();
	
	bool HasSpeakerAssigned();

	//const TSoftObjectPtr<const UObject> GetCharacterAsset(UWorld* World, const FGameplayTag& CharacterTag, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext) const;
	
	const TScriptInterface<IYapCharacterInterface> GetDirectedAt(UWorld* World, EYapLoadContext LoadContext); // Non-const because of async loading handle
	
private:
	const TScriptInterface<IYapCharacterInterface> GetCharacter_Internal(UWorld* World, const FGameplayTag& CharacterTag, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext);

public:
	// TODO I don't think fragments should know where their position is!
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }

	void SetRunState(EYapFragmentRunState NewState) { RunState = NewState; }
	
	EYapFragmentRunState GetRunState() const { return RunState; }

	void SetEntryState(EYapFragmentEntryStateFlags NewStateFlags) { LastEntryState = (EYapFragmentEntryStateFlags)NewStateFlags; }
	
	EYapFragmentEntryStateFlags GetLastEntryState() const { return LastEntryState; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	bool CheckActivationLimit() const { if (ActivationLimit <= 0) return true; return ActivationCount < ActivationLimit; }

	bool IsActivationLimitMet() const { if (ActivationLimit <= 0) return false; return ActivationCount >= ActivationLimit; }

	const FText& GetDialogueText(UWorld* World, EYapMaturitySetting MaturitySetting) const;
	
	const FText& GetTitleText(UWorld* World, EYapMaturitySetting MaturitySetting) const;
	
	const UObject* GetAudioAsset(UWorld* World, EYapMaturitySetting MaturitySetting) const;
	
	const FYapBit& GetBit(UWorld* World) const;

	const FYapBit& GetBit(UWorld* World, EYapMaturitySetting MaturitySetting) const;

	const FYapBit& GetMatureBit() const { return MatureBit; }

	const FYapBit& GetChildSafeBit() const { return ChildSafeBit; }

	FYapBit& GetMatureBitMutable() { return MatureBit; }

	FYapBit& GetChildSafeBitMutable() { return ChildSafeBit; }

	TOptional<float> GetSpeechTime(UWorld* World, const UYapNodeConfig& NodeConfig) const;

	double GetStartTime() const { return StartTime; }

	void SetStartTime(double InTime) { StartTime = InTime; }

	double GetEndTime() const { return EndTime; }

	void SetEndTime(double InTime) { EndTime = InTime; }

	bool IsAwaitingManualAdvance() const;

	void SetAwaitingManualAdvance();
	
	void ClearAwaitingManualAdvance();

	TOptional<float> GetSpeechTime(UWorld* World, EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext, const UYapNodeConfig& NodeConfig) const;
	
public:
	TOptional<float> GetPaddingSetting() const { return Padding; };
	
	float GetPaddingValue(UWorld* World, const UYapNodeConfig& NodeConfig) const;

	bool GetUsesPadding(UWorld* World, const UYapNodeConfig& NodeConfig) const;

	float GetProgressionTime(UWorld* World, const UYapNodeConfig& NodeConfig) const;
	
	void IncrementActivations();

	const FName& GetFragmentID() const { return FragmentID; } 

	const FName& GetAudioID() const { return AudioID; }
	
	// TODO - want to design a better system for this.
	//void ReplaceBit(EYapMaturitySetting MaturitySetting, const FYapBitReplacement& ReplacementBit);

	const FGuid& GetGuid() const { return Guid; }

	bool UsesStartPin() const { return bShowOnStartPin; }

	bool UsesEndPin() const { return bShowOnEndPin; }

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }

	FFlowPin GetPromptPin() const;

	FFlowPin GetEndPin() const;

	FFlowPin GetStartPin() const;;

	void ResolveMaturitySetting(UWorld* World, EYapMaturitySetting& MaturitySetting) const;
	
	TOptional<EYapInterruptibleFlags> GetSkippableSetting() const { return InterruptibleFlags; }
	
	TOptional<EYapInterruptibleFlags>& GetSkippableSetting() { return InterruptibleFlags; }
	
	TOptional<EYapAutoAdvanceFlags> GetAutoAdvanceSetting() const { return AutoAdvanceFlags; }
	
	TOptional<EYapAutoAdvanceFlags>& GetAutoAdvanceSetting() { return AutoAdvanceFlags; }
	
	/** Gets the evaluated skippable setting to be used for this fragment (incorporating project default settings and fallbacks) */
	bool GetInterruptible(bool Default, bool bInConversation) const;
	
	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode(UWorld* World, const UYapNodeConfig& NodeConfig) const;
	
	EYapTimeMode GetTimeMode(UWorld* World, EYapMaturitySetting MaturitySetting, const UYapNodeConfig& NodeConfig) const;

	FGameplayTag GetMoodTag() const { return MoodTag; }

	const TArray<FInstancedStruct>& GetData() const { return Data; }
	
	bool IsTimeModeNone() const;

	bool HasAudio() const;

	bool HasData() const;

#if WITH_EDITOR
public:
	FYapBit& GetBitMutable(EYapMaturitySetting MaturitySetting);
		
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	void SetPaddingToNextFragment(float NewValue) { Padding = NewValue; }

	TArray<TObjectPtr<UYapCondition>>& GetConditionsMutable() { return Conditions; }

	void ResetGUID() { Guid = FGuid::NewGuid(); }
	
	FName GetPromptPinName() const { return GetPromptPin().PinName; }

	FName GetEndPinName() const { return GetEndPin().PinName; }

	FName GetStartPinName() const { return GetStartPin().PinName; }

	void ResetEndPin() { bShowOnEndPin = false; }

	void ResetStartPin() { bShowOnStartPin = false; }
	
	void InvalidateFragmentTag(UFlowNode_YapDialogue* OwnerNode);

	void SetMoodTag(const FGameplayTag& NewValue) { MoodTag = NewValue; };

	void SetTimeModeSetting(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	EYapTimeMode GetTimeModeSetting() const { return TimeMode; }
	// TODO implement this
	bool GetBitReplaced() const { return false; };
#endif

	
	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(const FGameplayTag& CharacterTag);
	
	void SetDirectedAt(const FGameplayTag& CharacterTag);
#endif
};