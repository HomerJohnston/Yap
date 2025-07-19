// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/YapNodeConfig.h"
#include "Yap/YapFragment.h"
#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"

#include "FlowNode_YapDialogue.generated.h"

class UYapCharacterAsset;

// ------------------------------------------------------------------------------------------------
/**
 * Determines how a Talk node evaluates. Player Prompt nodes don't use this.
 */
UENUM()
enum class EYapDialogueTalkSequencing : uint8
{
	RunAll				UMETA(ToolTip = "The node will always try to run every fragment. The node will execute the Out pin after it finishes trying to run all fragments."), 
	RunUntilFailure		UMETA(ToolTip = "The node will attempt to run every fragment. If any one fails, the node will execute the Out pin."),
	SelectOne			UMETA(ToolTip = "The node will attempt to run every fragment. If any one passes, the node will execute the Out pin."),
	SelectRandom		UMETA(ToolTip = "The node will randomly run one available fragment, then execute the Out pin."),
	COUNT				UMETA(Hidden)
};

// ------------------------------------------------------------------------------------------------
/**
 * Node type. Freestyle talking or player prompt. Changes the execution flow of dialogue.
 */
UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = true))
enum class EYapDialogueNodeType : uint8
{
	Talk			= 1 << 0,
	TalkAndAdvance	= 1 << 1	UMETA(DisplayName = "Talk & Advance"),
	PlayerPrompt	= 1 << 2,
	COUNT			= 1 << 3	UMETA(Hidden)
};

USTRUCT()
struct FYapFragmentRunData
{
	GENERATED_BODY()
	
	FYapFragmentRunData() {}

	FYapFragmentRunData(uint8 InFragmentIndex, FTimerHandle InSpeechTimerHandle, FTimerHandle InPaddingTimerHandle)
		: FragmentIndex(InFragmentIndex)
		, SpeechTimerHandle(InSpeechTimerHandle)
		, PaddingTimerHandle(InPaddingTimerHandle)
	{}

	UPROPERTY(Transient)
	int32 FragmentIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FTimerHandle SpeechTimerHandle;
	
	UPROPERTY(Transient)
	FTimerHandle PaddingTimerHandle;
};

// ------------------------------------------------------------------------------------------------
/**
 * Subclass this to create different "domains" for yap nodes. It's recommended to use the base class for common dialogue, and add subclasses for other dialogue-like features such as tutorial popups.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Dialogue", Keywords = "yap")) /*, ToolTip = "Emits Yap dialogue events"*/
class YAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class SYapConditionDetailsViewWidget;
	friend class UFlowGraphNode_YapDialogue;
#endif
	friend struct FYapDialogueActiveSmartObject;

	// TODO should I get rid of this?
	friend class UYapSubsystem;

public:
	UFlowNode_YapDialogue();

	// TODO constexpr?
	static FName OutputPinName;
	
	static FName BypassPinName;
	
	// ============================================================================================
	// SETTINGS
	// ============================================================================================
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UYapNodeConfig> Config;
	
	/** What type of node we are. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueNodeType DialogueNodeType;

	/** Maximum number of times we can successfully enter & exit this node. Any further attempts will trigger the Bypass output. */
	UPROPERTY(BlueprintReadOnly)
	int32 NodeActivationLimit;

	/** Controls how Talk nodes flow. See EYapDialogueTalkSequencing. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueTalkSequencing TalkSequencing;

	// This is used to update the fragment sequencing mode when the talk mode is changed
	EYapDialogueTalkSequencing OldTalkSequencing;

	/** Controls if dialogue can be interrupted. */
	UPROPERTY(BlueprintReadOnly)
	TOptional<bool> Skippable;

	/** Controls if dialogue automatically advances (only applicable if it has a time duration set). */
	UPROPERTY(BlueprintReadOnly)
	TOptional<bool> AutoAdvance;

	/** Tags can be used to interact with this dialogue node during the game. Dialogue nodes can be looked up and/or modified by UYapSubsystem by their tag. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;

	/** Conditions which must be met for this dialogue to run. All conditions must pass (AND, not OR evaluation). If any conditions fail, Bypass output is triggered. */
	UPROPERTY(Instanced, BlueprintReadOnly)
	TArray<TObjectPtr<UYapCondition>> Conditions;

	/** Unique node ID for audio system. */
	UPROPERTY(EditInstanceOnly)
	FString AudioID = "";
	
	/** Actual dialogue contents. */
	UPROPERTY(EditInstanceOnly)
	TArray<FYapFragment> Fragments;

	/** Whether the dialogue data of this bit can be edited. Dialogue should be locked after exporting a .PO file for translators to make it harder to accidentally edit source text. */
	// Placeholder - not implemented yet
	//UPROPERTY()
	//bool bLocked = false;
	
	// ============================================================================================
	// STATE
	// ============================================================================================
	
protected:
	// TODO what happens if I enter a node before it finishes playing? Should I use the activation count to reseed new GUIDs for fragments? This is an extreme edge case
	/** How many times this node has been successfully ran. */
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	/** The most recent running fragment */
	UPROPERTY(Transient)
	TOptional<uint8> FocusedFragmentIndex;

	/** Flow doesn't give me a nice way to prevent entering this node twice. */
	UPROPERTY(Transient)
	bool bNodeActive = false;
	
	/**  */
	UPROPERTY(Transient)
	TMap<FYapPromptHandle, uint8> PromptIndices;

	/** The most recent running speech handle */
	UPROPERTY(Transient)
	FYapSpeechHandle FocusedSpeechHandle;

	/** Fragments which are either speaking or in padding time will be in here */
	UPROPERTY(Transient)
	TMap<FYapSpeechHandle, uint8> RunningFragments;

	/** Fragments which are in active speech (including after passing a negative padding time) will be in here */
	UPROPERTY(Transient)
	TSet<FYapSpeechHandle> SpeakingFragments;

	/**  */
	UPROPERTY(Transient)
	TSet<FYapSpeechHandle> FragmentsInPadding;

	// ============================================================================================
	// PUBLIC API
	// ============================================================================================

public:
	/**  */
	EYapDialogueNodeType GetNodeType() const { return DialogueNodeType; }

	/** Is this dialogue a Talk node or a Player Prompt node? */
	bool IsPlayerPrompt() const { return DialogueNodeType == EYapDialogueNodeType::PlayerPrompt; }

	bool HasValidConfig() const;

	const UYapNodeConfig& GetNodeConfig() const;
	
	/** Does this node use title text? */
	bool UsesTitleText() const;

	/** How many times has this dialogue node successfully ran? */
	int32 GetNodeActivationCount() const { return NodeActivationCount; }

	/** How many times is this dialogue node allowed to successfully run? */
	int32 GetNodeActivationLimit() const { return NodeActivationLimit; }

	const FYapFragment& GetFragment(uint8 FragmentIndex) const;
	
	/** Dialogue fragments getter. */
	const TArray<FYapFragment>& GetFragments() const { return Fragments; }

	/** Simple helper function. */
	uint8 GetNumFragments() const { return Fragments.Num(); }

	/** Is dialogue from this node skippable by default? */
	bool GetSkippable() const;

	bool GetNodeAutoAdvance() const;
	
	bool GetFragmentAutoAdvance(uint8 FragmentIndex) const;

	int32 GetRunningFragmentIndex() const { return FocusedFragmentIndex.Get(INDEX_NONE); }
	
	// TODO this sucks can I register the fragments some other way instead
	/** Finds the first fragment on this dialogue containing a tag. */
	FYapFragment* FindTaggedFragment(const FGameplayTag& Tag);

protected:
	bool CanSkip(FYapSpeechHandle Handle) const;

public:
	FString GetAudioID() const { return AudioID; }

protected:
	bool CheckActivationLimits() const;

#if WITH_EDITOR
private:
	TOptional<bool> GetSkippableSetting() const;
	
	void InvalidateFragmentTags();

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }
	
	TArray<UYapCondition*>& GetConditionsMutable() { return MutableView(Conditions); }

	bool ToggleNodeType(bool bHasOutputConnections);
#endif
	
	// ============================================================================================
	// OVERRIDES
	// ============================================================================================

protected:
#if WITH_EDITOR
	/** UFlowNodeBase override */
	FString GetStatusString() const override;

	/** UFlowNodeBase override */
	FString GetNodeDescription() const override;
#endif
	/** UFlowNodeBase override */
	void InitializeInstance() override;

	/** UFlowNodeBase override */
	void ExecuteInput(const FName& PinName) override;

	/** UFlowNode override */
	void OnPassThrough_Implementation() override;

	// ============================================================================================
	// INTERNAL API
	// ============================================================================================
	
protected:
	bool CanEnterNode();

	bool CheckConditions();

	bool TryBroadcastPrompts();

	void RunPrompt(uint8 Uint8);
	
	bool TryStartFragments();

	bool RunFragment(uint8 FragmentIndex);

public:
	/** This gets run by the subsystem when the actual speaking finishes */
	UFUNCTION()
	void OnSpeechComplete(UObject* Instigator, FYapSpeechHandle Handle);

	/** This gets run by the dialogue node's own timers, only if the fragment is set to use padding */
	UFUNCTION()
	void OnPaddingComplete(FYapSpeechHandle Handle);

	/** Called when a fragment is done running - speech is done AND padding is done */
	void FinishFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex);

	/** This should be called whenever speech finishes OR padding finishes */
	void TryAdvanceFromFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex);

	/** Forcefully begins playing the next fragment or triggers the output node */
	void AdvanceFromFragment(const FYapSpeechHandle& Handle, uint8 FragmentIndex);
	
	bool IsBypassPinRequired() const;

	bool IsOutputConnectedToPromptNode() const;
	
	int16 FindFragmentIndex(const FGuid& InFragmentGuid) const;

	const FYapFragment& GetFragmentByIndex(uint8 Index) const;

#if WITH_EDITOR
	FYapFragment& GetFragmentMutableByIndex(uint8 Index);
#endif

	
protected:
	void TriggerSpeechStartPin(uint8 FragmentIndex);

	void TriggerSpeechEndPin(uint8 FragmentIndex);

	void BindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle);

	void UnbindToSubsystemSpeechCompleteEvent(const FYapSpeechHandle& Handle);
	
	bool FragmentCanRun(uint8 FragmentIndex);
	
	
	UFUNCTION()
	void OnPromptChosen(UObject* Instigator, FYapPromptHandle Handle);
	
	UFUNCTION()
	void OnCancel(UObject* Instigator, FYapSpeechHandle Handle);

	UFUNCTION()
	void OnAdvanceConversation(UObject* Instigator, FYapConversationHandle Handle);
	
	void FinishNode(FName OutputPinToTrigger);

	void SetActive();

	void SetInactive();

	/** Fragment access */
public:
	TOptional<float> GetSpeechTime(uint8 FragmentIndex) const;

#if WITH_EDITOR
	TOptional<float> GetSpeechTime(uint8 FragmentIndex, EYapMaturitySetting Maturity, EYapLoadContext LoadContext) const;
#endif
	
	float GetPadding(uint8 FragmentIndex) const;
	
#if WITH_EDITOR
public:
	TArray<FYapFragment>& GetFragmentsMutable();
	
private:
	FYapFragment& GetFragmentByIndexMutable(uint8 Index);
	
	void RemoveFragment(int32 Index);

	FText GetNodeTitle() const override;
	
	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EYapDialogueTalkSequencing GetMultipleFragmentSequencing() const;
	
	TArray<FFlowPin> GetContextOutputs() const override;

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);
	
public:
	const FGameplayTag& GetDialogueTag() const { return DialogueTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;
	
	void ForceReconstruction();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void PostEditImport() override;
	
	virtual bool CanRefreshContextPinsDuringLoad() const { return true; }
	
	FText GetNodeToolTip() const override { return FText::GetEmpty(); };

	void PostLoad() override;

	void PreSave(FObjectPreSaveContext SaveContext) override;
	
	void PreloadContent() override;
#endif // WITH_EDITOR
};
