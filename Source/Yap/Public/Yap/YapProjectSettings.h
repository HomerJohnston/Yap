// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "YapCharacterDefinition.h"
#include "YapNodeConfig.h"
#include "Yap/YapBroker.h"
#include "Yap/GameplayTagFilterHelper.h"

#include "YapProjectSettings.generated.h"

class UYapBroker;
enum class EYapDialogueProgressionFlags : uint8;
enum class EYapMaturitySetting : uint8;
enum class EYapMissingAudioErrorLevel : uint8;

#define LOCTEXT_NAMESPACE "Yap"

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class YAP_API UYapProjectSettings : public UDeveloperSettings, public FGameplayTagFilterHelper<UYapProjectSettings>
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
	
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftObjectPtr<UYapNodeConfig> DefaultNodeConfig;
	
	// - - - - - EDITOR - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Normally, when assigning dialogue text, Yap will parse the text and attempt to cache a word count to use for determine text time length. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingWordCount = false;

	/** Normally, when assigning an audio length, Yap will read the audio asset and set the speaking time based on it. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingAudioLength = false;

	/** This is used when setting AutoAdvanceToPromptNodes is enabled; the graph will attempt to search through these nodes for Prompt nodes. */
	//UPROPERTY(Config, EditAnywhere, Category = "Editor")
	//TArray<TSubclassOf<UFlowNode>> PassThroughNodeTypes;

	/** This is used when setting AutoAdvanceToPromptNodes is enabled; the graph will only recursively visit this many child nodes looking for Prompt nodes. */
	//UPROPERTY(Config, EditAnywhere, Category = "Editor")
	//int32 MaxPassthroughSearchDepth = 2;

	// - - - - - ERROR HANDLING - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** If set, you will not be warned when Yap is using default broker functions. Turn this on if you do not need to customize your broker. */
	UPROPERTY(Config, EditAnywhere, Category = "Error Handling")
	bool bSuppressBrokerWarnings = false;

	/** Default texture to use for missing character portraits. */
	UPROPERTY(Config, EditAnywhere, Category = "Error Handling")
	TSoftObjectPtr<UTexture2D> DefaultPortraitTexture;

	// - - - - - OTHER - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** Optional. Setting this will helpfully filter character tag selectors.
	 *  NOTE:You will need to close and reopen the project settings window for changes to this to take effect.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Characters", DisplayName = "Root Tag")
	FGameplayTag CharacterTagRoot;
	
	/** Map gameplay tags to your game's characters here. Any blueprints or assets which implement the Yap Character interface can be used. */
	UPROPERTY(Config, EditAnywhere, Category = "Characters", DisplayName = "Characters")
	TArray<FYapCharacterDefinition> CharacterArray;

	// Not exposed for editing, this is updated automatically whenever the character array is edited. Game code actually uses this as the character source. 
	TMap<FGameplayTag, TSoftObjectPtr<UObject>> CharacterMap;

	// TODO: SAssetView doesn't support blueprint classes, so we can't use this yet.
	/** By default, Yap will discover all native C++ classes that inherit the Yap Character interface. You can add blueprint classes which inherit it here. This avoids forcefully loading all assets to discover them. */
	//UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract, DisplayName = "Character Classes"))
	//TArray<TSoftClassPtr<UObject>> AdditionalCharacterClasses;

	// ============================================================================================
	// STATE
	// ============================================================================================
protected:

#if WITH_EDITORONLY_DATA
	/**  */
	//TMap<EYap_TagFilter, FGameplayTag*> TagContainers;
	
	/** A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it */
	//TMultiMap<FName, TMap<UClass*, EYap_TagFilter>> TagFilterSubscriptions;
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

	static const TSubclassOf<UYapBroker> GetBrokerClass();

	static const TArray<TSoftClassPtr<UObject>>& GetAudioAssetClasses();

	static const TSoftObjectPtr<UYapNodeConfig>& GetDefaultNodeConfig() { return Get().DefaultNodeConfig; }
	
	static const TArray<const UClass*> GetAllowableCharacterClasses();

	static const TArray<FYapCharacterDefinition>& GetCharacterDefinitions() { return Get().CharacterArray; }
	
	static void AddAdditionalCharacterClass(TSoftClassPtr<UObject> Class);
	
	static bool HasCustomAudioAssetClasses() { return Get().AudioAssetClasses.Num() > 0; };

	static bool CacheFragmentWordCountAutomatically() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLengthAutomatically() { return !Get().bPreventCachingAudioLength; }
	
	static const TSoftObjectPtr<UTexture2D> GetDefaultPortraitTextureAsset() { return Get().DefaultPortraitTexture; };

	static const TSoftObjectPtr<UObject>* FindCharacter(FGameplayTag Character) { return Get().CharacterMap.Find(Character); }
	
	static const TMap<FGameplayTag, TSoftObjectPtr<UObject>>& GetCharacters() { return Get().CharacterMap; }

	static const FGameplayTag& GetCharacterRootTag() { return Get().CharacterTagRoot; }

#if WITH_EDITOR
	// TODO this is kind of ugly
	static TMap<TSoftObjectPtr<UObject>, FGameplayTag> ReversedCharacterMap;
	static void UpdateReversedCharacterMap();
#endif
	
#if WITH_EDITOR
public:
	
protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;

	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	void PostLoad() override;

	void PostInitProperties() override;
	
	void ProcessCharacterArray(bool bUpdateMap);
	
	void RebuildCharacterMap();
#endif
};

#undef LOCTEXT_NAMESPACE


