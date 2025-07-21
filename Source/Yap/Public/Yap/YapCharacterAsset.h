// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "Interfaces/IYapCharacterInterface.h"

#include "YapCharacterAsset.generated.h"

enum class EFlowYapCharacterMood : uint8;

#define LOCTEXT_NAMESPACE "Yap"

// TODO data validation - on packaging make sure the portraits key tags list matches project settings
// TODO add validation warning to the details customization
// TODO add a "skip warning" bool to portrait entries, to make it allowable for them to be unset (on packaging, any unset textures should, by default, log a warning message)
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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Name", Category = "Default")
	FText EntityName;

	/** Color for display. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Color", Category = "Default")
	FLinearColor EntityColor;
	
	/** Used to find this actor in the world. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	FGameplayTag IdentityTag;
	
	/** If set, the character will use a single portrait texture for all moods. */
    UPROPERTY(EditAnywhere, Category = "Default")
	bool bUseSinglePortrait = false;

	/** Portrait texture to use. */
    UPROPERTY(EditAnywhere, EditFixedSize, meta=(ReadOnlyKeys, ForceInlineRow, EditCondition = "bUseSinglePortrait", EditConditionHides), Category = "Default")
	TObjectPtr<UTexture2D> Portrait;
	
	/** Avatar icons to use in dialogue UI */
    UPROPERTY(EditAnywhere, EditFixedSize, meta=(ReadOnlyKeys, ForceInlineRow, EditCondition = "!bUseSinglePortrait", EditConditionHides), Category = "Default")
	TMap<FName, TObjectPtr<UTexture2D>> Portraits;

	// --------------------- //
	/* IYapSpeaker Interface */
public:

	FText GetYapCharacterName() const override { return EntityName; };

	FLinearColor GetYapCharacterColor() const override { return EntityColor; };

	FGameplayTag GetYapCharacterTag() const override { return IdentityTag; };
	
	const UTexture2D* GetYapCharacterPortrait(const FGameplayTag& MoodTag) const override;

	/* IYapSpeaker Interface */
	// --------------------- //

	#if WITH_EDITOR
public:
	void PostLoad() override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void RefreshPortraitList();
#endif

};

#undef LOCTEXT_NAMESPACE
