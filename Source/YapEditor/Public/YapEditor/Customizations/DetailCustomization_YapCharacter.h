// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "GameplayTagContainer.h"
#include "IDetailCustomization.h"

class UFlowNode_YapDialogue;
class UYapCharacterAsset;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapCharacter : public IDetailCustomization
{
public:
	FDetailCustomization_YapCharacter();

private:
	TWeakObjectPtr<UYapCharacterAsset> CharacterBeingCustomized;

public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapCharacter());
	}
	
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	
	EVisibility Visibility_MoodTagsOutOfDateWarning() const;
	
	FText Text_PortraitsListHint() const;

	void OnClicked_RefreshMoodTagsButton() const;

	float ButtonWidth;
	
	TSharedPtr<IPropertyHandle> DefaultPortraitProperty;
	TSharedPtr<IPropertyHandle> PortraitsProperty_OBSOLETE;
	TSharedPtr<IPropertyHandle> PortraitsMapProperty;

	const TMap<FName, TObjectPtr<UTexture2D>>& GetPortraitsMap() const;

	bool CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder);

	void CachePropertyHandles(IDetailLayoutBuilder& DetailBuilder);
	
	void SortCategories(IDetailLayoutBuilder& DetailBuilder) const;
	
	void DrawObsoleteDataWarning(IDetailLayoutBuilder& DetailBuilder);

	bool GetHasPortraitData() const;
	
	bool GetHasObsoleteData() const;

	void OnClicked_FixupOldPortraitsMap();

	bool FixupOldPortraitsMap();
};

#undef LOCTEXT_NAMESPACE
