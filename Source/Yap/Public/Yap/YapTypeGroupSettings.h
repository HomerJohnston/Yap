// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/Enums/YapMissingAudioErrorLevel.h"
#include "Yap/Enums/YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "YapTypeGroupSettings.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

USTRUCT()
struct YAP_API FYapTypeGroupSettings
{
    GENERATED_BODY()

	// CONSTRUCTION ===============================================================================

public:
	FYapTypeGroupSettings();

	FYapTypeGroupSettings(bool bDefaultIn);

	friend class UYapProjectSettings;

#if WITH_EDITOR
	friend class FPropertyCustomization_YapGroupSettings;
#endif
	
	// SETTINGS ===================================================================================

	// - - - - - PRIVATE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
private:
	/** If true, this struct is the default settings. This is only set internally by UYapProjectSettings. */
	bool bDefault = false;
	
	UPROPERTY(Config, EditAnywhere, meta = (DoNotDraw))
	FLinearColor GroupColor = FLinearColor::White;

	// - - - - - AUDIO - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
private:
#if WITH_EDITORONLY_DATA
	
	/** Where to look for audio assets when auto-assigning audio to dialogue. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FDirectoryPath AudioAssetsRootFolder;
	
	/** Where to look for flow assets when auto-assigning audio to dialogue. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FDirectoryPath FlowAssetsRootFolder;

	// TODO make error not package
	/** Controls how missing audio fields are handled. */ 
	UPROPERTY(Config, EditAnywhere, Category = "Audio", DisplayName = "Missing Audio Handling")
	EYapMissingAudioErrorLevel MissingAudioErrorLevel = EYapMissingAudioErrorLevel::Warning;
#endif
	
	// - - - - - DIALOGUE TAGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  

	/** Filters dialogue and fragment tags. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Tags")
	FGameplayTag DialogueTagsParent;
	
	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
	
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	EYapTimeMode DefaultTimeModeSetting = EYapTimeMode::AudioTime;

	/** If set, the Open Convo. node will pause flow graph execution. Yap must be notified to continue. Use this if you want to wait for a conversation opening animation. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	bool bConversationsRequireAdvanceTriggerToStart = false;
	
	/** If set, dialogue will be non-skippable by default and must play for its entire duration. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (EditCondition = "!bDoNotUseSpeechTime", EditConditionHides))
	bool bForcedDialogueDuration = false;
	
	/** If set, dialogue will not auto-advance when its duration finishes and will require advancement by using the Dialogue Handle. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (EditCondition = "!bDoNotUseSpeechTime", EditConditionHides))
	bool bManualAdvanceOnly = false;

	/** If set, prompt nodes will always default to auto-advance. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bPromptAutoAdvance = false;
	
	/**
	 * If set, the *last* fragment of an auto-advancing talk node will check if the fragment is playing into a player prompt node. If it is, it will immediately advance into the player prompt node after starting speech.
	 * This lets prompt options pop-up immediately upon starting that speech. This is roughly equivalent to using the last fragment's "Start" pin to move into the Prompt node.
	 * WARNING: This setting cause all activation conditions of the last fragment of any talk nodes which flow into player prompt nodes to be ignored! This ensures consistent game behavior.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", DisplayName = "Auto-advance to Prompt Nodes", meta = (EditCondition = "bDoNotUseSpeechTime || bManualAdvanceOnly", EditConditionHides))
	bool bAutoAdvanceToPromptNodes = false;
	
	/** By default, a player prompt node will auto-select the prompt when only one is displayed. This setting prevents that. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bPreventAutoSelectLastPrompt = false;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	float DefaultFragmentPaddingTime = 0.25f;
	
	/** When speaking time is calculated from text, this sets the minimum speaking time. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	float MinimumAutoTextTimeLength = 1.0;
	
	/** When speaking time is calculated from the length of an audio asset, this sets the minimum speaking time. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	float MinimumAutoAudioTimeLength = 0.5;
	
	/** Total minimum speaking time for any fragment. Should be fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	float MinimumSpeakingTime = 0.25;
	
	/** When dialogue is set to audo-advance, skip requests will be ignored if the total remaining playback time (speech time + padding time) is less than this. This should normally be left at zero.*/
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	float MinimumTimeRemainingToAllowSkip = 0.0;
	
	/** Skip requests will be ignored if the total elapsed time (speech time + padding time) is less than this. This should normally be a small number. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	float MinimumTimeElapsedToAllowSkip = 0.25;

	/** Controls how fast dialogue plays. Only useful for text-based speaking time. Can be modified further by your Yap Broker class for user settings. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

	// - - - - - FLOW GRAPH SETTINGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	FText TalkModeTitle = LOCTEXT("Talk_DefaultNodeTitle", "Talk");

	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	FText PromptModeTitle = LOCTEXT("PlayerPrompt_DefaultNodeTitle", "Prompt");
	
	/** Adjusts the width of all dialogue nodes in graph grid units (16 px). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings", meta = (ClampMin = -6, ClampMax = +100, UIMin = -6, UIMax = 20, Delta = 1))
	int32 DialogueWidthAdjustment = 0;
	
	/** Set the default font for dialogue. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	FSlateFontInfo GraphDialogueFont;
	
	/** If set, dialogue in the nodes will cut off to the right. This may help if you intend to use lots of multi-line dialogue text. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bPreventDialogueTextWrapping = true;

	/** If enabled, will show title text on normal talk nodes. Title text is not commonly used on automatically progressing dialogue, so it is hidden by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bShowTitleTextOnTalkNodes = false;

	/** If enabled, will hide the title text field on prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHideTitleTextOnPromptNodes = false;
	
	/** If this type group doesn't require any speaker info (such as for generic tutorial popups), you can enable this to clean up the flow graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHideSpeakerSelector = false;

	/** If this type group doesn't require any mood info (such as for generic tutorial popups), you can enable this to clean up the flow graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHideMoodSelector = false;
	
	/** If this type group doesn't require any child safe info (such as for generic tutorial popups), you can enable this to clean up the flow graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHideChildSafeButton = false;
	
	/** If this type group doesn't require any audio (like if your game does not have dialogue audio), you can enable this to clean up the flow graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHideAudioID = false;
	
	// ============================================================================================
public:
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MissingAudioErrorLevel))
	bool bMissingAudioErrorLevel_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = DefaultTimeModeSetting))
	bool bDefaultTimeModeSetting_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = DialogueTagsParent))
	bool bDialogueTagsParent_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = AudioAssetsRootFolder))
	bool bAudioAssetsRootFolder_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = FlowAssetsRootFolder))
	bool bFlowAssetsRootFolder_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bConversationsRequireAdvanceTriggerToStart))
	bool bConversationsRequireAdvanceTriggerToStart_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bForcedDialogueDuration))
	bool bForcedDialogueDuration_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bManualAdvanceOnly))
	bool bManualAdvanceOnly_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bPromptAutoAdvance))
	bool bPromptAutoAdvance_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bAutoAdvanceToPromptNodes))
	bool bAutoAdvanceToPromptNodes_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bPreventAutoSelectLastPrompt))
	bool bPreventAutoSelectLastPrompt_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = DefaultFragmentPaddingTime))
	bool bDefaultFragmentPaddingTime_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MinimumAutoTextTimeLength))
	bool bMinimumAutoTextTimeLength_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MinimumAutoAudioTimeLength))
	bool bMinimumAutoAudioTimeLength_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MinimumSpeakingTime))
	bool bMinimumSpeakingTime_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MinimumTimeRemainingToAllowSkip))
	bool bMinimumTimeRemainingToAllowSkip_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = MinimumTimeElapsedToAllowSkip))
	bool bMinimumTimeElapsedToAllowSkip_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = TextWordsPerMinute))
	bool bTextWordsPerMinute_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = TalkModeTitle))
	bool bTalkModeTitle_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = PromptModeTitle))
	bool bPromptModeTitle_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = DialogueWidthAdjustment))
	bool bDialogueWidthAdjustment_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = GraphDialogueFont))
	bool bGraphDialogueFont_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bPreventDialogueTextWrapping))
	bool bPreventDialogueTextWrapping_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bShowTitleTextOnTalkNodes))
	bool bShowTitleTextOnTalkNodes_Override = false;

	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bHideTitleTextOnPromptNodes))
	bool bHideTitleTextOnPromptNodes_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bHideSpeakerSelector))
	bool bHideSpeakerSelector_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bHideMoodSelector))
	bool bHideMoodSelector_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bHideChildSafeButton))
	bool bHideChildSafeButton_Override = false;
	
	UPROPERTY(EditAnywhere, meta = (DefaultOverride = bHideAudioID))
	bool bHideAudioID_Override = false;
	
	// PROPERTY GETTERS
	// ============================================================================================
public:
	bool GetDefault() const { return bDefault; }

	FLinearColor GetGroupColor() const { return GroupColor; }

#define RETURN(BOOL, VAR) const { return BOOL ? VAR : Default().VAR; }  
	
#if WITH_EDITORONLY_DATA
	const FDirectoryPath& GetAudioAssetsRootFolder() RETURN(bAudioAssetsRootFolder_Override, AudioAssetsRootFolder)

	const FDirectoryPath& GetFlowAssetsRootFolder() RETURN(bFlowAssetsRootFolder_Override, FlowAssetsRootFolder)
#endif

	const FGameplayTag& GetDialogueTagsParent() RETURN(bDialogueTagsParent_Override, DialogueTagsParent)
	
	EYapTimeMode GetDefaultTimeModeSetting() RETURN(bDefaultTimeModeSetting_Override, DefaultTimeModeSetting)

	EYapMissingAudioErrorLevel GetMissingAudioErrorLevel() RETURN(bMissingAudioErrorLevel_Override, MissingAudioErrorLevel)
	
	bool GetConversationsRequireAdvanceTriggerToStart() RETURN(bConversationsRequireAdvanceTriggerToStart_Override, bConversationsRequireAdvanceTriggerToStart)

	bool GetForcedDialogueDuration() RETURN(bForcedDialogueDuration_Override, bForcedDialogueDuration)
	
	bool GetManualAdvanceOnly() RETURN(bManualAdvanceOnly_Override, bManualAdvanceOnly)

	bool GetPromptAutoAdvance() RETURN(bPromptAutoAdvance_Override, bPromptAutoAdvance)
	
	bool GetAutoAdvanceToPromptNodes() RETURN(bAutoAdvanceToPromptNodes_Override, bAutoAdvanceToPromptNodes)
	
	bool GetPreventAutoSelectLastPrompt() RETURN(bPreventAutoSelectLastPrompt_Override, bPreventAutoSelectLastPrompt)
	
	float GetDefaultFragmentPaddingTime() RETURN(bDefaultFragmentPaddingTime_Override, DefaultFragmentPaddingTime)
	
	float GetMinimumAutoTextTimeLength() RETURN(bMinimumAutoTextTimeLength_Override, MinimumAutoTextTimeLength)
	
	float GetMinimumAutoAudioTimeLength() RETURN(bMinimumAutoAudioTimeLength_Override, MinimumAutoAudioTimeLength)
	
	float GetMinimumSpeakingTime() RETURN(bMinimumSpeakingTime_Override, MinimumSpeakingTime)
	
	float GetMinimumTimeRemainingToAllowSkip() RETURN(bMinimumTimeRemainingToAllowSkip_Override, MinimumTimeRemainingToAllowSkip)
	
	float GetMinimumTimeElapsedToAllowSkip() RETURN(bMinimumTimeElapsedToAllowSkip_Override, MinimumTimeElapsedToAllowSkip)
	
	FText GetTalkModeTitle() RETURN(bTalkModeTitle_Override, TalkModeTitle)

	FText GetPromptModeTitle() RETURN(bPromptModeTitle_Override, PromptModeTitle)
	
	int32 GetDialogueWidthAdjustment() RETURN(bDialogueWidthAdjustment_Override, DialogueWidthAdjustment)
	
	const FSlateFontInfo& GetGraphDialogueFont() RETURN(bGraphDialogueFont_Override, GraphDialogueFont)
	
	bool GetPreventDialogueTextWrapping() RETURN(bPreventDialogueTextWrapping_Override, bPreventDialogueTextWrapping)

	bool GetShowTitleTextOnTalkNodes() RETURN(bShowTitleTextOnTalkNodes_Override, bShowTitleTextOnTalkNodes)
	
	bool GetHideTitleTextOnPromptNodes() RETURN(bHideTitleTextOnPromptNodes_Override, bHideTitleTextOnPromptNodes)
	
	bool GetHideSpeakerSelector() RETURN(bHideSpeakerSelector_Override, bHideSpeakerSelector)

	bool GetHideMoodSelector() RETURN(bHideMoodSelector_Override, bHideMoodSelector)
	
	bool GetHideChildSafeButton() RETURN(bHideChildSafeButton_Override, bHideChildSafeButton)

	bool GetHideAudioID() RETURN(bHideAudioID_Override, bHideAudioID)
	// OTHER HELPERS
	// ============================================================================================
private:
	static TArray<FString>& GetDefaultMoodTags();

	static const FYapTypeGroupSettings& Default();
};

#undef LOCTEXT_NAMESPACE