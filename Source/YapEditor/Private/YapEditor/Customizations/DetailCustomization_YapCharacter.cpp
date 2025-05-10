// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapCharacter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/SlateWidgets/SYapHyperlink.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FDetailCustomization_YapCharacter::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;

	bUseSinglePortraitProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, bUseSinglePortrait));
	PortraitProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portrait));
	PortraitsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portraits));

	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1)
	{
		CharacterBeingCustomized = Cast<UYapCharacterAsset>(Objects[0].Get());
	}
	else
	{
		return;
	}
	
	DetailBuilder.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& CategoryMap)
	{
		const TMap<FName, int32> SortOrder =
		{
			{ TEXT("YapEntity"), 0},
			{ TEXT("YapCharacter"), 1}
		};

		for (const TPair<FName, int32>& SortPair : SortOrder)
		{
			if (CategoryMap.Contains(SortPair.Key))
			{
				CategoryMap[SortPair.Key]->SetSortOrder(SortPair.Value);
			}
		}
	});
	
	IDetailCategoryBuilder& CharacterCategory = DetailBuilder.EditCategory("YapCharacter");

	bUseSinglePortraitProperty->MarkHiddenByCustomization();
	PortraitProperty->MarkHiddenByCustomization();
	PortraitsProperty->MarkHiddenByCustomization();

	CharacterCategory.AddProperty(bUseSinglePortraitProperty);
	CharacterCategory.AddProperty(PortraitProperty);
	CharacterCategory.AddProperty(PortraitsProperty);

	FDetailWidgetRow X = CharacterCategory.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
	[
		SNew(SVerticalBox)
		.Visibility_Lambda( [this] ()
		{
			bool bUseSinglePortrait;
			bUseSinglePortraitProperty->GetValue(bUseSinglePortrait);

			return bUseSinglePortrait ? EVisibility::Collapsed : EVisibility::Visible;
		})
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 12, 0, 8)
		[
			SNew(SBox)
			.Visibility(this, &FDetailCustomization_YapCharacter::Visibility_MoodTagsOutOfDateWarning)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh", "Portraits list out of date, "))
					.ColorAndOpacity(YapColor::OrangeRed)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("CharacterPortraits_PerformMoodTagsRefresh", "click to refresh"))
					.OnNavigate(this, &FDetailCustomization_YapCharacter::OnClicked_RefreshMoodTagsButton)
					.ToolTipText(LOCTEXT("RefreshMoodTags_ToolTIp", "Will process the portraits list, removing entries which are no longer present in project settings, and adding missing entries."))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 8, 0, 12)
		[
			SNew(SBox)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.Text(this, &FDetailCustomization_YapCharacter::Text_PortraitsListHint)
					.ColorAndOpacity(YapColor::LightYellow)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("CharacterPortraits_OpenProjectSettings", "Project Settings"))
					.OnNavigate_Lambda( [] () { Yap::EditorFuncs::OpenProjectSettings(); } )
				]
			]
		]
	];
}

EVisibility FDetailCustomization_YapCharacter::Visibility_MoodTagsOutOfDateWarning() const
{
	TMap<FName, TObjectPtr<UTexture2D>> PortraitsMap = GetPortraitsMap();
	
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();

	if (PortraitsMap.Num() != ProjectMoodTags.Num())
	{
		return EVisibility::Visible;
	}

	TSet<FName> ProjectMoodTagNames;
	TSet<FName> CharacterMoodTagsAsNames;
	
	ProjectMoodTagNames.Reserve(ProjectMoodTags.Num());
	
	for (const FGameplayTag& Tag : ProjectMoodTags)
	{
		ProjectMoodTagNames.Add(Tag.GetTagName());
	}

	for (const FName& Name : ProjectMoodTagNames)
	{
		if (!PortraitsMap.Contains(Name))
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FText FDetailCustomization_YapCharacter::Text_PortraitsListHint() const
{
	return UYapProjectSettings::GetMoodTags().Num() == 0 ? LOCTEXT("CharacterPortraits_MoodTagsEmpty_Info_1", "You need to create mood tags. Go to ") : LOCTEXT("CharacterPortraits_MoodTags_Info_1", "To edit portrait mood tags, go to ");
}

void FDetailCustomization_YapCharacter::OnClicked_RefreshMoodTagsButton() const
{
	check(CharacterBeingCustomized.IsValid())
	{
		FYapScopedTransaction T(YapTransactions::RefreshCharacterPortraitList, LOCTEXT("RefreshCharacterPortraitList_Transaction", "Refresh character portrait list"), CharacterBeingCustomized.Get());

		CharacterBeingCustomized->RefreshPortraitList();
	}
}

const TMap<FName, TObjectPtr<UTexture2D>>& FDetailCustomization_YapCharacter::GetPortraitsMap() const
{
	TArray<void*> RawData;

	PortraitsProperty->AccessRawData(RawData);

	return *reinterpret_cast<const TMap<FName, TObjectPtr<UTexture2D>>*>(RawData[0]);
}

#undef LOCTEXT_NAMESPACE
