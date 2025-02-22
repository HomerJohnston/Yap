// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Yap/Enums/YapTimeMode.h"
#include "Yap/YapBroker.h"
#include "YapLog.h"

#include "YapProjectSettings.generated.h"

class UYapBroker;
enum class EYapDialogueProgressionFlags : uint8;
enum class EYapMaturitySetting : uint8;
enum class EYapMissingAudioErrorLevel : uint8;

#define LOCTEXT_NAMESPACE "Yap"

enum class EYap_TagFilter : uint8
{
	Conditions,
	Prompts,
};

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class YAP_API UYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FDetailCustomization_YapProjectSettings;
#endif
	
	// ============================================================================================
	// CONSTRUCTION / GETTER
	// ============================================================================================
public:
	UYapProjectSettings();

protected:
	static UYapProjectSettings& Get()
	{
		return *StaticClass()->GetDefaultObject<UYapProjectSettings>();
	}

	// ============================================================================================
	// SETTINGS
	// ============================================================================================
protected:
	
	// - - - - - CORE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** You must create a Yap Broker class (C++ or blueprint) and set it here for Yap to work. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapBroker> BrokerClass = nullptr;
	
	// Do not expose this for editing; only hard-coded
	UPROPERTY() 
	TArray<TSoftClassPtr<UObject>> DefaultAssetAudioClasses;
	
	// - - - - - AUDIO - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Controls how missing audio fields are handled. */ // TODO make error not package
	UPROPERTY(Config, EditAnywhere, Category = "Audio", DisplayName = "Missing Audio Handling", meta = (EditCondition = "DefaultTimeModeSetting == EYapTimeMode::AudioTime", EditConditionHides))
	EYapMissingAudioErrorLevel MissingAudioErrorLevel;
	
	/** What type of classes are allowable to use for dialogue assets (sounds). If unset, defaults to Unreal's USoundBase. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> AudioAssetClasses;

#if WITH_EDITORONLY_DATA
	/** Where to look for audio assets when auto-assigning audio to dialogue. Audio must be placed into a folder path matching the flow asset folder path, and into a subfolder matching the flow asset name.
	 *
	 * Example:
	 * Content\Flows*\E1L1\TalkWithWally.uasset
	 * Content\Audio*\E1L1\TalkWithWally\SpeechAudio123.uasset
	 *
	 * In the above example, the starred Flows and Audio folders would be set as the two root folders. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FDirectoryPath AudioAssetsRootFolder;
	
	/** Where to look for flow assets when auto-assigning audio to dialogue. Audio must be placed into a folder path matching the flow asset folder path, and into a subfolder matching the flow asset name.
	 *
	 * Example:
	 * Content\Flows*\E1L1\TalkWithWally.uasset
	 * Content\Audio*\E1L1\TalkWithWally\SpeechAudio123.uasset
	 *
	 * In the above example, the starred Flows and Audio folders would be set as the two root folders. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FDirectoryPath FlowAssetsRootFolder;
#endif
	
	// - - - - - MOOD TAGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** Parent tag to use for mood tags. All sub-tags of this parent will be used as mood tags! */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag MoodTagsParent;

	/** Optional default mood tag to use, for dialogue fragments which do not have a mood tag set. */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag DefaultMoodTag;

	/** Where to look for portrait key icons. If unspecified, will use the default "Plugins/FlowYap/Resources/MoodTags" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FDirectoryPath MoodTagIconPath;
	
	// - - - - - DIALOGUE TAGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  

	/** Filters dialogue and fragment tags. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Tags")
	FGameplayTag DialogueTagsParent;

	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
	
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	EYapTimeMode DefaultTimeModeSetting;

	/** If set, the Open Convo. node will pause flow graph execution. Yap must be notified to continue. Use this if you want to wait for a conversation opening animation. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 2.0, Delta = 0.01))
	bool bConversationsRequireAdvanceTriggerToStart = false;
	
	/** If set, dialogue will be non-skippable by default and must play for its entire duration. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bForcedDialogueDuration = false;

	/** If set, dialogue will not auto-advance when its duration finishes and will require advancement by using the Dialogue Handle. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bManualAdvanceOnly = false;

	/** If set, Manual Advance Only will be ignored for the last fragment of any talk nodes which are connected to a prompt node. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", DisplayName = " >>> Auto Advance To Prompt Nodes", meta = (EditCondition = "bManualAdvanceOnly", EditConditionHides))
	bool bAutoAdvanceToPromptNodes = false;

	/**
	 * If set, the *last* fragment of an auto-advancing talk node will check if the fragment is playing into a player prompt node. If it is, it will immediately advance into the player prompt node after starting speech.
	 * WARNING: This setting cause all activation conditions of the last fragment of any talk nodes which flow into player prompt nodes to be ignored! This ensures consistent game behavior.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bAdvanceImmediatelyIntoPromptNodes = false;
	
	/** By default, a player prompt node will auto-select the prompt when only one is displayed. This setting prevents that. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bPreventAutoSelectLastPrompt = false;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	TOptional<float> DefaultFragmentPaddingTime = 0.25f;

	/** Controls how fast dialogue plays. Only useful for text-based speaking time. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

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

	// - - - - - EDITOR - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Normally, when assigning dialogue text, Yap will parse the text and attempt to cache a word count to use for determine text time length. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingWordCount = false;

	/** Normally, when assigning an audio length, Yap will read the audio asset and set the speaking time based on it. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingAudioLength = false;
	
	/** If enabled, will show title text on normal talk nodes as well as player prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bShowTitleTextOnTalkNodes = false;
	
	// - - - - - GRAPH APPEARANCE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if WITH_EDITORONLY_DATA
	/** Adjusts the width of all dialogue nodes in graph grid units (16 px). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = -6, ClampMax = +100, UIMin = -6, UIMax = 20, Delta = 1))
	int32 DialogueWidthAdjustment = 0;

	/** Turn off to hide the On Start / On End pin-buttons, useful if you want a simpler graph without these features. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance")
	bool bHidePinEnableButtons = false;
	
	/** Controls how large the portrait widgets are in the graph. Sizes smaller than 64 will result in some odd slate snapping. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 64, ClampMax = 128, UIMin = 32, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	/** Controls the length of the time progress line on the dialogue widget (left side, for time of the running dialogue). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float DialogueTimeSliderMax = 5.0f;
	
	/** Controls the length of the time progress line on the dialogue widget (right side, for delay to next action). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float PaddingTimeSliderMax = 2.0f;

	/** If set, dialogue in the nodes will cut off to the right. This may help if you intend to use lots of multi-line dialogue text. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance")
	bool bPreventDialogueTextWrapping = true;
	
	/** Set the default font for dialogue. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance")
	FSlateFontInfo GraphDialogueFont;
#endif

	// - - - - - ERROR HANDLING - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** If set, you will not be warned when Yap is using default broker functions. Turn this on if you do not need to customize your broker. */
	UPROPERTY(Config, EditAnywhere, Category = "Error Handling")
	bool bSuppressBrokerWarnings = false;

	/** Default texture to use for missing character portraits. */
	UPROPERTY(Config, EditAnywhere, Category = "Error Handling")
	TSoftObjectPtr<UTexture2D> MissingPortraitTexture;
	
	// ============================================================================================
	// STATE
	// ============================================================================================
protected:

#if WITH_EDITORONLY_DATA
	/**  */
	TMap<EYap_TagFilter, FGameplayTag*> TagContainers;
	
	/** A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it */
	TMultiMap<FName, TMap<UClass*, EYap_TagFilter>> TagFilterSubscriptions;
#endif
	
	// ------------------------------------------
	// UObject overrides
public:

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	// ------------------------------------------
	// UDeveloperSettings overrides
public:

#if WITH_EDITOR
	static FName CategoryName;
	
	FName GetCategoryName() const override { return CategoryName; }

	FText GetSectionText() const override { return LOCTEXT("Settings", "Settings"); }
	
	FText GetSectionDescription() const override { return LOCTEXT("YapProjectSettingsDescription", "Project-specific settings for Yap"); }
#endif

	// ------------------------------------------
	// Custom API overrides
public:

#if WITH_EDITOR
	static FString GetMoodTagIconPath(FGameplayTag Key, FString FileExtension);

	static const FGameplayTag& GetMoodTagsParent() { return Get().MoodTagsParent; }
	
	static const FGameplayTag& GetDialogueTagsParent() { return Get().DialogueTagsParent; };
	
	static FGameplayTagContainer GetMoodTags();
#endif

	static bool GetSuppressBrokerWarnings() { return Get().bSuppressBrokerWarnings; }

	static FGameplayTag GetDefaultMoodTag() { return Get().DefaultMoodTag; }
	
	static EYapTimeMode GetDefaultTimeModeSetting() { return Get().DefaultTimeModeSetting; }

	static bool GetDefaultSkippableSetting() { return !Get().bForcedDialogueDuration; }
	
	static bool GetDefaultAutoAdvanceSetting() { return !Get().bManualAdvanceOnly; }

	static bool GetAutoSelectLastPromptSetting() { return !Get().bPreventAutoSelectLastPrompt; }
	
	static const TSoftClassPtr<UYapBroker>& GetBrokerClass() { return Get().BrokerClass; }
	
	static const TArray<TSoftClassPtr<UObject>>& GetAudioAssetClasses();

#if WITH_EDITOR
	static const UYapBroker* GetEditorBrokerDefault();

	static const FString GetAudioAssetRootFolder();
#endif

	static bool HasCustomAudioAssetClasses() { return Get().AudioAssetClasses.Num() > 0; };

	static bool GetShowTitleTextOnTalkNodes() { return Get().bShowTitleTextOnTalkNodes; }

	static int32 GetTextWordsPerMinute() { return Get().TextWordsPerMinute; }

	static float GetMinimumAutoTextTimeLength() { return Get().MinimumAutoTextTimeLength; };
	
	static float GetMinimumAutoAudioTimeLength() { return Get().MinimumAutoAudioTimeLength; }
	
	static float GetMinimumFragmentTime() { return Get().MinimumSpeakingTime; }

	static float GetMinimumTimeRemainingToAllowSkip() { return Get().MinimumTimeRemainingToAllowSkip; }

	static float GetMinimumTimeElapsedToAllowSkip() { return Get().MinimumTimeElapsedToAllowSkip; }

	static bool GetAutoAdvanceToPromptNodes() { return Get().bAutoAdvanceToPromptNodes; }

	//
	static bool GetConversationsRequireAdvanceTriggerToStart() { return Get().bConversationsRequireAdvanceTriggerToStart; }
	
	static bool CacheFragmentWordCountAutomatically() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLengthAutomatically() { return !Get().bPreventCachingAudioLength; }
	
	static TOptional<float> GetDefaultFragmentPaddingTime() { return Get().DefaultFragmentPaddingTime; }
	
	static EYapMissingAudioErrorLevel GetMissingAudioBehavior() { return Get().MissingAudioErrorLevel; }

	static const TSoftObjectPtr<UTexture2D> GetMissingPortraitTextureAsset() { return Get().MissingPortraitTexture; };

#if WITH_EDITOR
public:
	static const FString& GetMoodTagIconPath();
	
	static int32 GetDialogueWidthAdjustment() { return Get().DialogueWidthAdjustment; };

	static int32 GetPortraitSize() { return Get().PortraitSize; }

	static float GetDialogueTimeSliderMax() { return Get().DialogueTimeSliderMax; }

	static float GetFragmentPaddingSliderMax() { return Get().PaddingTimeSliderMax; }

	static bool GetWrapDialogueText() { return !Get().bPreventDialogueTextWrapping; }
	
	static bool ShowPinEnableButtons()  { return !Get().bHidePinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

	static FSlateFontInfo& GetGraphDialogueFont() { return Get().GraphDialogueFont; };

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
#endif
};

#undef LOCTEXT_NAMESPACE
