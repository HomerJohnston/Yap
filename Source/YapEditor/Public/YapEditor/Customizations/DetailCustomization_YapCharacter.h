// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "IDetailCustomization.h"

class UYapCharacter;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapCharacter : public IDetailCustomization
{
private:
	TWeakObjectPtr<UYapCharacter> CharacterBeingCustomized;
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapCharacter());
	}


	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	
	EVisibility Visibility_MoodTagsOutOfDateWarning() const;
	
	FText Text_PortraitsListHint() const;

	void OnClicked_RefreshMoodTagsButton() const;

	TSharedPtr<IPropertyHandle> bUseSinglePortraitProperty;
	TSharedPtr<IPropertyHandle> PortraitProperty;
	TSharedPtr<IPropertyHandle> PortraitsProperty;

	const TMap<FName, TObjectPtr<UTexture2D>>& GetPortraitsMap() const;
};

#undef LOCTEXT_NAMESPACE