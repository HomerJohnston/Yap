// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapCharacterAsset.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapNodeBlueprint.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapMoodTags.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/SlateWidgets/SYapHyperlink.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FDetailCustomization_YapCharacterAsset::FDetailCustomization_YapCharacterAsset()
{
	ButtonWidth = 300;

	Handle = UGameplayTagsManager::OnEditorRefreshGameplayTagTree.AddRaw(this, &FDetailCustomization_YapCharacterAsset::RefreshDetails);
	
	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}
}

FDetailCustomization_YapCharacterAsset::~FDetailCustomization_YapCharacterAsset()
{
	UGameplayTagsManager::OnEditorRefreshGameplayTagTree.Remove(Handle);

	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

FText FDetailCustomization_YapCharacterAsset::Text_MoodTag(FGameplayTag MoodRoot, FGameplayTag MoodTag) const
{
	const TWeakObjectPtr<const UYapNodeConfig>* Config = NodeConfigs.Find(MoodRoot);
	
	if (!Config)
	{
		return LOCTEXT("MoodTagIcon_MissingConfigError", "Error: Missing config");
	}

	if (MissingMoodTagIcons.Contains(MoodTag))
	{
		return FText::Format(LOCTEXT("MoodTagIcon_MissingIconResource", "Could not find icon: {0}"), FText::FromString((*Config)->GetMoodTagIconPath(MoodTag, "png (or svg)")));
	}

	
	return FText::GetEmpty();	
}

void FDetailCustomization_YapCharacterAsset::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	if (!CacheEditedCharacterAsset(InDetailBuilder))
	{
		return;
	}

	CachePropertyHandles(InDetailBuilder);
	
	DrawObsoleteDataWarning(InDetailBuilder);
	
	SortCategories(InDetailBuilder);
	
	IDetailCategoryBuilder& CharacterCategory = InDetailBuilder.EditCategory("Character");
	CharacterCategory.AddProperty(DefaultPortraitProperty);

	IDetailCategoryBuilder& PortraitsCategory = InDetailBuilder.EditCategory("Portraits", LOCTEXT("PortraitsCategory_Title", "Portraits"));

	TSharedPtr<IPropertyHandleMap> PortraitsMap = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, PortraitsMap))->AsMap();

	uint32 NumMoodRoots;
	PortraitsMap->GetNumElements(NumMoodRoots);

	for (uint32 i = 0; i < NumMoodRoots; ++i)
	{
		TSharedPtr<IPropertyHandle> ListElement = PortraitsMap->GetElement(i);
		TSharedPtr<IPropertyHandle> ListKey = ListElement->GetKeyHandle();
		TSharedPtr<IPropertyHandleMap> ListElementAsMap = ListElement->AsMap();

		FName ListKeyName;
		ListKey->GetValue(ListKeyName);

		FGameplayTag MoodRoot = FGameplayTag::RequestGameplayTag(ListKeyName, false);

		FText ListHeading;

		if (MoodRoot.IsValid())
		{
			ListHeading = FText::Format(LOCTEXT("MoodTagCategory_Header", "{0}"), FText::FromName(ListKeyName));// FText::FromName(ListKeyName);
		}
		else
		{
			ListHeading = FText::Format(LOCTEXT("InvalidMoodTag", "INVALID: {0}"), FText::FromName(ListKeyName));
		}
		
		IDetailGroup& Grp = PortraitsCategory.AddGroup(ListKeyName, ListHeading, false, true);

		FDetailWidgetRow& GrpHeader = Grp.HeaderRow()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SColorBlock)
				.Color(YapColor::Noir_Trans)
			]
			+ SOverlay::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.Padding(8)
			[
				SNew(STextBlock)
				.Text(ListHeading)
				.Font(YapFonts.Font_CharacterMoodRootHeading)
			]
		];
				
		TSharedPtr<IPropertyHandle> MoodMap = ListElement->GetChildHandle(0); // FYapPortraitList only has one property, a map
		TSharedPtr<IPropertyHandleMap> MoodMapAsMap = MoodMap->AsMap();

		const UYapNodeConfig& Config = Yap::GetConfigUsingMoodRoot(MoodRoot);

		NodeConfigs.Add(MoodRoot, &Config);
		
		uint32 NumMoods;
		MoodMapAsMap->GetNumElements(NumMoods);

		for (uint32 j = 0; j < NumMoods; ++j)
		{
			TSharedPtr<IPropertyHandle> MoodMapElement = MoodMapAsMap->GetElement(j);
			TSharedPtr<IPropertyHandle> MoodNameProperty = MoodMapElement->GetKeyHandle();

			void* TextureAddress;
			MoodMapElement->GetValueData(TextureAddress);

			FName MoodName;
			MoodNameProperty->GetValue(MoodName);

			FGameplayTag MoodTag = FGameplayTag::RequestGameplayTag(MoodName, false);

			TSharedPtr<SWidget> NameContentWidget = nullptr;

			if (MoodTag.IsValid())
			{
				NameContentWidget = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 8, 0)
				[
					SNew(SImage)
					.Image(this, &FDetailCustomization_YapCharacterAsset::Image_MoodTag, MoodRoot, MoodTag)
					.ToolTipText(this, &FDetailCustomization_YapCharacterAsset::Text_MoodTag, MoodRoot, MoodTag)
					.ColorAndOpacity(YapColor::White)
					.DesiredSizeOverride(FVector2d(24, 24))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromName(Yap::Tags::GetLeafOfTag(MoodTag)))
					.Font(YapFonts.Font_CharacterMoodRootHeading)
				];
			}
			else
			{
				NameContentWidget = SNew(STextBlock)
				.Font(InDetailBuilder.GetDetailFont())
				.Text(FText::Format(LOCTEXT("InvalidMoodTag", "INVALID: {0}"), FText::FromName(MoodName)))
				.ColorAndOpacity(YapColor::Orange);
			}
			
			Grp.AddWidgetRow()
			.NameContent()
			[
				NameContentWidget.ToSharedRef()
			]
			.ValueContent()
			[
				MoodMapElement->CreatePropertyValueWidget()	
			];
		}
	}
	
	AddBottomControls(PortraitsCategory);
	
	// TEMP
	//PortraitsCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, PortraitsMap));
}

void FDetailCustomization_YapCharacterAsset::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder)
{
	CachedDetailBuilder = InDetailBuilder;
	CustomizeDetails(*InDetailBuilder);
}

EVisibility FDetailCustomization_YapCharacterAsset::Visibility_MoodTagsOutOfDateWarning() const
{
	if (!CharacterBeingCustomized.IsValid())
	{
		return EVisibility::Collapsed;
	}

	return CharacterBeingCustomized->GetPortraitsOutOfDate() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FDetailCustomization_YapCharacterAsset::Text_PortraitsListHint() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		FGameplayTagContainer MoodTagRoots = Yap::GetMoodTagRoots();

		if (MoodTagRoots.Num() == 0)
		{
			return LOCTEXT("CharacterPortraits_MoodTagRootsEmpty_Info_1", "To enable mood portraits, create a Yap Node Config asset and set a root mood tag in it.");
		}
		
		FGameplayTagContainer MoodTags = Yap::GetAllMoodTagsUnder(MoodTagRoots);
		
		if (MoodTags.Num() == 0)
		{
			return LOCTEXT("CharacterPortraits_MoodTagsEmpty_Info_1", "You need to create gameplay tags for moods. ");
		}
		else
		{
			return FText::GetEmpty();
		}
	}

	return INVTEXT("ERROR");
}

void FDetailCustomization_YapCharacterAsset::OnClicked_RefreshMoodTagsButton() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		FYapScopedTransaction T(YapTransactions::RefreshCharacterPortraitList, LOCTEXT("RefreshCharacterPortraitList_Transaction", "Refresh character portrait list"), CharacterBeingCustomized.Get());

		TSharedPtr<IPropertyHandleMap> PortraitsPropertyAsMap = PortraitsProperty->AsMap();

		FGameplayTagContainer MoodRoots = Yap::GetMoodTagRoots();

		FGameplayTagContainer AlreadyExistingMoodRoots;

		// First clear any NONE entries
		//RemoveEntryFromMap(PortraitsPropertyAsMap, NAME_None);

		// ====================================================================================
		// EPISODE 1: The Phantom Tag - first step, iterate through the map and find invalid mood roots. Erase those entire lists.
		{
			bool bLoopAgain = true;

			while (bLoopAgain)
			{
				bLoopAgain = false;
			
				uint32 NumLists;
				PortraitsPropertyAsMap->GetNumElements(NumLists);
			
				// Iterate through the portraits map and remove any invalid mood tags
				for (uint32 i = 0; i < NumLists; ++i)
				{
					TSharedPtr<IPropertyHandle> PortraitListElement = PortraitsPropertyAsMap->GetElement(i);

					if (PortraitListElement.IsValid())
					{
						TSharedPtr<IPropertyHandle> KeyProperty = PortraitListElement->GetKeyHandle();

						if (KeyProperty.IsValid())
						{
							FName PortraitListKey;
							KeyProperty->GetValue(PortraitListKey);

							FGameplayTag PortraitListKeyAsTag = FGameplayTag::RequestGameplayTag(PortraitListKey, false);

							if (!PortraitListKeyAsTag.IsValid() || !MoodRoots.HasTagExact(PortraitListKeyAsTag))
							{
								PortraitsPropertyAsMap->DeleteItem(i);

								// We will restart from the beginning any time we modify the map to be safe. I don't know if this is necessary but this process should be so small anyway.
								bLoopAgain = true;
								break;
							}
							else
							{
								AlreadyExistingMoodRoots.AddTag(PortraitListKeyAsTag);
							}
						}
					}
				}
			}
		}

		// ====================================================================================
		// EPISODE 2: Attack of The Tags - second step, iterate through the project's mood roots and add any which do not exist in the map yet 
		{
			for (const FGameplayTag& Root : MoodRoots)
			{
				if (!AlreadyExistingMoodRoots.HasTagExact(Root))
				{
					AddEntryToMap(PortraitsPropertyAsMap, Root);
				}
			}
		}

		// ====================================================================================
		// EPISODE 3: Revenge of the Tags - third step, for each list, iterate through the project's mood roots and add any which do not exist in the map yet 
		{
			uint32 NumLists = 0;

			using PropHandle = TSharedPtr<IPropertyHandle>;
			
			// Iterate through the map and find the "None" element and set its key
			PortraitsPropertyAsMap->GetNumElements(NumLists);

			for (uint32 i = 0; i < NumLists; ++i)
			{
				PropHandle PortraitListElement = PortraitsPropertyAsMap->GetElement(i);

				if (PortraitListElement.IsValid())
				{
					PropHandle KeyProperty = PortraitListElement->GetKeyHandle();

					FName KeyNameTest;
					KeyProperty->GetValue(KeyNameTest);

					if (KeyProperty.IsValid())
					{
						// WARNING: Right now the only property inside my list is the map. If I ever change the layout of FYapPortraitList, this code will break!
						RefreshList(KeyProperty, PortraitListElement->GetChildHandle(0));
					}
				}
			}
		}
	}

	auto SortNames = [] (const FName& N1, const FName& N2) { return N1.Compare(N2) < 0; };
	
	CharacterBeingCustomized->PortraitsMap.KeySort( SortNames );

	for (TPair<FName, FYapPortraitList>& KVP : CharacterBeingCustomized->PortraitsMap)
	{
		KVP.Value.Map.KeySort( SortNames );
	}
	
	RefreshDetails();
}

bool FDetailCustomization_YapCharacterAsset::CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder)
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

void FDetailCustomization_YapCharacterAsset::CachePropertyHandles(IDetailLayoutBuilder& DetailBuilder)
{
	DefaultPortraitProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portrait));
	DefaultPortraitProperty->MarkHiddenByCustomization();

	OBSOLETE_PortraitsProperty_OBSOLETE = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portraits));
	OBSOLETE_PortraitsProperty_OBSOLETE->MarkHiddenByCustomization();

	PortraitsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, PortraitsMap));
	PortraitsProperty->MarkHiddenByCustomization();
}

void FDetailCustomization_YapCharacterAsset::SortCategories(IDetailLayoutBuilder& DetailBuilder) const
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

void FDetailCustomization_YapCharacterAsset::DrawObsoleteDataWarning(IDetailLayoutBuilder& DetailBuilder)
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
					.OnNavigate(this, &FDetailCustomization_YapCharacterAsset::OnClicked_FixupOldPortraitsMap)
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

	IDetailPropertyRow& Row = CategoryBuilder.AddProperty(OBSOLETE_PortraitsProperty_OBSOLETE);

	CategoryBuilder.AddCustomRow(INVTEXT("TODO"))
	.Visibility(Vis)
	[
		SNew(SColorBlock)
		.Color(YapColor::WarningText.Desaturate(0.25))
	];
}

bool FDetailCustomization_YapCharacterAsset::GetHasPortraitData() const
{
	void* PortraitsMapDataPtr = nullptr;
	
	PortraitsProperty->GetValueData(PortraitsMapDataPtr);

	TMap<FGameplayTag, TObjectPtr<UTexture2D>>* PortraitsMapPtr = static_cast<TMap<FGameplayTag, TObjectPtr<UTexture2D>>*>(PortraitsMapDataPtr);

	return PortraitsMapPtr->Num() > 0;
}

bool FDetailCustomization_YapCharacterAsset::GetHasObsoleteData() const
{
	if (CharacterBeingCustomized.IsValid())
	{
		return CharacterBeingCustomized->GetHasObsoletePortraitData();
	}

	return false;
}

void FDetailCustomization_YapCharacterAsset::OnClicked_FixupOldPortraitsMap()
{
	FYapScopedTransaction Transaction(YapTransactions::FixupOldPortraitsMap, LOCTEXT("FixupOldPortraitsMap_Transaction", "Fixup old portraits map"), CharacterBeingCustomized.Get());

	if (!FixupOldPortraitsMap())
	{
		Transaction.AbortAndUndo();
	}
}

bool FDetailCustomization_YapCharacterAsset::FixupOldPortraitsMap()
{
	CharacterBeingCustomized->Modify();
	
	void* OldPortraitsMapDataPtr = nullptr;
	OBSOLETE_PortraitsProperty_OBSOLETE->GetValueData(OldPortraitsMapDataPtr);
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
		PortraitsProperty->GetValueData(NewPortraitsMapDataPtr);
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

void FDetailCustomization_YapCharacterAsset::AddEntryToMap(TSharedPtr<IPropertyHandleMap> Map, const FGameplayTag& NewKey) const
{
	uint32 NumElements;
	
	// Add a new "None" element
	Map->AddItem();

	// Iterate through the map and find the "None" element and set its key
	Map->GetNumElements(NumElements);

	for (uint32 i = 0; i < NumElements; ++i)
	{
		TSharedPtr<IPropertyHandle> Element = Map->GetElement(i);

		if (Element.IsValid())
		{
			TSharedPtr<IPropertyHandle> KeyProperty = Element->GetKeyHandle();

			if (KeyProperty.IsValid())
			{
				FName KeyAsName;
				KeyProperty->GetValue(KeyAsName);

				FGameplayTag KeyAsTag = FGameplayTag::RequestGameplayTag(KeyAsName, false);

				// We found the new "None" element
				if (!KeyAsTag.IsValid())
				{
					KeyProperty->SetValueFromFormattedString(NewKey.GetTagName().ToString());
					break;
				}
			}
		}
	}
}

void FDetailCustomization_YapCharacterAsset::RefreshList(TSharedPtr<IPropertyHandle> MoodRootProperty, TSharedPtr<IPropertyHandle> ListProperty) const
{
	using PropHandle = TSharedPtr<IPropertyHandle>;
	using MapHandle = TSharedPtr<IPropertyHandleMap>;

	// WARNING: Right now the only property inside my list is the map. If I ever change the layout of FYapPortraitList, this code will break!
	MapHandle Map = ListProperty->AsMap(); // FYapPortraitList.Map

	FName MoodRootName;
	MoodRootProperty->GetValue(MoodRootName); // UYapCharacterAsset.PortraitsMap.Key

	FGameplayTag MoodRootTag = FGameplayTag::RequestGameplayTag(MoodRootName, false);

	if (!MoodRootTag.IsValid())
	{
		UE_LOG(LogYapEditor, Error, TEXT("Failed to parse root mood tag of list! This should never happen."));
		return;
	}
	
	// Find all existing mood tags.
	if (Map.IsValid())
	{
		TSet<FName> ExistingMoodTagNames;
		FGameplayTagContainer RequiredMoodTags = Yap::GetAllMoodTagsUnder(MoodRootTag);

		uint32 NumMoodEntries = 0;
		Map->GetNumElements(NumMoodEntries);
		
		for (uint32 j = 0; j < NumMoodEntries; ++j)
		{
			PropHandle MoodEntryElement = Map->GetElement(j);

			if (MoodEntryElement.IsValid())
			{
				PropHandle MoodEntryKey = MoodEntryElement->GetKeyHandle();

				if (MoodEntryKey.IsValid())
				{
					FName MoodEntryName;
					MoodEntryKey->GetValue(MoodEntryName);

					//if (MoodEntryName != NAME_None)
					//{
						ExistingMoodTagNames.Add(MoodEntryName);
					//}
				}
			}
		}

		// Ensure there are no "None" entries
		//RemoveEntryFromMap(Map, FGameplayTag::EmptyTag.GetTagName());

		// Ensure there are no invalid entries (e.g. developers removed mood tags)
		for (const FName& ExistingTagName : ExistingMoodTagNames)
		{
			FGameplayTag ExistingTag = FGameplayTag::RequestGameplayTag(ExistingTagName, false);
		
			if (RequiredMoodTags.HasTagExact(ExistingTag))
			{
				continue;
			}

			RemoveEntryFromMap(Map, ExistingTagName);
		}
	
		// Ensure all tags exist (e.g. developers added mood tags)
		for (const FGameplayTag& RequiredTag : RequiredMoodTags)
		{
			if (ExistingMoodTagNames.Contains(RequiredTag.GetTagName()))
			{
				continue;
			}

			AddEntryToMap(Map, RequiredTag);
		}
	}
}

void FDetailCustomization_YapCharacterAsset::RemoveEntryFromMap(TSharedPtr<IPropertyHandleMap> Map, const FName& DeadKey) const
{
	uint32 NumElements;
	
	// Iterate through the map and find the element
	Map->GetNumElements(NumElements);

	for (uint32 i = 0; i < NumElements; ++i)
	{
		TSharedPtr<IPropertyHandle> Element = Map->GetElement(i);

		if (Element.IsValid())
		{
			TSharedPtr<IPropertyHandle> KeyProperty = Element->GetKeyHandle();

			if (KeyProperty.IsValid())
			{
				FName KeyAsName;
				KeyProperty->GetValue(KeyAsName);

				// We found the new "None" element
				if (KeyAsName == DeadKey)
				{
					Map->DeleteItem(i);
					break;
				}
			}
		}
	}
}

void FDetailCustomization_YapCharacterAsset::RefreshDetails() const
{
	if (CachedDetailBuilder.IsValid())
	{
		IDetailLayoutBuilder* Layout = nullptr;
		if (auto LockedDetailBuilder = CachedDetailBuilder.Pin())
		{
			Layout = LockedDetailBuilder.Get();
		}
		if (LIKELY(Layout))
		{
			Layout->ForceRefreshDetails();
		}
	}
}

void FDetailCustomization_YapCharacterAsset::AddBottomControls(IDetailCategoryBuilder& Category) const
{
	Category.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
	[
		SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 12, 0, 8)
		[
			SNew(SBox)
			.IsEnabled_Lambda( [this] () { return GetHasObsoleteData() ? false : true; } )
			.Visibility_Lambda( [this] () { return Yap::GetMoodTagRoots().Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.Visibility(this, &FDetailCustomization_YapCharacterAsset::Visibility_MoodTagsOutOfDateWarning)
					.SimpleTextMode(true)
					.Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh", "Portraits list is out of date. "))
					.ColorAndOpacity(YapColor::OrangeRed)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SYapHyperlink)
					.Text(LOCTEXT("CharacterPortraits_PerformMoodTagsRefresh", "Click to refresh."))
					.OnNavigate(this, &FDetailCustomization_YapCharacterAsset::OnClicked_RefreshMoodTagsButton)
					.ToolTipText(LOCTEXT("RefreshMoodTags_ToolTIp", "Will process the portraits list, removing entries which are no longer present in project settings, and adding missing entries. Also sorts the list."))
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
					.Text(this, &FDetailCustomization_YapCharacterAsset::Text_PortraitsListHint)
					.ColorAndOpacity(YapColor::LightYellow)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(4)
				[
					SNew(SYapHyperlink)
					.Visibility_Lambda( [this] () { return Yap::GetMoodTagRoots().Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
					.Text(LOCTEXT("CharacterPortraits_OpenProjectSettings", "Edit Mood Tags"))
					.OnNavigate_Lambda( [] () {Yap::EditorFuncs::OpenGameplayTagsEditor(Yap::GetMoodTagRoots()); } )
				]
			]
		]
	];
}

void FDetailCustomization_YapCharacterAsset::PostUndo(bool bSuccess)
{
	RefreshDetails();
}

void FDetailCustomization_YapCharacterAsset::PostRedo(bool bSuccess)
{
	RefreshDetails();
}

const FSlateBrush* FDetailCustomization_YapCharacterAsset::Image_MoodTag(FGameplayTag MoodRoot, FGameplayTag MoodTag) const
{
	const TWeakObjectPtr<const UYapNodeConfig>* Config = NodeConfigs.Find(MoodRoot);

	if (Config)
	{
		const FSlateBrush* Brush = (*Config)->GetMoodTagBrush(MoodTag);

		if (Brush)
		{
			return Brush;
		}
	}

	FDetailCustomization_YapCharacterAsset* MutableThis = const_cast<FDetailCustomization_YapCharacterAsset*>(this);
	MutableThis->MissingMoodTagIcons.Add(MoodTag);
	
	return FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Circle_Alert);
}

#undef LOCTEXT_NAMESPACE
