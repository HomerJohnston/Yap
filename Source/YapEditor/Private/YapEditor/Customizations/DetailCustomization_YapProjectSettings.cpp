// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapProjectSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsEditorModule.h"
#include "SGameplayTagPicker.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Globals/YapTagHelpers.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FText FDetailCustomization_YapProjectSettings::GetMoodTags() const
{
	return INVTEXT("TODO");

	/*
	const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
	
	const FGameplayTag& ParentTag = ProjectSettings->GetMoodTagsParent();

	if (!ParentTag.IsValid())
	{
		return LOCTEXT("None_Label", "<None>");
	}
	
	FGameplayTagContainer MoodTags = ProjectSettings->GetMoodTags();
	
	FString TagString;
	bool bFirst = true;
	int32 ParentTagLen = ParentTag.ToString().Len() + 1;
	
	for (const FGameplayTag& Tag : MoodTags)
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
	
	return FText::FromString(TagString);
	*/
}

const FSlateBrush* FDetailCustomization_YapProjectSettings::TODOBorderImage() const
{
	return FAppStyle::Get().GetBrush("SCSEditor.Background");
	//return FAppStyle::GetBrush("Menu.Background");
}

void SortCategory(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap, int32& Order, TSet<FName>& SortedCategories, FName NextCategory)
{
	IDetailCategoryBuilder* Builder = (*AllCategoryMap.Find(FName("Core")));
	
	if (!Builder)
	{
		UE_LOG(LogYapEditor, Error, TEXT("Could not find project settings category %s"), *NextCategory.ToString());
		return;
	}

	SortedCategories.Add(NextCategory);
	
	Builder->SetSortOrder(Order++);
}

void CustomSortYapProjectSettingsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap )
{
	int i = 0;

	TSet<FName> SortedCategories;

	SortCategory(AllCategoryMap, i, SortedCategories, "Core");
	SortCategory(AllCategoryMap, i, SortedCategories, "Mood Tags");
	SortCategory(AllCategoryMap, i, SortedCategories, "Dialogue Tags");
	SortCategory(AllCategoryMap, i, SortedCategories, "Dialogue Playback");
	SortCategory(AllCategoryMap, i, SortedCategories, "Editor");
	SortCategory(AllCategoryMap, i, SortedCategories, "Flow Graph Settings");
	SortCategory(AllCategoryMap, i, SortedCategories, "Error Handling");
	SortCategory(AllCategoryMap, i, SortedCategories, "Other");

	if (SortedCategories.Num() != AllCategoryMap.Num())
	{
		UE_LOG(LogYapEditor, Error, TEXT("Not all categories were sorted!"));
	}
}

void FDetailCustomization_YapProjectSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	DetailBuilder.SortCategories(&CustomSortYapProjectSettingsCategories);
	
	IDetailCategoryBuilder& MoodTagsCategory = DetailBuilder.EditCategory("Mood Tags");
	IDetailCategoryBuilder& DialogueTagsCategory = DetailBuilder.EditCategory("Dialogue Tags");
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_ResetDefaultMoodTags() const
{
	/*
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();
	
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

FReply FDetailCustomization_YapProjectSettings::OnClicked_DeleteAllMoodTags() const
{
	/*
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();

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

FReply FDetailCustomization_YapProjectSettings::OnClicked_OpenMoodTagsManager()
{
	/*
	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("MoodTags", "Mood Tags");
	Args.bRestrictedTags = false;
	Args.Filter = UYapProjectSettings::GetMoodTagsParent().ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
	*/
	
	return FReply::Handled();
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
