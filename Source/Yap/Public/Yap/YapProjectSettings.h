// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "YapCharacterStaticDefinition.h"
#include "YapNodeConfig.h"
#include "Editor/YapAudioIDFormat.h"
#include "Yap/YapBroker.h"
#include "Yap/GameplayTagFilterHelper.h"

#include "YapProjectSettings.generated.h"

class UYapBroker;
class UTexture2D;
enum class EYapDialogueProgressionFlags : uint8;
enum class EYapMaturitySetting : uint8;
enum class EYapAudioPriority : uint8;

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

	/** Can be overridden by individual node configs. Uses a simple parser:
	 * 
	 * *   random capitalized letter (**** becomes JAHX)
	 * ?   random alphanumeric character (???? becomes Z4P1)
	 * #   incremental fragment number (#### becomes 0001, 0002, 0003...)
	 *
	 * The default pattern will generate audio IDs like: JSU-010, JSU-020, JSU-030, JSU-040 */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	FYapAudioIDFormat DefaultAudioIDFormat;
	
	/** Characters will not be used during random audio ID generation. Default settings preclude letter I and letter O to avoid confusion with numbers. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Illegal Audio ID Characters")
	FString IllegalAudioIDCharacters = TEXT("IO");

	
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
	TArray<FYapCharacterStaticDefinition> CharacterArray;

	// Not exposed for editing, this is updated automatically whenever the character array is edited. Game code actually uses this as the character source. 
	TMap<FGameplayTag, FYapCharacterStaticDefinition> CharacterMap;

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

	static const TArray<FYapCharacterStaticDefinition>& GetCharacterDefinitions() { return Get().CharacterArray; }
	
	static void AddAdditionalCharacterClass(TSoftClassPtr<UObject> Class);
	
	static bool HasCustomAudioAssetClasses() { return Get().AudioAssetClasses.Num() > 0; };

	static bool CacheFragmentWordCountAutomatically() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLengthAutomatically() { return !Get().bPreventCachingAudioLength; }

	static const FYapAudioIDFormat& GetDefaultAudioIDFormat() { return Get().DefaultAudioIDFormat; }

	static const FString& GetIllegalAudioIDCharacters() { return Get().IllegalAudioIDCharacters; }
	
	static const TSoftObjectPtr<UTexture2D> GetDefaultPortraitTextureAsset() { return Get().DefaultPortraitTexture; };

#if WITH_EDITOR
	static const UObject* FindCharacter(FGameplayTag CharacterTag, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext);
#endif
	
	static const TMap<FGameplayTag, FYapCharacterStaticDefinition>& GetCharacters() { return Get().CharacterMap; }

	static const FGameplayTag& GetCharacterRootTag() { return Get().CharacterTagRoot; }

	static const FGameplayTag& FindCharacterTag(const TSoftObjectPtr<UObject>& Character);
	
#if WITH_EDITOR
	// TODO this is kind of ugly. Can I get rid of it.
	static TMap<FYapCharacterStaticDefinition, FGameplayTag> ReversedCharacterMap;
	static void UpdateReversedCharacterMap();
#endif
	
#if WITH_EDITOR
public:
	
protected:
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	void PostInitProperties() override;
	
	void ProcessCharacterArray(bool bUpdateMap);
	
	void RebuildCharacterMap();
};

#undef LOCTEXT_NAMESPACE


