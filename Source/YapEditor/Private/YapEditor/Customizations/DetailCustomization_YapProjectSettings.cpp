// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapProjectSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "YapEditor"

const FSlateBrush* FDetailCustomization_YapProjectSettings::TODOBorderImage() const
{
	return FAppStyle::Get().GetBrush("SCSEditor.Background");
	//return FAppStyle::GetBrush("Menu.Background");
}

void FDetailCustomization_YapProjectSettings::SortCategory(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap, int32& Order, TSet<FName>& SortedCategories, FName NextCategory)
{
	SortedCategories.Add(NextCategory);

	(*AllCategoryMap.Find(NextCategory))->SetSortOrder(Order);
}

void FDetailCustomization_YapProjectSettings::CustomSortYapProjectSettingsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
{
	int i = 0;

	TSet<FName> SortedCategories;

	SortCategory(AllCategoryMap, i, SortedCategories, "Core");
	SortCategory(AllCategoryMap, i, SortedCategories, "Editor");
	SortCategory(AllCategoryMap, i, SortedCategories, "Error Handling");
	SortCategory(AllCategoryMap, i, SortedCategories, "Characters");
	
	if (SortedCategories.Num() != AllCategoryMap.Num())
	{
		UE_LOG(LogYapEditor, Error, TEXT("Not all categories were sorted!"));
	}
}

void FDetailCustomization_YapProjectSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CachedDetailBuilder = &DetailBuilder;
	
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	DetailFont = DetailBuilder.GetDetailFont();

	ProjectSettings = nullptr;
	
	for (TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (Object->IsA<UYapProjectSettings>())
		{
			ProjectSettings = Cast<UYapProjectSettings>(Object.Get());
			break;
		}
	}

	if (ProjectSettings.IsValid())
	{
		DetailBuilder.SortCategories(&FDetailCustomization_YapProjectSettings::CustomSortYapProjectSettingsCategories);
		
		IDetailCategoryBuilder& CharactersCategory = DetailBuilder.EditCategory("Characters");
		ProcessCategory(CharactersCategory, DetailBuilder);
	}
}

void FDetailCustomization_YapProjectSettings::ProcessCategory(IDetailCategoryBuilder& Category, IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TSharedRef<IPropertyHandle>> Properties;
	Category.GetDefaultProperties(Properties, true, true);
	
	for (TSharedPtr<IPropertyHandle> PropertyHandle : Properties)
	{
		Category.AddProperty(PropertyHandle);
	}

	Category.AddCustomRow(INVTEXT(""));
			
	Category.AddCustomRow(INVTEXT("Utilities"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CharacterManagementButtonsLabel", "Character Map Utilities:"))
		.Font(DetailFont)
	]
	.ValueContent()
	[
		SNew(SBox)
		.WidthOverride(200)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0, 8, 0, 8)
			.AutoHeight()
			[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(LOCTEXT("SortCharactersButton_Label", "Sort Alphabetically"))
					.ToolTipText(LOCTEXT("SortCharactersButton_ToolTip", "Sorts the list using the character tags."))
					.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_SortCharacters)	
			]
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 8)
			.AutoHeight()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("RemoveEmptyCharactersButton_Label", "Remove Empty"))
				.ToolTipText(LOCTEXT("RemoveEmptyCharactersButton_ToolTip", "Removes entries which have both the tag and the asset unset."))
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_DeleteEmptyCharacters)	
			]
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 8)
			.AutoHeight()
			[
				SNew(SButton)
				.IsEnabled_Lambda([this] () { return ProjectSettings.Get()->CharacterTagRoot.IsValid() ? true : false; })
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("PopulateCharactersFromParentButton_ToolTip", "Populate from Root Tag"))
				.ToolTipText(LOCTEXT("PopulateCharactersFromParentButton_ToolTip", "Finds all child (leaf) tags under the set character tag parent above and adds rows into the map for them."))
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_PopulateFromParent)	
			]
		]
	];
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_OpenDialogueTagsManager(const FGameplayTag& DomainTag)
{
	/*
	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("DialogueTags", "Dialogue Tags");
	Args.bRestrictedTags = false;
	Args.Filter = UYapProjectSettings::GetDomain(DomainTag).GetDialogueTagsParent().ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
	*/
	
	return FReply::Handled();
}

/*
FReply FDetailCustomization_YapProjectSettings::OnClicked_CleanupDialogueTags()
{
	// If the dialogue tags parent is unset, don't do anything
	if (!UYapProjectSettings::GetDialogueTagsParent().IsValid())
	{
		FText TitleText = LOCTEXT("MissingDialogueTagParentSetting_Title", "Missing Dialogue Tags Parent");
		FText DescriptionText = LOCTEXT("MissingDialogueTagParentSetting_Description", "No dialogue tags parent is set - cannot clean up tags, aborting!");		

		Yap::EditorFuncs::PostNotificationInfo_Warning(TitleText, DescriptionText);
		
		return FReply::Handled();
	}
	
	TSharedPtr<FGameplayTagNode> DialogueTagsNode = UGameplayTagsManager::Get().FindTagNode(UYapProjectSettings::GetDialogueTagsParent());

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
	
	return FReply::Handled();
}
*/

FReply FDetailCustomization_YapProjectSettings::OnClicked_RefreshMoodTagIcons()
{
	//UYapEditorSubsystem::Get()->UpdateMoodTagBrushes();

	return FReply::Handled();
}

FText FDetailCustomization_YapProjectSettings::ToolTipText_DefaultMoodTags() const
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

bool FDetailCustomization_YapProjectSettings::IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const
{
	TArray<void*> RawData;

	TagPropertyHandle->AccessRawData(RawData);

	const FGameplayTag* Tag = reinterpret_cast<const FGameplayTag*>(RawData[0]);

	return Tag->IsValid();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_SortCharacters() const
{
	if (!ProjectSettings.IsValid())
	{
		return FReply::Handled();
	}

	FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), ProjectSettings.Get());
	
	ProjectSettings->CharacterArray.StableSort();
	ProjectSettings->Modify();
	ProjectSettings->TryUpdateDefaultConfigFile();
	
	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_DeleteEmptyCharacters() const
{
	if (!ProjectSettings.IsValid())
	{
		return FReply::Handled();
	}

	auto RemoveEmpty = [] (const FYapCharacterDefinition& CharacterDefinition) -> bool
	{
		return CharacterDefinition.CharacterAsset.IsNull() && !CharacterDefinition.CharacterTag.IsValid();
	};
	
	FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), ProjectSettings.Get());
	
	ProjectSettings->CharacterArray.RemoveAll(RemoveEmpty);
	ProjectSettings->Modify();
	ProjectSettings->TryUpdateDefaultConfigFile();
	
	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_PopulateFromParent() const
{
	if (!ProjectSettings.IsValid())
	{
		return FReply::Handled();
	}

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
	FGameplayTagContainer CharacterTags = TagsManager.RequestGameplayTagChildren(ProjectSettings->CharacterTagRoot);

	TSet<FGameplayTag> Existing;
	
	for (const FYapCharacterDefinition& CharacterDefinition : ProjectSettings.Get()->CharacterArray)
	{
		Existing.Add(CharacterDefinition.CharacterTag);
	}
	
	for (const FGameplayTag& Tag : CharacterTags)
	{
		if (Existing.Contains(Tag))
		{
			continue;
		}
		
		ProjectSettings->CharacterArray.Emplace(FYapCharacterDefinition(Tag));
	}
	
	return FReply::Handled();
}

FText FDetailCustomization_YapProjectSettings::GetDeletedTagsText(const TArray<FName>& TagNamesToDelete)
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

	FText TagNameListText = FText::FromString(TagNameList);

	if (AppendCountText.IsEmpty())
	{
		return TagNameListText;
	}
	else
	{
		return FText::Format(INVTEXT("{0}{1}"), TagNameListText, AppendCountText);
	}
}

#undef LOCTEXT_NAMESPACE
