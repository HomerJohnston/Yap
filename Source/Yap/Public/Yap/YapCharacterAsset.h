// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "Interfaces/IYapCharacterInterface.h"

#include "YapCharacterAsset.generated.h"

class UFlowNode_YapDialogue;
enum class EFlowYapCharacterMood : uint8;

#define LOCTEXT_NAMESPACE "Yap"

// TODO data validation - on packaging make sure the portraits key tags list matches project settings
// TODO add validation warning to the details customization
// TODO add a "skip warning" bool to portrait entries, to make it allowable for them to be unset (on packaging, any unset textures should, by default, log a warning message)

USTRUCT()
struct YAP_API FYapPortraitList
{
	GENERATED_BODY()

	/** Texture for each mood. Moods are stored as FNames instead of Gameplay Tags for easier handling. */
	UPROPERTY(EditAnywhere, EditFixedSize, meta=(ReadOnlyKeys, ForceInlineRow))
	TMap<FName, TObjectPtr<UTexture2D>> Map;

	
};

/**
 * This is an OPTIONAL asset to create characters for your game. Yap can use any blueprint or asset which implements the IYapCharacterInterface.
 * If you already have character classes built for your game, just use this class (UYapCharacterAsset) as a reference source to help implement the interface on your own classes.
 */
UCLASS(meta = (DataAssetCategory = "TODO"))
class YAP_API UYapCharacterAsset : public UObject, public IYapCharacterInterface
{
#if WITH_EDITOR
	friend class FDetailCustomization_YapCharacter;
#endif

	GENERATED_BODY()
public:
	UYapCharacterAsset();

protected:
	/** Human-readable name of this character or entity. */
	UPROPERTY(EditAnywhere, Category = "Character", BlueprintReadOnly, DisplayName = "Name")
	FText EntityName;

	/** Color for display. */
	UPROPERTY(EditAnywhere, Category = "Character", BlueprintReadOnly, DisplayName = "Color")
	FLinearColor EntityColor;
	
	/** Default portrait texture. This is used when a fragment has "no" mood tag and is also used for the asset thumbnail. */
	UPROPERTY(EditAnywhere, Category = "Portraits", DisplayName = "Default Portrait")
	TObjectPtr<UTexture2D> Portrait;

	// These use FNames instead of FGameplayTags to avoid interfering with the asset referencer (allowing me to automatically populate this list without preventing users from deleting gameplay tags)
	/** Portrait textures, raw map access (this is normally the same as the customized view above, you do not normally need to edit this). If you edit this you will need to close and reopen the asset. */ 
	UPROPERTY(EditAnywhere, Category = "Portraits", AdvancedDisplay, DisplayName = "Portraits Map (Raw Access)", meta = (ForceInlineRow))
	TMap<FName, FYapPortraitList> PortraitsMap;
	
	/** OBSOLETE - This will be transferred automatically to the new PortraitsMap property upon saving. */
	UPROPERTY()
	TMap<FName, TObjectPtr<UTexture2D>> Portraits;
	
	// --------------------- //
	/* IYapSpeaker Interface */
public:

	FText GetCharacterName() const override { return EntityName; };

	FLinearColor GetCharacterColor() const override { return EntityColor; };
	
	const UTexture2D* GetCharacterPortrait(const FGameplayTag& MoodTag) const override;

	/* IYapSpeaker Interface */
	// --------------------- //

	#if WITH_EDITOR
public:
	void PostLoad() override;

	void PreSave(FObjectPreSaveContext SaveContext) override;
	
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void RefreshPortraitList();

	bool HasAnyMoodTags() const;
	
	FGameplayTagContainer GetAllMoodTags() const;

	bool GetHasWarnings() const;
	
	bool GetHasObsoletePortraitData() const;
	
	bool GetPortraitsOutOfDate() const;
#endif

};

#undef LOCTEXT_NAMESPACE
