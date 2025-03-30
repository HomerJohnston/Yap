// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapGroupSettings.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsEditorModule.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "SGameplayTagPicker.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Yap/YapTypeGroupSettings.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Globals/YapTagHelpers.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TMap<FName, TSharedPtr<IPropertyHandle>> FPropertyCustomization_YapGroupSettings::DefaultPropertyHandles;

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	GrabOriginalStructPtr(StructPropertyHandle);
	
	IndexChildrenProperties(StructPropertyHandle);
	
	GatherOverrides();
	
	GroupProperties();
	
	SortGroups();
	
	UpdateOverriddenCounts();

	HookUpPropertyChangeDelegates();

	HeaderColorPropertyHolder = SNew(SBox)
		[
			SNew(STextBlock)
			.Text(INVTEXT("Test"))
		];
	
	if (IsDefault())
	{
		HeaderRow.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		];	
	}
	else
	{
		HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];
		
		HeaderRow.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				HeaderColorPropertyHolder.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0, 0, 0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda( [this] () { return FText::Format(LOCTEXT("OverriddenPropertiesCount", "Overridden: {0}"), FText::AsNumber(TotalOverrides)); })
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderColorPropertyHolder->SetContent(StructBuilder.GenerateStructValueWidget(GroupColorPropertyHandle.ToSharedRef()));
	
	for (auto& [CategoryString, PropertyHandles] : PropertyGroups)
	{
		DrawGroup(StructBuilder, CategoryString, PropertyHandles);
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::GrabOriginalStructPtr(TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	void* StructData;

	FPropertyAccess::Result Result = StructPropertyHandle->GetValueData(StructData);

	if (Result == FPropertyAccess::Success)
	{
		Settings = reinterpret_cast<FYapTypeGroupSettings*>(StructData);
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::IndexChildrenProperties(TSharedRef<class IPropertyHandle> StructPropertyHandle)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	IndexedPropertyHandles.Empty(NumChildren);

	// Iterate through all of the properties and add them to the array. We're doing this so that we can always pull identical IPropertyHandles for the properties from one source
	// GetChildHandle always returns a unique handle object, so you can't grab the same property and find in a map (== isn't implemented, only IsSamePropertyNode() which is useless).
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex);

		IndexedPropertyHandles.Add(ChildHandle);

		static const FName GroupColorName = GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, GroupColor);

		FName PropertyName = ChildHandle->GetProperty()->GetFName();

		if (PropertyName == GroupColorName)
		{
			GroupColorPropertyHandle = ChildHandle;
		}

		if (IsDefault())
		{
			DefaultPropertyHandles.Add(ChildHandle->GetProperty()->GetFName(), ChildHandle);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::UpdateOverriddenCounts()
{
	GroupOverridenCounts.Empty();
	TotalOverrides = 0;
	
	for (int32 ChildIndex = 0; ChildIndex < IndexedPropertyHandles.Num(); ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = IndexedPropertyHandles[ChildIndex];

		if (IsOverridden(ChildHandle->GetProperty()->GetFName()))
		{
			FString Category = ChildHandle->GetMetaData("Category");
			int32& Count = GroupOverridenCounts.FindOrAdd(FName(Category));

			Count++;
			TotalOverrides++;
		}
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::HookUpPropertyChangeDelegates()
{
	/*
	for (int32 ChildIndex = 0; ChildIndex < IndexedPropertyHandles.Num(); ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = IndexedPropertyHandles[ChildIndex];
	}
	*/
}

// ------------------------------------------------------------------------------------------------
bool FPropertyCustomization_YapGroupSettings::IsDefault() const
{
	if (Settings)
	{
		return Settings->bDefault;
	}

	checkNoEntry();

	return false;
}

// ------------------------------------------------------------------------------------------------
bool FPropertyCustomization_YapGroupSettings::IsOverridden(FName PropertyName) const
{
	if (IsDefault())
	{
		return false;
	}

	const TSharedPtr<IPropertyHandle>* BoolControlPtr = PropertyBoolControlHandles.Find(PropertyName);

	if (BoolControlPtr)
	{
		TSharedPtr<IPropertyHandle> BoolControl = *BoolControlPtr;

		bool bTemp;
		BoolControl->GetValue(bTemp);

		return bTemp;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
FLinearColor FPropertyCustomization_YapGroupSettings::GetGroupColor()
{
	return Settings->GroupColor;
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::GroupProperties()
{
	TArray<FName> IgnoredProperties
	{
		GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, bDefault),
		GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, GroupColor)
	};
	
	// Iterate through all of the properties and find initial information
	for (int32 ChildIndex = 0; ChildIndex < IndexedPropertyHandles.Num(); ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = IndexedPropertyHandles[ChildIndex];

		// Don't draw the default property, keep it hidden
		if (IgnoredProperties.Contains(ChildHandle->GetProperty()->GetFName()))
		{
			continue;
		}

		// Don't group up bools used to enable named group overrides
		if (ChildHandle->HasMetaData("DefaultOverride"))
		{
			continue;	
		}
		
		FString Category = ChildHandle->GetMetaData("Category");

		TArray<TSharedPtr<IPropertyHandle>>& Group = PropertyGroups.FindOrAdd(Category);

		Group.Emplace(ChildHandle);
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::GatherOverrides()
{
	if (IsDefault())
	{
		return;
	}
	
	for (int32 ChildIndex = 0; ChildIndex < IndexedPropertyHandles.Num(); ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = IndexedPropertyHandles[ChildIndex];
		
		if (ChildHandle->HasMetaData("DefaultOverride"))
		{
			FString OverriddenPropertyNameAsString = ChildHandle->GetMetaData(FName("DefaultOverride"));

			if (OverriddenPropertyNameAsString.IsEmpty())
			{
				// Forgot to specify a property name to override?
				checkNoEntry();
			}
			
			FName OverriddenPropertyName(OverriddenPropertyNameAsString);

			PropertyBoolControlHandles.Add(OverriddenPropertyName, ChildHandle);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::SortGroups()
{
	TArray<FString> Categories
	{
		"Core",
		"Mood Tags",
		"Dialogue Tags",
		"Dialogue Playback",
		"Editor",
		"Audio",
		"Flow Graph Settings",
		"Other",
	};

	PropertyGroups.KeySort([&Categories] (const FString& A, const FString& B)
	{
		int32 IndexA = Categories.Find(A);
		int32 IndexB = Categories.Find(B);

		return IndexA < IndexB;
	});
}

void FPropertyCustomization_YapGroupSettings::DrawGroup(IDetailChildrenBuilder& StructBuilder, FString CategoryString, TArray<TSharedPtr<IPropertyHandle>>& PropertyHandles)
{
	if (PropertyHandles.Num() == 0)
	{
		return;
	}
	
	IDetailGroup& Group = StructBuilder.AddGroup(FName(CategoryString), FText::FromString(CategoryString));

	FDetailWidgetRow& HeaderRow = Group.HeaderRow();

	FName CategoryName(CategoryString);
	
	HeaderRow.NameContent()
	[
		SNew(SBorder)
		.ForegroundColor_Lambda( [this, CategoryName] ()
		{
			FLinearColor Color = GetGroupColor();

			int32* Count = GroupOverridenCounts.Find(CategoryName);
			
			if (!Count || *Count == 0)
			{
				Color *= YapColor::LightGray;
			}
			else
			{
				Color /= YapColor::LightGray;
			}
			
			return Color;
		})
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.None))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(CategoryString))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
	
	for (int32 i = 0; i < PropertyHandles.Num(); ++i)
	{
		TSharedPtr<IPropertyHandle>& Property = PropertyHandles[i];

		DrawProperty(StructBuilder, Group, Property);
	}
}

void FPropertyCustomization_YapGroupSettings::DrawProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{
	if (Property->HasMetaData("DoNotDraw"))
	{
		return;
	}
	
	if (IsDefault())
	{
		DrawDefaultProperty(StructBuilder, Group, Property);
	}
	else
	{
		DrawNamedGroupProperty(StructBuilder, Group, Property);
	}

	DrawExtraPanelContent(Group, Property);
}

void FPropertyCustomization_YapGroupSettings::DrawDefaultProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{	
	Group.AddWidgetRow()
	.NameContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SBorder)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.None))
		.ForegroundColor_Lambda( [this] () { return GetGroupColor(); } )
		[
			Property->CreatePropertyNameWidget()
		]
	]
	.ValueContent()
	[
		(CastField<FStructProperty>(Property->GetProperty()))
			? StructBuilder.GenerateStructValueWidget(Property.ToSharedRef())
			: Property->CreatePropertyValueWidget()
	];
}

void FPropertyCustomization_YapGroupSettings::DrawNamedGroupProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{
	FName PropertyName = Property->GetProperty()->GetFName();
	
	TSharedPtr<IPropertyHandle>* BoolControlPtr = PropertyBoolControlHandles.Find(Property->GetProperty()->GetFName());
	TSharedPtr<IPropertyHandle>* DefaultValuePtr = DefaultPropertyHandles.Find(Property->GetProperty()->GetFName());

	check(BoolControlPtr);
	check(DefaultValuePtr);
	
	TSharedPtr<IPropertyHandle> BoolControl = *BoolControlPtr;
	TSharedPtr<IPropertyHandle> DefaultValue = *DefaultValuePtr;

	TSharedRef<SWidget> NormalValueWidget = 
			(CastField<FStructProperty>(Property->GetProperty()))
				? StructBuilder.GenerateStructValueWidget(Property.ToSharedRef())
				: Property->CreatePropertyValueWidget();
	
	TSharedRef<SWidget> DefaultValueWidget = 
			(CastField<FStructProperty>(DefaultValue->GetProperty()))
				? StructBuilder.GenerateStructValueWidget(DefaultValue.ToSharedRef())
				: DefaultValue->CreatePropertyValueWidget();

	DefaultValueWidget->SetEnabled(false);
	
	Group.AddWidgetRow()
	.NameContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SCheckBox)
		.IsChecked_Lambda( [this, BoolControl] ()
		{
			bool bTemp;
			BoolControl->GetValue(bTemp);
			return bTemp ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda( [this, BoolControl] (ECheckBoxState NewState)
		{
			BoolControl->SetValue(NewState == ECheckBoxState::Checked);
			UpdateOverriddenCounts();
		})
		[
			SNew(SBorder)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.None))
			.ForegroundColor_Lambda( [this, PropertyName] ()
			{
				FLinearColor Color = GetGroupColor();
				
				if (!IsOverridden(PropertyName))
				{
					Color *= YapColor::LightGray;
				}
				else
				{
					Color /= YapColor::LightGray;
				}
				
				return Color;
			})
			[
				Property->CreatePropertyNameWidget()
			]
		]
	]
	.ValueContent()
	[
		SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda( [BoolControl] () { bool bTemp; BoolControl->GetValue(bTemp); return bTemp ? 0 : 1; } )
		+ SWidgetSwitcher::Slot()
		[
			NormalValueWidget
		]
		+ SWidgetSwitcher::Slot()
		[
			DefaultValueWidget
		]
	];
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::DrawExtraPanelContent(IDetailGroup& Group, TSharedPtr<IPropertyHandle> Property)
{
	if (Property->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, DialogueTagsParent))
	{
		//DrawDialogueTagsExtraControls(Group, Property);
		DrawTagExtraControls(Group, Property, LOCTEXT("DialogueTags", "Dialogue Tags"));
	}
}

// ------------------------------------------------------------------------------------------------
const FSlateBrush* FPropertyCustomization_YapGroupSettings::BorderImage() const
{
	return FAppStyle::Get().GetBrush("SCSEditor.Background");
}

// ------------------------------------------------------------------------------------------------
FText FPropertyCustomization_YapGroupSettings::GetChildTagsAsText(TSharedPtr<IPropertyHandle> ParentTagProperty) const
{
	FGameplayTag ParentTag = GetTagPropertyFromHandle(ParentTagProperty);
	
	if (!ParentTag.IsValid())
	{
		return LOCTEXT("None_Label", "<None>");
	}

	FName ParentTagPropertyName = ParentTagProperty->GetProperty()->GetFName();
	
	const bool* bDirty = CachedGameplayTagsPreviewTextsDirty.Find(ParentTagPropertyName);
	const FText* Text = CachedGameplayTagsPreviewTexts.Find(ParentTagPropertyName);
	
	if (!bDirty || !Text || (*bDirty))
	{
		return LOCTEXT("Error_Label", "<Error>");
	}

	return CachedGameplayTagsPreviewTexts[ParentTagPropertyName];
}

// ------------------------------------------------------------------------------------------------
bool FPropertyCustomization_YapGroupSettings::IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const
{
	TArray<void*> RawData;

	TagPropertyHandle->AccessRawData(RawData);

	const FGameplayTag* Tag = reinterpret_cast<const FGameplayTag*>(RawData[0]);

	return Tag->IsValid();
}

// ------------------------------------------------------------------------------------------------
FGameplayTag& FPropertyCustomization_YapGroupSettings::GetTagPropertyFromHandle(TSharedPtr<IPropertyHandle> TagPropertyHandle) const
{
	TArray<void*> RawData;

	TagPropertyHandle->AccessRawData(RawData);

	FGameplayTag* Tag = reinterpret_cast<FGameplayTag*>(RawData[0]);

	return *Tag;
}

// ------------------------------------------------------------------------------------------------
FReply FPropertyCustomization_YapGroupSettings::OnClicked_OpenTagsManager(FGameplayTag ParentTag, FText Title)
{
	FGameplayTagManagerWindowArgs Args;
	Args.Title = Title;
	Args.bRestrictedTags = false;
	Args.Filter = ParentTag.ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);

	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
FReply FPropertyCustomization_YapGroupSettings::OnClicked_CleanupDialogueTags()
{
	/*
	// If the dialogue tags parent is unset, don't do anything
	if (!GetTagPropertyFromHandle(MoodTagParentPropertyHandle).IsValid())
	{
		FText TitleText = LOCTEXT("MissingDialogueTagParentSetting_Title", "Missing Dialogue Tags Parent");
		FText DescriptionText = LOCTEXT("MissingDialogueTagParentSetting_Description", "No dialogue tags parent is set - cannot clean up tags, aborting!");		

		Yap::EditorFuncs::PostNotificationInfo_Warning(TitleText, DescriptionText);
		
		return FReply::Handled();
	}
	
	TSharedPtr<FGameplayTagNode> DialogueTagsNode = UGameplayTagsManager::Get().FindTagNode(GetTagPropertyFromHandle(MoodTagParentPropertyHandle));

	TArray<TSharedPtr<FGameplayTagNode>> ParentNodes = { DialogueTagsNode };

	TArray<FName> NodeNames; // I can't store raw leaf nodes because for some reason the gameplay tags manager invalidates them all after I remove one? 

	while (ParentNodes.Num() > 0)
	{
		TSharedPtr<FGameplayTagNode> Node = ParentNodes.Pop();

		TArray<TSharedPtr<FGameplayTagNode>> ChildNodes = Node->GetChildTagNodes();

		if (Node->IsExplicitTag() && !Node->GetAllSourceNames().Contains(FGameplayTagSource::GetNativeName()))
		{
			NodeNames.Add(Node->GetCompleteTagName());
		}
		
		if (ChildNodes.Num() > 0)
		{
			ParentNodes.Append(ChildNodes);
		}
	}

	TArray<FName> TagNamesToDelete; // Don't store raw FGameplayTagNodes because they all get destroyed every time the tag tree changes...
	TagNamesToDelete.Reserve(NodeNames.Num());
	
	for (auto It = NodeNames.CreateIterator(); It; ++It)
	{
		TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*It);

		if (!Node.IsValid() || Node->GetSimpleTagName() == NAME_None)
		{
			continue;
		}
		
		TArray<FAssetIdentifier> References = Yap::Tags::FindTagReferences(Node->GetCompleteTag().GetTagName());

		if (References.Num() == 0)
		{
			TagNamesToDelete.Add(Node->GetCompleteTagName());
		}
	}

	FText TitleText = LOCTEXT("DeleteGameplayTags_Title", "Delete Gameplay Tags");
	FText DescriptionText = FText::Format(LOCTEXT("DeleteGameplayTags_Description", "Would you like to delete the following tags?\n\n{0}\n\nNote: this operation is slow; if you have hundreds of obsolete tags, the editor may freeze for a minute."), GetDeletedTagsText(TagNamesToDelete));

	EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, DescriptionText, TitleText);

	if (Choice == EAppReturnType::Yes)
	{
		int32 EraseCount = 0;

		for (FName TagName : TagNamesToDelete)
		{
			TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(TagName);
			IGameplayTagsEditorModule::Get().DeleteTagFromINI(Node);
			EraseCount++;
		}

		UE_LOG(LogYap, Display, TEXT("Erased %d tag nodes"), EraseCount);
	}
	*/
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
FText FPropertyCustomization_YapGroupSettings::GetDeletedTagsText(const TArray<FName>& TagNamesToDelete)
{
	FString TagNameList;

	int32 Count = 0;

	FText AppendCountText = FText::GetEmpty();
	
	for (FName TagName : TagNamesToDelete)
	{
		TagNameList += TagName.ToString();

		if (Count < TagNamesToDelete.Num() - 1)
		{
			TagNameList += "\n";
		}
		
		++Count;
		
		if (Count >= 10)
		{
			AppendCountText = FText::Format(LOCTEXT("TagsToDelete_MoreCount", "... and {0} {0}|plural(one=more,other=more)"), FText::AsNumber(TagNamesToDelete.Num() - Count));
			break;
		}
	}

	FText TagNameListText = FText::FromString(TagNameList);// FText::Format(LOCTEXT("", "{0}"), FText::FromString(TagNameList));

	if (AppendCountText.IsEmpty())
	{
		return TagNameListText;
	}
	else
	{
		return FText::Format(INVTEXT("{0}{1}"), TagNameListText, AppendCountText);
	}
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::RecacheTagListText(TSharedPtr<IPropertyHandle> ParentTagProperty)
{
	const FGameplayTag& ParentTag = GetTagPropertyFromHandle(ParentTagProperty);
	
	FGameplayTagContainer Tags = UGameplayTagsManager::Get().RequestGameplayTagChildren(ParentTag);
	
	FString TagString;
	bool bFirst = true;
	int32 ParentTagLen = ParentTag.ToString().Len() + 1;
	
	FName ParentTagPropertyName = ParentTagProperty->GetProperty()->GetFName();
	
	for (const FGameplayTag& Tag : Tags)
	{
		if (bFirst)
		{			
			bFirst = false;
		}
		else
		{
			TagString += "\n";
		}
		
		TagString += Tag.ToString().RightChop(ParentTagLen);
	}

	if (TagString.IsEmpty())
	{
		TagString = "<None>";
	}

	CachedGameplayTagsPreviewTexts.Add(ParentTagPropertyName, FText::FromString(TagString));
	CachedGameplayTagsPreviewTextsDirty.Add(ParentTagPropertyName, false);
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::DrawTagExtraControls(IDetailGroup& Group, TSharedPtr<IPropertyHandle> ParentTagPropertyHandle, FText TagEditorTitle)
{
	float VerticalPadding = 3.0;

	// Make the widget able to hold all of the default tags
	float TagLineHeight = 15.0; // This is the height of a single tag name in pixels
	float LineHeightPercentage = 18.0 / TagLineHeight; // Desired row height divided by actual height
	float TotalHeight = FMath::RoundFromZero(/*FYapGroupSettings::GetDefaultMoodTags().Num()*/ 6 * TagLineHeight * LineHeightPercentage + VerticalPadding * 2.0);

	Group.AddWidgetRow()
	.Visibility(TAttribute<EVisibility>::CreateLambda([this, ParentTagPropertyHandle] ()
	{
		if (IsDefault())
		{
			return EVisibility::Visible;
		}

		if (IsOverridden(ParentTagPropertyHandle->GetProperty()->GetFName()))
		{
			return EVisibility::Visible;
		}

		return EVisibility::Collapsed;
	}))
	.NameContent()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Visibility(TAttribute<EVisibility>::CreateLambda([this, ParentTagPropertyHandle] ()
		{
			if (!IsTagPropertySet(ParentTagPropertyHandle) && (IsDefault() || IsOverridden(ParentTagPropertyHandle->GetProperty()->GetFName())))
			{
				return EVisibility::Visible;
			}

			return EVisibility::Collapsed;
		}))
		.Font(YapFonts.Font_WarningText)
		.Text(LOCTEXT("TagParentUnset_Warning", "Tags parent is unset!"))
		.ColorAndOpacity(YapColor::LightYellow)
	]
	.ValueContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				// OPEN MOOD TAGS MANAGER BUTTON
				SNew(SButton)
				.IsEnabled(this, &FPropertyCustomization_YapGroupSettings::IsTagPropertySet, ParentTagPropertyHandle)
				.OnClicked(this, &FPropertyCustomization_YapGroupSettings::OnClicked_OpenTagsManager, GetTagPropertyFromHandle(ParentTagPropertyHandle), TagEditorTitle)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("EditTags", "Edit tags"))
				.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Open tags manager"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				// RESET MOOD TAGS TO DEFAULTS BUTTON
				SNew(SButton)
				.IsEnabled(this, &FPropertyCustomization_YapGroupSettings::IsTagPropertySet, ParentTagPropertyHandle)
				//.OnClicked(this, &FPropertyCustomization_YapGroupSettings::OnClicked_ResetDefaultMoodTags)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ResetTags_Button", "Reset to defaults..."))
				//.ToolTipText(this, &FPropertyCustomization_YapGroupSettings::ToolTipText_DefaultMoodTags)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				// DELETE ALL MOOD TAGS BUTTON
				SNew(SButton)
				.IsEnabled(this, &FPropertyCustomization_YapGroupSettings::IsTagPropertySet, ParentTagPropertyHandle)
				//.OnClicked(this, &FPropertyCustomization_YapGroupSettings::OnClicked_DeleteAllMoodTags)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("DeleteTags_Button", "Delete all..."))
				.ToolTipText(LOCTEXT("DeleteTags_ToolTip", "Attempts to delete all tags"))
			]
		]
	];
}

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::DrawDialogueTagsExtraControls(IDetailGroup& Group, TSharedPtr<IPropertyHandle> DialogueTagsParentProperty)
{
	if (DialogueTagsParentProperty->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, DialogueTagsParent))
	{
		checkNoEntry();
	}
	
	float VerticalPadding = 3.0;
	
	Group.AddWidgetRow()
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("DialogueTags", "Dialogue Tags"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0, VerticalPadding)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(LOCTEXT("EditDialogueTags_ToolTip", "Opens the tag manager"))
			.Text(LOCTEXT("EditDialogueTags_Button", "Edit dialogue tags"))
			.OnClicked(this, &FPropertyCustomization_YapGroupSettings::OnClicked_OpenTagsManager, GetTagPropertyFromHandle(DialogueTagsParentProperty), LOCTEXT("DialogueTags", "Dialogue Tags"))
		]
		+ SVerticalBox::Slot()
		.Padding(0, VerticalPadding)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(LOCTEXT("CleanupUnusedDialogueTags_ToolTip", "Finds and deletes unused tags under the Dialogue Tags Parent - make sure all assets have been saved first!"))
			.Text(LOCTEXT("CleanupUnusedDialogueTags_Button", "Cleanup unused tags"))
			.OnClicked(this, &FPropertyCustomization_YapGroupSettings::OnClicked_CleanupDialogueTags)
		]
	];
}

#undef LOCTEXT_NAMESPACE
