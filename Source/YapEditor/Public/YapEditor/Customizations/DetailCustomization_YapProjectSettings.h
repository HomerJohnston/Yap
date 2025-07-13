// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IDetailCustomization.h"

class UYapProjectSettings;
class SGameplayTagPicker;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapProjectSettings : public IDetailCustomization
{
private:
	FName DialogueTagsParent {"DialogueTagsParent"};

	FSlateFontInfo DetailFont;
	
	IDetailLayoutBuilder* CachedDetailBuilder = nullptr;

	TSharedPtr<IPropertyHandle> CachedCharacterArrayPropertyHandle;
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapProjectSettings());
	}

protected:
	TWeakObjectPtr<UYapProjectSettings> ProjectSettings;

	static void SortCategory(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap, int32& Order, TSet<FName>& SortedCategories, FName NextCategory);

	static void CustomSortYapProjectSettingsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap);

	const FSlateBrush* TODOBorderImage() const;

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnClicked_OpenDialogueTagsManager(const FGameplayTag& DomainTag);

	FReply OnClicked_RefreshMoodTagIcons();
	
	FText ToolTipText_DefaultMoodTags() const;

	bool IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const;

	FReply OnClicked_SortCharacters() const;

	FReply OnClicked_DeleteEmptyCharacters() const;

	FReply OnClicked_PopulateFromParent() const;
	
	void ProcessCategory(IDetailCategoryBuilder& Category, IDetailLayoutBuilder& DetailBuilder);
	
private:
	FText GetDeletedTagsText(const TArray<FName>& TagNamesToDelete);
};

#undef LOCTEXT_NAMESPACE
