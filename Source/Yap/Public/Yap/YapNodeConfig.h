// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/Enums/YapMissingAudioErrorLevel.h"
#include "Yap/Enums/YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "GameplayTagFilterHelper.h"
#include "YapNodeConfig.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ================================================================================================

enum class EYapDialogueNodeType : uint8;

USTRUCT()
struct FYapNodeConfigGroup_General
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	/** Show an optional label for this node type on flow graphs, this will appear on the right side of the node's header area. */
	UPROPERTY(EditAnywhere)
	FText NodeLabel;
	
	UPROPERTY(EditAnywhere, meta = (DoNotDraw))
	TOptional<FLinearColor> GroupColor;
	
	/** Controls which node types are permitted to be selected. */
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/Yap.EYapDialogueNodeType"))
	int32 AllowableNodeTypes;

	// OK so EditCondition doesn't have bitwise operators so let's hard code every number that would include the correct bit I hope I never change anything
	// UPROPERTY(EditAnywhere, meta = (EditCondition = "AllowableNodeTypes & EYapDialogueNodeType > 0", EditConditionHides))

	/** Optional, change the flow graph node title from 'Talk' to '...'. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "AllowableNodeTypes == 1 || AllowableNodeTypes == 3 || AllowableNodeTypes == 5 || AllowableNodeTypes == 7", EditConditionHides))
	FText TalkLabelOverride;
	
	/** Optional, change the flow graph node title from 'Talk & Advance' to '...'. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "AllowableNodeTypes == 2 || AllowableNodeTypes == 3 || AllowableNodeTypes == 6 || AllowableNodeTypes == 7", EditConditionHides))
	FText TalkAndAdvanceLabelOverride;
	
	/** Optional, change the flow graph node title from 'Prompt' to '...'. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "AllowableNodeTypes == 4 || AllowableNodeTypes == 5 || AllowableNodeTypes == 6 || AllowableNodeTypes == 7", EditConditionHides))
	FText PromptLabelOverride;

	/** Placeholder. This does nothing for now. */
	UPROPERTY(Config, EditAnywhere, meta = (EditCondition = "false && (AllowableNodeTypes == 7)", EditConditionHides))
	bool bPromptRequiresTalkAndAdvance = false;

	/** Use this to filter the character tag selector. If all of your game's character tags are under YourGame.Entity.Character.XYZ then set this to YourGame.Entity.Character for easier character selection. */
	UPROPERTY(EditAnywhere)
	FGameplayTag CharacterTagBase;
#endif
	
	/** If this node type doesn't require any speaker info (such as for generic tutorial popups or simple player prompts), you can enable this. Requires graph refresh. */
	UPROPERTY(EditAnywhere)
	bool bDisableSpeaker = false;

	/** If this node type doesn't require any directed at info (such as for generic tutorial popups or simple player prompts), you can enable this. Requires graph refresh. */
	UPROPERTY(EditAnywhere)
	bool bDisableDirectedAt = false;
	
	/** If this node type doesn't require any child safe info (such as for generic tutorial popups or simple player prompts), you can enable this. Requires graph refresh. */
	UPROPERTY(EditAnywhere)
	bool bDisableChildSafe = false;
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_Audio
{
	GENERATED_BODY()

	/** Disable all audio settings. */
	UPROPERTY(EditAnywhere)
	bool bDisableAudio = false;
	
#if WITH_EDITORONLY_DATA
	/** Highest folder containing flow assets when auto-assigning audio to dialogue. Expand for more info...*/
	UPROPERTY(EditAnywhere, DisplayName = "Flows Root Folder",  meta = (EditCondition = "!bDisableAudio", EditConditionHides))
	FDirectoryPath FlowAssetsRootFolder;
#endif

	// TODO make errors fail packaging
	/** Controls how missing audio fields are handled. */ 
	UPROPERTY(EditAnywhere, DisplayName = "Missing Audio Handling", meta = (EditCondition = "!bDisableAudio", EditConditionHides))
	EYapMissingAudioErrorLevel MissingAudioErrorLevel = EYapMissingAudioErrorLevel::Warning;
	
	/** Optionally hide the audio ID tag. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bDisableAudio", EditConditionHides))
	bool bHideAudioID = false;
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_DialoguePlaybackTime
{
	GENERATED_BODY()
	
	/** Time mode to use by default. */
	UPROPERTY(EditAnywhere)
	EYapTimeMode DefaultTimeModeSetting = EYapTimeMode::AudioTime;

	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(EditAnywhere, Category = "Time", meta = (UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	float PaddingTimeDefault = 0.25f;
	
	/** When speaking time is calculated from text, this sets the minimum speaking time. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	float MinimumAutoTextTimeLength = 1.0;
	
	/** When speaking time is calculated from the length of an audio asset, this sets the minimum speaking time. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	float MinimumAutoAudioTimeLength = 0.5;
	
	/** Total minimum speaking time for any fragment. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	float MinimumSpeakingTime = 0.0;
	
	/** When dialogue is set to audo-advance, skip requests will be ignored if the total remaining playback time (speech time + padding time) is less than this. This should normally be left at zero.*/
	//UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	//float MinimumTimeRemainingToAllowSkip = 0.0;
	
	/** Skip requests will be ignored if the total elapsed time (speech time + padding time) is less than this. This should normally be a small number. */
	//UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	//float MinimumTimeElapsedToAllowSkip = 0.25;

	// ---
	
	/** Controls how fast dialogue plays. Only useful for text-based speaking time. Can be modified further by your Yap Broker class for user settings. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	float TextWordsPerMinute = 120;	
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_DialoguePlayback
{
	GENERATED_BODY()

	/** If set, a character will be able to say multiple things at once. By default, if you launch two dialogues on the same character, the second one will cause the first one to cancel. */
	UPROPERTY(EditAnywhere)
	bool bPermitOverlappingSpeech = false;
	
	/** If set, dialogue will be non-skippable by default and must play for its entire duration. */
	UPROPERTY(EditAnywhere)
	bool bForcedDialogueDuration = false;
	
	/** If set, dialogue in a conversation will not auto-advance by default and will require advancement by using the Conversation Handle. */
	UPROPERTY(EditAnywhere, DisplayName = "Manual Advance")
	bool bManualAdvanceInConversations = false;

	/** If set, dialogue outside of a conversation will not auto-advance by default and will require advancement by using the Speech Handle. */
	UPROPERTY(EditAnywhere)
	bool bManualAdvanceFreeSpeech = false;

	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_DialoguePlaybackTime TimeSettings;
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_PromptSettings
{
	GENERATED_BODY()
	
	/** If set, prompt nodes will advance immediately upon selection. */
	UPROPERTY(EditAnywhere, DisplayName = "Prompts Auto-Advance by Default")
	bool bPromptAdvancesImmediately = false;
	
	/** By default, a player prompt node will auto-select the prompt when only one is displayed. This setting prevents that. */
	UPROPERTY(EditAnywhere)
	bool bAutoSelectLastPrompt = false;
	
	/** If set, connecting a normal "Talk" node into a "Prompt" node will show a warning. Use this if you want to help encourage "Talk and Advance" node usage. */
	UPROPERTY(EditAnywhere)
	bool bWarnWhenTalkNodeEntersPrompt = false;
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_FlowGraphSettings
{
	GENERATED_BODY()

	FYapNodeConfigGroup_FlowGraphSettings();

#if WITH_EDITORONLY_DATA
	/** Adjusts the width of all dialogue nodes in graph grid units (16 px). */
	UPROPERTY(EditAnywhere, meta = (ClampMin = -6, ClampMax = +100, UIMin = -6, UIMax = 20, Delta = 1))
	int32 DialogueWidthAdjustment = 0;
		
	/** If set, dialogue in the nodes will cut off to the right. This may help if you intend to use lots of multi-line dialogue text. */
	UPROPERTY(EditAnywhere)
	bool bPreventDialogueTextWrapping = false;
#endif
	
	/** If enabled, will show title text on normal talk nodes. Title text is not commonly used on automatically progressing dialogue, so it is hidden by default. */
	UPROPERTY(EditAnywhere)
	bool bShowTitleTextOnTalkNodes = false;

	/** If enabled, will hide the title text field on prompt nodes. */
	UPROPERTY(EditAnywhere)
	bool bHideTitleTextOnPromptNodes = false;

#if WITH_EDITORONLY_DATA
	/** Set the default font for dialogue. */
	UPROPERTY(EditAnywhere)
	FSlateFontInfo DialogueFont;
	
	// TODO: replace my start/end pins with N timed pins instead? To kick off stuff at any time through a dialogue?
	/** Turn off to hide the On Start / On End pin-buttons, useful if you want a simpler graph without these features. */
	UPROPERTY(Config, EditAnywhere)
	bool bHidePinEnableButtons = false;
	
	/** Controls how large the portrait widgets are in the graph. Sizes smaller than 64 will result in some odd slate snapping. */
	UPROPERTY(Config, EditAnywhere, meta = (ClampMin = 64, ClampMax = 128, UIMin = 32, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	/** Controls the length of the time progress line on the dialogue widget (left side, for time of the running dialogue). */
	UPROPERTY(Config, EditAnywhere, meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float DialogueTimeSliderMax = 5.0f;
	
	/** Controls the length of the time progress line on the dialogue widget (right side, for delay to next action). */
	UPROPERTY(Config, EditAnywhere, meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float PaddingTimeSliderMax = 2.0f;	
#endif
};

// ================================================================================================

USTRUCT()
struct FYapNodeConfigGroup_MoodTags
{
	GENERATED_BODY()

	/** If this type group doesn't require any mood info (such as for generic tutorial popups or simple player prompts), you can enable this to clean up the flow graph. Requires graph refresh. */
	UPROPERTY(EditAnywhere)
	bool bDisableMoodTags = false;
	
	/** Parent tag to use for mood tags. All sub-tags of this parent will be used as mood tags. If unset, will not use mood tags for this group. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bDisableMoodTags", EditConditionHides))
	FGameplayTag MoodTagsRoot;

	/** Optional default mood tag to use, for fragments which do not have a mood tag set. */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bDisableMoodTags", EditConditionHides))
	FGameplayTag DefaultMoodTag;

#if WITH_EDITORONLY_DATA
	/** Where to look for editor-only mood tag icons. Expand for more info... */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bDisableMoodTags", EditConditionHides))
	FDirectoryPath EditorIconsPath;
#endif
	
	FGameplayTagContainer GetAllMoodTags();
};

// ================================================================================================

/**
 * Yap node config assets can be created and used to control how different dialogue node subclasses play.
 */
UCLASS(Blueprintable)
class YAP_API UYapNodeConfig : public UObject, public FGameplayTagFilterHelper<UYapNodeConfig>
{
    GENERATED_BODY()

	// CONSTRUCTION ===============================================================================

public:
	UYapNodeConfig();

	friend class UYapProjectSettings;

#if WITH_EDITOR
	friend class FPropertyCustomization_YapGroupSettings;
	friend class SFlowGraphNode_YapDialogueWidget;
#endif

private:
	// --------------------------------------------------------------------------------------------
	// SETTINGS 
public:
	
	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_General General;
	
	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_Audio Audio;

	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_DialoguePlayback DialoguePlayback;

	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_PromptSettings Prompts;

	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_FlowGraphSettings Graph;

	UPROPERTY(EditAnywhere)
	FYapNodeConfigGroup_MoodTags MoodTags;
	
	// --------------------------------------------------------------------------------------------
	// OVERRIDE TOGGLES
private:

	// --------------------------------------------------------------------------------------------
	// PROPERTY GETTERS
public:

#if WITH_EDITORONLY_DATA
	TOptional<FLinearColor> GetGroupColor() const { return General.GroupColor; }
#endif
	
#ifdef RETURN
	
#endif
#if WITH_EDITORONLY_DATA
	const FDirectoryPath& GetFlowAssetsRootFolder() const { return Audio.FlowAssetsRootFolder; }
#endif

	EYapTimeMode GetDefaultTimeModeSetting() const { return DialoguePlayback.TimeSettings.DefaultTimeModeSetting; }

	EYapMissingAudioErrorLevel GetMissingAudioErrorLevel() const { return Audio.MissingAudioErrorLevel; }
	
	bool GetForcedDialogueDuration() const { return DialoguePlayback.bForcedDialogueDuration; }
	
	bool GetManualAdvanceOnly() const
	{
		EYapTimeMode TimeMode = GetDefaultTimeModeSetting();
		
		if (TimeMode == EYapTimeMode::None)
		{
			return true;
		}
		
		return DialoguePlayback.bManualAdvanceInConversations;
	}

	bool GetPromptAdvancesImmediately() const { return Prompts.bPromptAdvancesImmediately; }
	
	bool GetAutoSelectLastPrompt() const { return Prompts.bAutoSelectLastPrompt; }

	bool GetIgnoreManualAdvanceOutsideConversations() const { return DialoguePlayback.bManualAdvanceFreeSpeech; }
	
	float GetDefaultFragmentPaddingTime() const { return DialoguePlayback.TimeSettings.PaddingTimeDefault; }
	
	float GetMinimumAutoTextTimeLength() const { return DialoguePlayback.TimeSettings.MinimumAutoTextTimeLength; }
	
	float GetMinimumAutoAudioTimeLength() const { return DialoguePlayback.TimeSettings.MinimumAutoAudioTimeLength; }
	
	float GetMinimumSpeakingTime() const { return DialoguePlayback.TimeSettings.MinimumSpeakingTime; }
	
	//float GetMinimumTimeRemainingToAllowSkip() const { return MinimumTimeRemainingToAllowSkip; }
	
	//float GetMinimumTimeElapsedToAllowSkip() const { return MinimumTimeElapsedToAllowSkip; }
	
	FText GetTalkModeTitle() const;

    FText GetTalkAndAdvanceModeTitle() const;

    FText GetPromptModeTitle() const;

#if WITH_EDITORONLY_DATA
    int32 GetDialogueWidthAdjustment() const { return Graph.DialogueWidthAdjustment; }
	
	const FSlateFontInfo& GetGraphDialogueFont() const { return Graph.DialogueFont; }
	
	bool ShowPinEnableButtons()  { return !Graph.bHidePinEnableButtons; }
	
	int32 GetPortraitSize() const { return Graph.PortraitSize; }

	float GetDialogueTimeSliderMax() const { return Graph.DialogueTimeSliderMax; }

	float GetFragmentPaddingSliderMax() const { return Graph.PaddingTimeSliderMax; }
		
	bool GetPreventDialogueTextWrapping() const { return Graph.bPreventDialogueTextWrapping; }

	bool GetShowTitleTextOnTalkNodes() const { return Graph.bShowTitleTextOnTalkNodes; }
	
	bool GetHideTitleTextOnPromptNodes() const { return Graph.bHideTitleTextOnPromptNodes; }
#endif
	bool GetUsesTitleText(EYapDialogueNodeType NodeType) const;
	
	bool GetUsesSpeaker() const { return !General.bDisableSpeaker; }

	bool GetUsesDirectedAt() const { return !General.bDisableDirectedAt; }
	
	bool GetUsesChildSafe() const { return !General.bDisableChildSafe; }

	bool GetUsesAudioAsset() const { return !Audio.bDisableAudio; }

	bool GetHideAudioID() const { return !Audio.bDisableAudio && Audio.bHideAudioID; }

	int32 GetTextWordsPerMinute() { return DialoguePlayback.TimeSettings.TextWordsPerMinute; }

	bool GetUsesMoodTags() const { return !MoodTags.bDisableMoodTags; }

	const FGameplayTag& GetMoodTagsParent() const { return MoodTags.MoodTagsRoot; }

	const FGameplayTag& GetDefaultMoodTag() const { return MoodTags.DefaultMoodTag; }

	FGameplayTagContainer GetMoodTags() const;

#if WITH_EDITOR
protected:
	static TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>> MoodTagIconBrushes;
	
	static TUniquePtr<FSlateImageBrush> NullMoodTagIconBrush;
	
public:
	const FDirectoryPath& GetMoodTagEditorIconsPath() const { return MoodTags.EditorIconsPath; };
	
	void PostLoad() override;

    void RebuildMoodTagIcons();

	void BuildIcon(const FGameplayTag& MoodTag);
	
    FString GetMoodTagIconPath(FGameplayTag Key, FString FileExtension) const;

    FSlateImageBrush* GetMoodTagIcon(FGameplayTag MoodTag) const;

	const FSlateBrush* GetMoodTagBrush(FGameplayTag Name) const;

	bool CanEditChange(const FProperty* InProperty) const override;

	bool CanEditChange(const FEditPropertyChain& PropertyChain) const override;
#endif
	
	const FGameplayTag& GetMoodTagsRoot() const;
	
	
	// OTHER HELPERS
	// ============================================================================================
public:
	static TArray<FString>& GetDefaultMoodTags();
};

#undef LOCTEXT_NAMESPACE
