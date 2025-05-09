// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapGroupSettings.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "ObjectEditorUtils.h"
#include "SGameplayTagPicker.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Yap/YapTypeGroupSettings.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TMap<FName, TSharedPtr<IPropertyHandle>> FPropertyCustomization_YapGroupSettings::DefaultPropertyHandles;

TArray<FName> FPropertyCustomization_YapGroupSettings::IgnoredProperties = 
{
	GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, bDefault),
	GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, GroupColor)
};

// ========

// Pleasantly borrowed from PostProcessSettingsCustomization.cpp
struct FYapCategoryOrGroup
{
	IDetailCategoryBuilder* Category;
	IDetailGroup* Group;

	FYapCategoryOrGroup(IDetailCategoryBuilder& NewCategory)
		: Category(&NewCategory)
		, Group(nullptr)
	{}

	FYapCategoryOrGroup(IDetailGroup& NewGroup)
		: Category(nullptr)
		, Group(&NewGroup)
	{}

	FYapCategoryOrGroup()
		: Category(nullptr)
		, Group(nullptr)
	{}

	void AddProperty(FPropertyCustomization_YapGroupSettings& Drawer, IDetailChildrenBuilder& StructBuilder, TSharedPtr<IPropertyHandle>& Property)
	{
		if (Category)
		{
			Category->AddProperty(Property);
		}
		else
		{
			Drawer.DrawProperty(StructBuilder, *Group, Property);
		}
	}

	IDetailGroup& AddGroup(FName GroupName, const FText& DisplayName)
	{
		if (Category)
		{
			return Category->AddGroup(GroupName, DisplayName);
		}
		else
		{
			return Group->AddGroup(GroupName, DisplayName);
		}
	}

	bool IsValid() const
	{
		return Group || Category;
	}
};

struct FYapGroupSettingsGroup
{
	FString RawGroupName;
	FString DisplayName;
	FYapCategoryOrGroup RootCategory;
	TArray<TSharedPtr<IPropertyHandle>> SimplePropertyHandles;
	TArray<TSharedPtr<IPropertyHandle>> AdvancedPropertyHandles;

	bool IsValid() const
	{
		return !RawGroupName.IsEmpty() && !DisplayName.IsEmpty() && RootCategory.IsValid();
	}

	FYapGroupSettingsGroup()
		: RootCategory()
	{}
};

// ========

// ------------------------------------------------------------------------------------------------
void FPropertyCustomization_YapGroupSettings::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UE_LOG(LogYapEditor, VeryVerbose, TEXT("========================================================================="));
	UE_LOG(LogYapEditor, VeryVerbose, TEXT("GROUP SETTINGS CUSTOMIZATION START"));

	IndexAllProperties(StructPropertyHandle);
	
	SortGroups();
	
	UpdateOverriddenCounts();

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
	
	///*
	uint32 NumChildren = 0;
	FPropertyAccess::Result Result = StructPropertyHandle->GetNumChildren(NumChildren);

	FProperty* Prop = StructPropertyHandle->GetProperty();
	FStructProperty* StructProp = CastField<FStructProperty>(Prop);

	TMap<FString, FYapCategoryOrGroup> NameToCategoryBuilderMap;
	TMap<FString, FYapGroupSettingsGroup> NameToGroupMap;

	if(Result == FPropertyAccess::Success && NumChildren > 0)
	{
		for( uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
		{
			TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle( ChildIndex );

			if( ChildHandle.IsValid() && ChildHandle->GetProperty() )
			{
				if (ChildHandle->HasMetaData("DoNotDraw"))
				{
					continue;
				}
				
				if (ChildHandle->HasMetaData("DefaultOverride"))
				{
					continue;
				}
				
				FProperty* Property = ChildHandle->GetProperty();

				FName CategoryFName = FObjectEditorUtils::GetCategoryFName(Property);
				
				FString RawCategoryName = CategoryFName.ToString();

				TArray<FString> CategoryAndGroups;
				RawCategoryName.ParseIntoArray(CategoryAndGroups, TEXT("|"), 1);

				FString RootCategoryName = CategoryAndGroups.Num() > 0 ? CategoryAndGroups[0] : RawCategoryName;

				FYapCategoryOrGroup* Category = NameToCategoryBuilderMap.Find(RootCategoryName);
				
				if(!Category)
				{
					IDetailGroup& NewGroup = StructBuilder.AddGroup(*RootCategoryName, FText::FromString(RootCategoryName));
					Category = &NameToCategoryBuilderMap.Emplace(RootCategoryName, NewGroup);
				}

				if(CategoryAndGroups.Num() > 1)
				{
					// Only handling one group for now
					// There are sub groups so add them now
					FYapGroupSettingsGroup& PPGroup = NameToGroupMap.FindOrAdd(RawCategoryName);
					
					// Is this a new group? It wont be valid if it is
					if(!PPGroup.IsValid())
					{
						PPGroup.RootCategory = *Category;
						PPGroup.RawGroupName = RawCategoryName;
						PPGroup.DisplayName = CategoryAndGroups[1].TrimStartAndEnd();
					}
	
					bool bIsSimple = !ChildHandle->GetProperty()->HasAnyPropertyFlags(CPF_AdvancedDisplay);
					if(bIsSimple)
					{
						PPGroup.SimplePropertyHandles.Add(ChildHandle);
					}
					else
					{
						PPGroup.AdvancedPropertyHandles.Add(ChildHandle);
					}
				}
				else
				{
					// This draws pretty much all of the properties
					Category->AddProperty(*this, StructBuilder, ChildHandle);
				}
			}
		}

		for(auto& NameAndGroup : NameToGroupMap)
		{
			FYapGroupSettingsGroup& YapGroup = NameAndGroup.Value;

			if(YapGroup.SimplePropertyHandles.Num() > 0 || YapGroup.AdvancedPropertyHandles.Num() > 0 )
			{
				// This is drawing a subcategory "like DialoguePlayback|Timed"
				IDetailGroup& SimpleGroup = YapGroup.RootCategory.AddGroup(*YapGroup.RawGroupName, FText::FromString(YapGroup.DisplayName));
				/*
				SimpleGroup.HeaderRow()
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(YapGroup.DisplayName))
						.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
					]
				];
				*/
				for(auto& SimpleProperty : YapGroup.SimplePropertyHandles)
				{
					//SimpleGroup.AddPropertyRow(SimpleProperty.ToSharedRef());
					DrawProperty(StructBuilder, SimpleGroup, SimpleProperty);
				}

				if(YapGroup.AdvancedPropertyHandles.Num() > 0)
				{
					IDetailGroup& AdvancedGroup = SimpleGroup.AddGroup(*(YapGroup.RawGroupName+TEXT("Advanced")), LOCTEXT("YapAdvancedGroup", "Advanced"));
					
					for(auto& AdvancedProperty : YapGroup.AdvancedPropertyHandles)
					{
						//AdvancedGroup.AddPropertyRow(AdvancedProperty.ToSharedRef());
						DrawProperty(StructBuilder, AdvancedGroup, AdvancedProperty);
					}
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------

void FPropertyCustomization_YapGroupSettings::IndexAllProperties(TSharedRef<class IPropertyHandle> StructPropertyHandle)
{
	IndexImportantProperties(StructPropertyHandle);
	
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	AllPropertyHandles.Empty(NumChildren);

	// Iterate through all of the properties and add them to the array. We're doing this so that we can always pull identical IPropertyHandles for the properties from one source
	// GetChildHandle always returns a unique handle object, so you can't grab the same property and find in a map (== isn't implemented, only IsSamePropertyNode() which is useless).
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex);

		AllPropertyHandles.Add(ChildHandle);

		// Discover default properties and discover override properties
		if (IsDefault())
		{
			if (!ChildHandle->HasMetaData("DefaultOverride"))
			{
				DefaultPropertyHandles.Add(ChildHandle->GetProperty()->GetFName(), ChildHandle);
			}
		}
		else
		{
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
}

// ------------------------------------------------------------------------------------------------

void FPropertyCustomization_YapGroupSettings::IndexImportantProperties(TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex);

		static const FName GroupColorName = GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, GroupColor);
		static const FName DefaultName = GET_MEMBER_NAME_CHECKED(FYapTypeGroupSettings, bDefault);

		FName PropertyName = ChildHandle->GetProperty()->GetFName();

		// Discover important properties
		if (PropertyName == GroupColorName)
		{
			GroupColorPropertyHandle = ChildHandle;
		}
		else if (PropertyName == DefaultName)
		{
			DefaultPropertyHandle = ChildHandle;
		}
	}
}

// ------------------------------------------------------------------------------------------------

void FPropertyCustomization_YapGroupSettings::UpdateOverriddenCounts()
{
	GroupOverridenCounts.Empty();
	TotalOverrides = 0;
	
	for (int32 ChildIndex = 0; ChildIndex < AllPropertyHandles.Num(); ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = AllPropertyHandles[ChildIndex];

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
bool FPropertyCustomization_YapGroupSettings::IsDefault() const
{
	bool bIsDefault;
	DefaultPropertyHandle->GetValue(bIsDefault);
	return bIsDefault;
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
	void* Data;
	GroupColorPropertyHandle->GetValueData(Data);
	return *reinterpret_cast<FLinearColor*>(Data);
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

	GroupPropertyArrays.KeySort([&Categories] (const FString& A, const FString& B)
	{
		int32 IndexA = Categories.Find(A);
		int32 IndexB = Categories.Find(B);

		return IndexA < IndexB;
	});
}

void FPropertyCustomization_YapGroupSettings::DrawProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{	
	UE_LOG(LogYapEditor, VeryVerbose, TEXT("     DrawProperty [%s]"), *Property->GetPropertyDisplayName().ToString());

	if (IsDefault())
	{
		return DrawDefaultProperty(StructBuilder, Group, Property);
	}
	else
	{
		DrawNamedGroupProperty(StructBuilder, Group, Property);
	}

	DrawExtraPanelContent(Group, Property);
}

void FPropertyCustomization_YapGroupSettings::DrawDefaultProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{
	IDetailPropertyRow& Row = Group.AddPropertyRow(Property.ToSharedRef());

	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> ValueWidget;
	FDetailWidgetRow WidgetRow;

	Row.GetDefaultWidgets(NameWidget, ValueWidget, WidgetRow);
	
	Row.CustomWidget(true)
	.NameContent()
	[
		
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, -3, 6, -4)
		[
			SNew(SBox)
			.WidthOverride(3)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
				.ColorAndOpacity(GetGroupColor())
			]
		]
		+ SHorizontalBox::Slot()
		[
			NameWidget.ToSharedRef()
		]
		/*
		SNew(SBorder)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.None))
		.ForegroundColor_Lambda( [this] () { return GetGroupColor(); } )
		[
			NameWidget.ToSharedRef()
		]
		*/

	]
	.ValueContent()
	[
		ValueWidget.ToSharedRef()
	];
}

void FPropertyCustomization_YapGroupSettings::DrawNamedGroupProperty(IDetailChildrenBuilder& StructBuilder, IDetailGroup& Group, TSharedPtr<IPropertyHandle>& Property)
{
	FName PropertyName = Property->GetProperty()->GetFName();

	uint32 ChildrenCount;
	Property->GetNumChildren(ChildrenCount);
	
	UE_LOG(LogYapEditor, VeryVerbose, TEXT("DrawNamedGroupProperty: %s - children: %i"), *PropertyName.ToString(), ChildrenCount);
		
	TSharedPtr<IPropertyHandle>* BoolControlPtr = PropertyBoolControlHandles.Find(Property->GetProperty()->GetFName());
	TSharedPtr<IPropertyHandle>* DefaultValuePtr = DefaultPropertyHandles.Find(Property->GetProperty()->GetFName());

	check(BoolControlPtr);
	check(DefaultValuePtr);
	
	TSharedPtr<IPropertyHandle> OverrideSetting = *BoolControlPtr;
	TSharedPtr<IPropertyHandle> DefaultValue = *DefaultValuePtr;
	
	IDetailPropertyRow& Row = Group.AddPropertyRow(Property.ToSharedRef());

	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> OverrideValueWidget;
	FDetailWidgetRow WidgetRow;
	Row.GetDefaultWidgets(NameWidget, OverrideValueWidget, WidgetRow);

	Row.IsEnabled(OverrideSetting);

	Row.ShowPropertyButtons(false);
	
	Row.CustomWidget(true)
	.Visibility(TAttribute<EVisibility>::CreateLambda( [OverrideSetting] ()
	{
		bool b;
		OverrideSetting->GetValue(b);
		return b ? EVisibility::Visible : EVisibility::Collapsed;
	}))
	.NameContent()
	[
		SNew(SCheckBox)
		.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_TypeSettingsOverride)
		.Padding(FMargin(4, 0, 0, 0))
		.IsChecked_Lambda( [this, OverrideSetting] ()
		{
			bool bTemp;
			OverrideSetting->GetValue(bTemp);
			return bTemp ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda( [this, OverrideSetting] (ECheckBoxState NewState)
		{
			OverrideSetting->SetValue(NewState == ECheckBoxState::Checked);
			UpdateOverriddenCounts();
		})
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, -3, 6, -4)
			[
				SNew(SBox)
				.WidthOverride(3)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
					.ColorAndOpacity(GetGroupColor())
				]
			]
			+ SHorizontalBox::Slot()
			[
				NameWidget.ToSharedRef()
			]
		]
	]
	.ValueContent()
	[
		OverrideValueWidget.ToSharedRef()
	];

	// TODO | this is the shittiest hack ever. I can't control the custom row well enough to display both the default value and the override depending
	// TODO | on the override state... so, draw the property twice, and hide one or the other.

	IDetailPropertyRow& DefaultRow = Group.AddPropertyRow(DefaultValue.ToSharedRef());

	TSharedPtr<SWidget> DefaultNameWidget;
	TSharedPtr<SWidget> DefaultValueWidget;
	FDetailWidgetRow DefaultWidgetRow;
	DefaultRow.GetDefaultWidgets(DefaultNameWidget, DefaultValueWidget, DefaultWidgetRow);

	DefaultRow.ShowPropertyButtons(false);

	DefaultValueWidget->SetEnabled(false);
	
	DefaultRow.CustomWidget(false)
	.Visibility(TAttribute<EVisibility>::CreateLambda( [OverrideSetting] ()
	{
		bool b;
		OverrideSetting->GetValue(b);
		return !b ? EVisibility::Visible : EVisibility::Collapsed;
	}))
	.NameContent()
	[
		SNew(SCheckBox)
		.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_TypeSettingsOverride)
		.Padding(FMargin(4, 0, 0, 0))
		.IsChecked_Lambda( [this, OverrideSetting] ()
		{
			bool b;
			OverrideSetting->GetValue(b);
			return b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda( [this, OverrideSetting] (ECheckBoxState NewState)
		{
			OverrideSetting->SetValue(NewState == ECheckBoxState::Checked);
			UpdateOverriddenCounts();
		})
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, -3, 6, -4)
			[
				SNew(SBox)
				.WidthOverride(3)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
					.ColorAndOpacity(GetGroupColor().Desaturate(0.25f) * YapColor::LightGray)
				]
			]
			+ SHorizontalBox::Slot()
			[
				NameWidget.ToSharedRef()
			]
		]
	]
	.ValueContent()
	[
		DefaultValueWidget.ToSharedRef()
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
