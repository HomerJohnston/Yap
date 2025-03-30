// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "YapTypeGroupSettings.h"
#include "Yap/Enums/YapTimeMode.h"
#include "Yap/YapBroker.h"

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

public:
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
	TArray<TSoftClassPtr<UObject>> DefaultAssetAudioClasses;
	
	/** What type of classes are allowable to use for dialogue assets (sounds). If unset, defaults to Unreal's USoundBase. */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> AudioAssetClasses;
	
	// - - - - - MOOD TAGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Parent tag to use for mood tags. All sub-tags of this parent will be used as mood tags! If unset, will not use mood tags for this group. */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag MoodTagsParent;

	
	/** Optional default mood tag to use, for fragments which do not have a mood tag set. */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag DefaultMoodTag;

	
	/** Where to look for mood icons. If unspecified, will use the default "Plugins/FlowYap/Resources/MoodTags" folder. */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FDirectoryPath MoodTagEditorIconsPath;
	
	// - - - - - DIALOGUE TYPE GROUPS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** Your most commonly used dialogue type should be setup here. Settings from here will be used for any dialogue nodes which do not have a group assigned. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	FYapTypeGroupSettings DefaultGroup;
	
	/**
	 * Type groups - if you want to have separate type handling for things like normal in-level chatting, tutorial text, incoming radio messages, etc... add "Groups" for them here.
	 * Your "Listeners" can register to receive events from one or more groups.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (ForceInlineRow))
	TMap<FGameplayTag, FYapTypeGroupSettings> NamedGroups;
	
	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
	
	/** Controls how fast dialogue plays. Only useful for text-based speaking time. Can be modified further by your Yap Broker class for user settings. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

	// - - - - - EDITOR - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Normally, when assigning dialogue text, Yap will parse the text and attempt to cache a word count to use for determine text time length. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingWordCount = false;

	/** Normally, when assigning an audio length, Yap will read the audio asset and set the speaking time based on it. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingAudioLength = false;
	
	// - - - - - GRAPH APPEARANCE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if WITH_EDITORONLY_DATA
	// TODO: replace my start/end pins with N timed pins instead? To kick off stuff at any time through a dialogue?
	/** Turn off to hide the On Start / On End pin-buttons, useful if you want a simpler graph without these features. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings")
	bool bHidePinEnableButtons = false;
	
	/** Controls how large the portrait widgets are in the graph. Sizes smaller than 64 will result in some odd slate snapping. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings", meta = (ClampMin = 64, ClampMax = 128, UIMin = 32, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	/** Controls the length of the time progress line on the dialogue widget (left side, for time of the running dialogue). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float DialogueTimeSliderMax = 5.0f;
	
	/** Controls the length of the time progress line on the dialogue widget (right side, for delay to next action). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Settings", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float PaddingTimeSliderMax = 2.0f;
#endif

	// - - - - - ERROR HANDLING - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** If set, you will not be warned when Yap is using default broker functions. Turn this on if you do not need to customize your broker. */
	UPROPERTY(Config, EditAnywhere, Category = "Error Handling")
	bool bSuppressBrokerWarnings = false;

	// - - - - - OTHER - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** Default texture to use for missing character portraits. */
	UPROPERTY(Config, EditAnywhere, Category = "Other")
	TSoftObjectPtr<UTexture2D> DefaultPortraitTexture;

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

	static FGameplayTagContainer GetMoodTags();
#endif

	static bool GetSuppressBrokerWarnings() { return Get().bSuppressBrokerWarnings; }

	static const TSoftClassPtr<UYapBroker>& GetBrokerClass() { return Get().BrokerClass; }
	
	static const TArray<TSoftClassPtr<UObject>>& GetAudioAssetClasses();

	
	static const FGameplayTag& GetMoodTagsParent() { return Get().MoodTagsParent; };

	static const FGameplayTag& GetDefaultMoodTag() { return Get().DefaultMoodTag; };

#if WITH_EDITOR
	const FDirectoryPath& GetMoodTagEditorIconsPath() const { return MoodTagEditorIconsPath; };
#endif
	
#if WITH_EDITOR
	static const UYapBroker* GetEditorBrokerDefault();

	static const FString GetAudioAssetRootFolder(FGameplayTag TypeGroup);
#endif

	static bool HasCustomAudioAssetClasses() { return Get().AudioAssetClasses.Num() > 0; };

	static int32 GetTextWordsPerMinute() { return Get().TextWordsPerMinute; }

	static bool CacheFragmentWordCountAutomatically() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLengthAutomatically() { return !Get().bPreventCachingAudioLength; }
	
	static const TSoftObjectPtr<UTexture2D> GetDefaultPortraitTextureAsset() { return Get().DefaultPortraitTexture; };

#if WITH_EDITOR
public:
	static FString GetMoodTagIconPath();

	static FLinearColor GetGroupColor(FGameplayTag TypeGroup);
	
	static int32 GetPortraitSize() { return Get().PortraitSize; }

	static float GetDialogueTimeSliderMax() { return Get().DialogueTimeSliderMax; }

	static float GetFragmentPaddingSliderMax() { return Get().PaddingTimeSliderMax; }
	
	static bool ShowPinEnableButtons()  { return !Get().bHidePinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
#endif

public:
	static const FYapTypeGroupSettings& GetTypeGroup(FGameplayTag TypeGroup);

	static const FYapTypeGroupSettings* GetTypeGroupPtr(FGameplayTag TypeGroup);
};

#undef LOCTEXT_NAMESPACE


