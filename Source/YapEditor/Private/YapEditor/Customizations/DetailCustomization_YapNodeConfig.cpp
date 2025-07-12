// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapNodeConfig.h"

#include "DetailLayoutBuilder.h"


void FDetailCustomization_YapNodeConfig::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
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
		DetailBuilder.SortCategories(&CustomSortYapProjectSettingsCategories);
		
		IDetailCategoryBuilder& CharactersCategory = DetailBuilder.EditCategory("Characters");
		ProcessCategory(CharactersCategory);
	}
}

FReply FDetailCustomization_YapNodeConfig::OnClicked_ResetDefaultMoodTags() const
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

FReply FDetailCustomization_YapNodeConfig::OnClicked_DeleteAllMoodTags() const
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

FReply FDetailCustomization_YapNodeConfig::OnClicked_OpenMoodTagsManager()
{
	/*
	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("MoodTags", "Mood Tags");
	Args.bRestrictedTags = false;
	Args.Filter = UYapNodeConfig::GetMoodTagsParent().ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
	*/
	
	return FReply::Handled();
}
