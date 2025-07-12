// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapNodeConfigGroup_MoodTags.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "SGameplayTagPicker.h"
#include "Yap/YapNodeConfig.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FPropertyCustomization_YapNodeConfigGroup_MoodTags::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    HeaderRow.NameContent()
    [
        StructPropertyHandle->CreatePropertyNameWidget()
    ];

    HeaderRow.ValueContent()
    [
        StructPropertyHandle->CreatePropertyValueWidget()
    ];
}

void FPropertyCustomization_YapNodeConfigGroup_MoodTags::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (StructPropertyHandle->IsValidHandle())
	{
		uint32 NumChildren = 0;
		StructPropertyHandle->GetNumChildren(NumChildren);

		for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
		{
			StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef());
		}
	}

	float VerticalPadding = 3.0;

	static const FName MoodTagRoot = GET_MEMBER_NAME_CHECKED(FYapNodeConfigGroup_MoodTags, MoodTagsRoot);
	
	TSharedPtr<IPropertyHandle> MoodTagsRootProperty = StructPropertyHandle->GetChildHandle(MoodTagRoot);
	
	StructBuilder.AddCustomRow(INVTEXT("TODO"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, VerticalPadding)
		[
			SNew(SButton)
			.IsEnabled(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::IsTagPropertySet, MoodTagsRootProperty)
			.OnClicked(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_OpenMoodTagsManager, MoodTagsRootProperty)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(LOCTEXT("EditMoodTags", "Edit mood tags"))
			.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Open tags manager"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, VerticalPadding)
		[
			SNew(SButton)
			.IsEnabled(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::IsTagPropertySet, MoodTagsRootProperty)
			.OnClicked(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_ResetDefaultMoodTags, MoodTagsRootProperty)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(LOCTEXT("ResetMoodTags_Button", "Reset to defaults..."))
			.ToolTipText(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::ToolTipText_DefaultMoodTags)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, VerticalPadding)
		[
			SNew(SButton)
			.IsEnabled(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::IsTagPropertySet, MoodTagsRootProperty)
			.OnClicked(this, &FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_DeleteAllMoodTags, MoodTagsRootProperty)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(LOCTEXT("DeleteMoodTags_Button", "Delete all..."))
			.ToolTipText(LOCTEXT("DeleteMoodTags_ToolTip", "Attempts to delete all tags"))
		]
	];
	
	
	/*
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
				/ *
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
				* /
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
	*/
}

bool FPropertyCustomization_YapNodeConfigGroup_MoodTags::IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const
{
	TArray<void*> RawData;

	TagPropertyHandle->AccessRawData(RawData);

	const FGameplayTag* Tag = reinterpret_cast<const FGameplayTag*>(RawData[0]);

	return Tag->IsValid();
}

FReply FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_ResetDefaultMoodTags(TSharedPtr<IPropertyHandle> MoodTagsRootProperty) const
{
	/*
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapNodeConfig* ProjectSettings = GetMutableDefault<UYapNodeConfig>();
	
	FYapTransactions::BeginModify(LOCTEXT("ResetMoodTags", "Reset mood tags"), ProjectSettings);

	FString DefaultMoodTagsParentString = "Yap.Mood";
	
	// Remove any tags that should not be present
	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())
	{
		bool bKeepTag = true;

		if (!ExistingTag.IsValid())
		{
			bKeepTag = false;
		}
		else
		{
			FString ExistingTagString = ExistingTag.ToString();

			if (!DefaultMoodTags.ContainsByPredicate(
				[&DefaultMoodTagsParentString, &ExistingTagString]
				(const FString& DefaultTagString)
				{
					FString FullString = DefaultMoodTagsParentString + "." + DefaultTagString;
					return FullString == ExistingTagString;
				}))
			{
				bKeepTag = false;
			}
		}
		
		if (!bKeepTag)
		{
			TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
			IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
		}
	}
	
	// Make sure all of the default tags exist
	for (const FString& DefaultTagString : DefaultMoodTags)
	{
		FString DefaultTagFullString = DefaultMoodTagsParentString + "." + DefaultTagString;

		FGameplayTag ExistingTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(DefaultTagFullString), false);

		if (!ExistingTag.IsValid())
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(DefaultTagFullString, "", Yap::FileUtilities::GetTagConfigFileName());
		}
	}

	FYapTransactions::EndModify();

	*/
	
	return FReply::Handled();
}

FReply FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_DeleteAllMoodTags(TSharedPtr<IPropertyHandle> MoodTagsRootProperty) const
{
	/*
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapNodeConfig* ProjectSettings = GetMutableDefault<UYapNodeConfig>();

	FYapTransactions::BeginModify(LOCTEXT("DeleteMoodTags", "Delete mood tags"), ProjectSettings);

	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())
	{
		TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
		IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
	}
	
	FYapTransactions::EndModify();
	*/
	
	return FReply::Handled();
}

FReply FPropertyCustomization_YapNodeConfigGroup_MoodTags::OnClicked_OpenMoodTagsManager(TSharedPtr<IPropertyHandle> MoodTagsRootProperty)
{
	void* Data;
	MoodTagsRootProperty->GetValueData(Data);

	FGameplayTag& MoodTagsRoot = *static_cast<FGameplayTag*>(Data);

	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("MoodTags", "Mood Tags");
	Args.bRestrictedTags = false;
	Args.Filter = MoodTagsRoot.ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
	
	return FReply::Handled();
}

FText FPropertyCustomization_YapNodeConfigGroup_MoodTags::ToolTipText_DefaultMoodTags() const
{
	return INVTEXT("TODO");
	/*
	const FGameplayTag& ParentTag = UYapProjectSettings::GetMoodTagsParent();

	if (!ParentTag.IsValid())
	{
		return LOCTEXT("MissingParentTag_Warning", "Parent tag needs to be set!");
	}

	FString ParentTagString = ParentTag.ToString();

	FString DefaultTagsAsString;

	for (int32 i = 0; i < DefaultMoodTags.Num(); ++i)
	{
		const FString& Tag = DefaultMoodTags[i];
				
		DefaultTagsAsString = DefaultTagsAsString + ParentTagString + "." + Tag;

		if (i < DefaultMoodTags.Num() - 1)
		{
			DefaultTagsAsString += "\n";
		}
	}

	return FText::Format(LOCTEXT("SetDefaultTags_ToolTip", "Sets the following tags:\n{0}"), FText::FromString(DefaultTagsAsString));
	*/
}


#undef LOCTEXT_NAMESPACE
