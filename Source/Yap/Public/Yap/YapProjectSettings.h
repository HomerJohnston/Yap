// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "YapNodeConfig.h"
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
	
	// Do not expose this for editing; only hard-coded
	TArray<TSoftClassPtr<UObject>> DefaultAssetAudioClasses;

	// Do not expose this for editing; only hard-coded
	TArray<TSoftClassPtr<UObject>> DefaultCharacterClasses;

	// - - - - - CORE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Create a Yap Broker class, inheriting from UYapBroker, and set it here to override some default Yap behavior such as reading your game's maturity setting. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapBroker> BrokerClass = nullptr;
	
	/** What type of classes are allowable to use for dialogue assets (sounds). If unset, defaults to Unreal's USoundBase. */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> AudioAssetClasses;
	
	/** By default, Yap will discover all native C++ classes that inherit the Yap Character interface. You can add blueprint classes which inherit it here. This avoids forcefully loading all assets to discover them. */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> AdditionalCharacterClasses;

	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapNodeConfig> DefaultNodeConfig;
	
	// - - - - - EDITOR - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Normally, when assigning dialogue text, Yap will parse the text and attempt to cache a word count to use for determine text time length. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingWordCount = false;

	/** Normally, when assigning an audio length, Yap will read the audio asset and set the speaking time based on it. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingAudioLength = false;

	/** This is used when setting AutoAdvanceToPromptNodes is enabled; the graph will attempt to search through these nodes for Prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	TArray<TSubclassOf<UFlowNode>> PassThroughNodeTypes;

	/** This is used when setting AutoAdvanceToPromptNodes is enabled; the graph will only recursively visit this many child nodes looking for Prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	int32 MaxPassthroughSearchDepth = 2;
	
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

	static bool GetSuppressBrokerWarnings() { return Get().bSuppressBrokerWarnings; }

	static const TSoftClassPtr<UYapBroker>& GetBrokerClass() { return Get().BrokerClass; }
	
	static const TArray<TSoftClassPtr<UObject>>& GetAudioAssetClasses();

	static const TSoftClassPtr<UYapNodeConfig>& GetDefaultNodeConfig() { return Get().DefaultNodeConfig; }
	
#if WITH_EDITOR
	static const UYapBroker* GetEditorBrokerDefault();
#endif

	static const TArray<TSoftClassPtr<UObject>>& GetAdditionalCharacterClasses() { return Get().AdditionalCharacterClasses; }

	static void AddAdditionalCharacterClass(TSoftClassPtr<UObject> Class);
	
	static bool HasCustomAudioAssetClasses() { return Get().AudioAssetClasses.Num() > 0; };

	static bool CacheFragmentWordCountAutomatically() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLengthAutomatically() { return !Get().bPreventCachingAudioLength; }
	
	static const TSoftObjectPtr<UTexture2D> GetDefaultPortraitTextureAsset() { return Get().DefaultPortraitTexture; };

#if WITH_EDITOR
public:
	
	static int32 GetPortraitSize() { return Get().PortraitSize; }

	static float GetDialogueTimeSliderMax() { return Get().DialogueTimeSliderMax; }

	static float GetFragmentPaddingSliderMax() { return Get().PaddingTimeSliderMax; }
	
	static bool ShowPinEnableButtons()  { return !Get().bHidePinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
#endif
};

#undef LOCTEXT_NAMESPACE


