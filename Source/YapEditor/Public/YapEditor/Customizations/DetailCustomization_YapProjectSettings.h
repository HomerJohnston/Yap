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
	//FSlateColor ForegroundColor_Button(FLinearColor Col, TSharedPtr<SButton> Button) const { return YapColor::Error; };

private:
	TArray<FString> DefaultMoodTags
	{
		"Angry",
		"Calm",
		"Confused",
		"Disgusted",
		"Happy",
		"Injured",
		"Laughing",
		"Panicked",
		"Sad",
		"Scared",
		"Smirking",
		"Stressed",
		"Surprised",
		"Thinking",
		"Tired"
	};

	FName DialogueTagsParent {"DialogueTagsParent"};

	FSlateFontInfo DetailFont;
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapProjectSettings());
	}

protected:
	TWeakObjectPtr<UYapProjectSettings> ProjectSettings;
	
	FText GetMoodTags() const;

	const FSlateBrush* TODOBorderImage() const;

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	FReply OnClicked_ResetDefaultMoodTags() const;

	FReply OnClicked_DeleteAllMoodTags() const;

	FReply OnClicked_OpenMoodTagsManager();

	FReply OnClicked_OpenDialogueTagsManager(const FGameplayTag& DomainTag);

	/*
	FReply OnClicked_CleanupDialogueTags();
	*/
	
	FReply OnClicked_RefreshMoodTagIcons();
	
	FText ToolTipText_DefaultMoodTags() const;

	bool IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const;

	FReply OnClicked_SortCharacters() const;

	FReply OnClicked_DeleteEmptyCharacters() const;

	FReply OnClicked_PopulateFromParent() const;
	
	void ProcessCategory(IDetailCategoryBuilder& Category) const;
	
private:
	FText GetDeletedTagsText(const TArray<FName>& TagNamesToDelete);
};

#undef LOCTEXT_NAMESPACE
