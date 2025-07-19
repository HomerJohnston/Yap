// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapCharacter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapNodeBlueprint.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapMoodTags.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/SlateWidgets/SYapHyperlink.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FDetailCustomization_YapCharacter::FDetailCustomization_YapCharacter()
{
	ButtonWidth = 300;
}

void FDetailCustomization_YapCharacter::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (!CacheEditedCharacterAsset(DetailBuilder))
	{
		return;
	}

	CachePropertyHandles(DetailBuilder);
	
	DrawObsoleteDataWarning(DetailBuilder);
	
	SortCategories(DetailBuilder);
	
	IDetailCategoryBuilder& Portraits = DetailBuilder.EditCategory("Portraits");

	Portraits.AddProperty(DefaultPortraitProperty);
	
	Portraits.AddProperty(PortraitsMapProperty)
	.IsEnabled(TAttribute<bool>::CreateLambda([this] ()
	{
		return !GetHasObsoleteData();
	}))
	
	.ShouldAutoExpand(true);
	
	FDetailWidgetRow X = Portraits.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
	[
		SNew(SVerticalBox)
		/*
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 8)
		[
			SNew(SButton)
			.IsEnabled_Lambda( [this] ()
			{
				// If we have obsolete data, we don't want to allow adding new data
				return !GetHasObsoleteData();
			})
			.OnClicked_Lambda(
			[this] () -> FReply
			{
				if (CharacterBeingCustomized.IsValid())
				{
					FYapScopedTransaction T(YapTransactions::AddMoodTagPortrait, LOCTEXT("AddMoodTagPortrait_Transaction", "Add mood tag portrait"), CharacterBeingCustomized.Get());

					CharacterBeingCustomized->RefreshPortraitList();
				}
				
				return FReply::Handled();
			})
			[
				SNew(SBox)
				.MinDesiredWidth(ButtonWidth)
				[
					SNew(STextBlock)
					.Text(INVTEXT("Rebuild Portrait Map Keys"))	
				]
			]
		]
		*/
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 12, 0, 8)
		[
			SNew(SBox)
			.Visibility(this, &FDetailCustomization_YapCharacter::Visibility_MoodTagsOutOfDateWarning)
			.IsEnabled_Lambda( [this] () { return GetHasObsoleteData() ? false : true; } )
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.SimpleTextMode(true)
					.Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh", "Portraits list is out of date, "))
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(4)
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.SimpleTextMode(true)
					.Text(this, &FDetailCustomization_YapCharacter::Text_PortraitsListHint)
					.ColorAndOpacity(YapColor::LightYellow)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(4)
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("CharacterPortraits_OpenProjectSettings", "Edit Mood Tags (Filtered)"))
					.OnNavigate_Lambda( [] () {Yap::EditorFuncs::OpenGameplayTagsEditor(Yap::GetMoodTagRoots()); } )
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(4)
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("CharacterPortraits_OpenProjectSettings", "Edit Mood Tags (All)"))
					.OnNavigate_Lambda( [] () {Yap::EditorFuncs::OpenGameplayTagsEditor(); } )
				]
			]
		]
	];
}

EVisibility FDetailCustomization_YapCharacter::Visibility_MoodTagsOutOfDateWarning() const
{
	if (!CharacterBeingCustomized.IsValid())
	{
		return EVisibility::Collapsed;
	}

	return CharacterBeingCustomized->GetPortraitsOutOfDate() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FDetailCustomization_YapCharacter::Text_PortraitsListHint() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		FGameplayTagContainer MoodTags = Yap::GetAllMoodTags();
		
		if (MoodTags.Num() == 0)
		{
			return LOCTEXT("CharacterPortraits_MoodTagsEmpty_Info_1", "You need to create gameplay tags for moods. ");
		}
		else
		{
			return FText::Format(LOCTEXT("CharacterPortraits_MoodTags_Info_1", "Project has {0} mood tags. "), MoodTags.Num());
		}
	}

	return INVTEXT("ERROR");
}

void FDetailCustomization_YapCharacter::OnClicked_RefreshMoodTagsButton() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		FYapScopedTransaction T(YapTransactions::RefreshCharacterPortraitList, LOCTEXT("RefreshCharacterPortraitList_Transaction", "Refresh character portrait list"), CharacterBeingCustomized.Get());

		CharacterBeingCustomized->RefreshPortraitList();
	}
}

const TMap<FName, TObjectPtr<UTexture2D>>& FDetailCustomization_YapCharacter::GetPortraitsMap() const
{
	TArray<void*> RawData;

	PortraitsProperty_OBSOLETE->AccessRawData(RawData);

	return *reinterpret_cast<const TMap<FName, TObjectPtr<UTexture2D>>*>(RawData[0]);
}

bool FDetailCustomization_YapCharacter::CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1)
	{
		CharacterBeingCustomized = Cast<UYapCharacterAsset>(Objects[0].Get());

		if (CharacterBeingCustomized.IsValid())
		{
			return true;
		}
	}

	return false;
}

void FDetailCustomization_YapCharacter::CachePropertyHandles(IDetailLayoutBuilder& DetailBuilder)
{
	DefaultPortraitProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portrait));
	DefaultPortraitProperty->MarkHiddenByCustomization();

	PortraitsProperty_OBSOLETE = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portraits));
	PortraitsProperty_OBSOLETE->MarkHiddenByCustomization();

	PortraitsMapProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, PortraitsMap));
	PortraitsMapProperty->MarkHiddenByCustomization();
}

void FDetailCustomization_YapCharacter::SortCategories(IDetailLayoutBuilder& DetailBuilder) const
{
	DetailBuilder.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& CategoryMap)
	{
		const TMap<FName, int32> SortOrder =
		{
			{ TEXT("Warnings"), 0},
			{ TEXT("Character"), 1},
			{ TEXT("Portraits"), 2}
		};

		for (const TPair<FName, int32>& SortPair : SortOrder)
		{
			if (CategoryMap.Contains(SortPair.Key))
			{
				CategoryMap[SortPair.Key]->SetSortOrder(SortPair.Value);
			}
		}
	});
}

void FDetailCustomization_YapCharacter::DrawObsoleteDataWarning(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Warnings");

	auto Vis = TAttribute<EVisibility>::CreateLambda([this] ()
	{
		return (GetHasObsoleteData() ? EVisibility::Visible : EVisibility::Collapsed);
	});
	
	CategoryBuilder.AddCustomRow(INVTEXT("TODO"))
	.Visibility(Vis)
	[
		SNew(SColorBlock)
		.Color(YapColor::WarningText.Desaturate(0.25))
	];
	
	CategoryBuilder.AddCustomRow(INVTEXT("TODO"))
	.Visibility(Vis)
	[
		SNew(SBox)
		.HAlign(HAlign_Center)
		.Padding(0, 8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("YapCharacterAsset_ObsoleteDataWarning", "This character has obsolete data. "))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(YapColor::WarningText)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("YapCharacterAsset_FixupOldPortraitsMap", "Migrate"))
					.ToolTipText(LOCTEXT("YapCharacterAsset_FixupOldPortraitsMap_Tooltip", "This will convert the old portraits map to the new format, and remove the old data."))
					.OnNavigate(this, &FDetailCustomization_YapCharacter::OnClicked_FixupOldPortraitsMap)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(YapFonts.Font_WarningText)
				.SimpleTextMode(true)
				.Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh_Info", "You must use the migrate option, or clear the old portraits map, before you can edit new portraits."))
				.ColorAndOpacity(YapColor::WarningText)
			]
		]
	];

	IDetailPropertyRow& Row = CategoryBuilder.AddProperty(PortraitsProperty_OBSOLETE);

	CategoryBuilder.AddCustomRow(INVTEXT("TODO"))
	.Visibility(Vis)
	[
		SNew(SColorBlock)
		.Color(YapColor::WarningText.Desaturate(0.25))
	];
}

bool FDetailCustomization_YapCharacter::GetHasPortraitData() const
{
	void* PortraitsMapDataPtr = nullptr;
	
	PortraitsMapProperty->GetValueData(PortraitsMapDataPtr);

	TMap<FGameplayTag, TObjectPtr<UTexture2D>>* PortraitsMapPtr = static_cast<TMap<FGameplayTag, TObjectPtr<UTexture2D>>*>(PortraitsMapDataPtr);

	return PortraitsMapPtr->Num() > 0;
}

bool FDetailCustomization_YapCharacter::GetHasObsoleteData() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		return CharacterBeingCustomized->GetHasObsoletePortraitData();
	}

	return false;
}

void FDetailCustomization_YapCharacter::OnClicked_FixupOldPortraitsMap()
{
	FYapScopedTransaction Transaction(YapTransactions::FixupOldPortraitsMap, LOCTEXT("FixupOldPortraitsMap_Transaction", "Fixup old portraits map"), CharacterBeingCustomized.Get());

	if (!FixupOldPortraitsMap())
	{
		Transaction.AbortAndUndo();
	}
}

bool FDetailCustomization_YapCharacter::FixupOldPortraitsMap()
{
	CharacterBeingCustomized->Modify();
	
	void* OldPortraitsMapDataPtr = nullptr;
	PortraitsProperty_OBSOLETE->GetValueData(OldPortraitsMapDataPtr);
	TMap<FName, TObjectPtr<UTexture2D>>* OldPortraitsMapPtr = static_cast<TMap<FName, TObjectPtr<UTexture2D>>*>(OldPortraitsMapDataPtr);

	bool bYesAllPressed = false;
	bool bNoAllPressed = false;
	
	for (const TPair<FName, TObjectPtr<UTexture2D>>& KVPair : *OldPortraitsMapPtr)
	{
		FName OldTagName = KVPair.Key;
		TObjectPtr<UTexture2D> OldPortrait = KVPair.Value;

		FGameplayTag ExistingMoodTag = FGameplayTag::RequestGameplayTag(OldTagName, false);

		if (!ExistingMoodTag.IsValid())
		{
			if (bYesAllPressed)
			{
				ExistingMoodTag = Yap::Tags::GetOrAddTag(OldTagName.ToString(), "");
			}
			else if (bNoAllPressed)
			{
				continue;
			}
			else
			{
				EAppReturnType::Type Return = FMessageDialog::Open(
					EAppMsgType::YesNoYesAllNoAllCancel,
					FText::Format(
							LOCTEXT("YapCharacterAsset_MoodTagMissing_Message", "Old tag does not exist. Would you like to add it to your project? If you press \"No\", the portrait entry will be deleted.\n\n{0}"),
							FText::FromName(OldTagName)),
							LOCTEXT("YapCharacterAsset_MoodTagMissing_Title", "Missing Gameplay Tag")
					);

				switch (Return)
				{
					case EAppReturnType::Cancel:
					{
						return false;
					}
					case EAppReturnType::YesAll:
					{
						bYesAllPressed = true;
						// Fall through
					}
					case EAppReturnType::Yes:
					{
						ExistingMoodTag = Yap::Tags::GetOrAddTag(OldTagName.ToString(), "");
						break;
					}
					case EAppReturnType::NoAll:
					{
						bNoAllPressed = true;
						// Fall through
					}
					case EAppReturnType::No:
					{
						continue;
					}
					default: { break; }
				}
			}
		}

		void* NewPortraitsMapDataPtr = nullptr;
		PortraitsMapProperty->GetValueData(NewPortraitsMapDataPtr);
		TMap<FGameplayTag, FYapPortraitList>* NewPortraitsMapPtr = static_cast<TMap<FGameplayTag, FYapPortraitList>*>(NewPortraitsMapDataPtr);
		
		FYapPortraitList& List = NewPortraitsMapPtr->FindOrAdd(ExistingMoodTag.RequestDirectParent());

		TObjectPtr<UTexture2D>& PortraitTexture = List.Map.FindOrAdd(ExistingMoodTag.GetTagName());

		// We already assigned a new portrait texture, notify the user
		if (PortraitTexture != nullptr)
		{
			EAppReturnType::Type Return = FMessageDialog::Open(
				EAppMsgType::OkCancel,
				FText::Format(
						LOCTEXT("YapCharacterAsset_PortraitAlreadyAssigned_Message", "Tag {0} already assigned.\nOld: {1}\nCurrent: {2}\n\nPress OK to ignore the old portrait and continue, or press Cancel to stop."),
						FText::FromName(ExistingMoodTag.GetTagName()),
						FText::FromString(OldPortrait.GetName()),
						FText::FromString(PortraitTexture.GetName())),
						LOCTEXT("YapCharacterAsset_PortraitAlreadyAssigned_Title", "Portrait Conflict")
				);

			switch (Return)
			{
				case EAppReturnType::Cancel:
				{
					return false;
				}
				default:
				{
					
				}
			}
		}

		PortraitTexture = OldPortrait;
	}
	
	OldPortraitsMapPtr->Empty();

	return true;
}

#undef LOCTEXT_NAMESPACE
