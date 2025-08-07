// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Globals/YapTagHelpers.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Yap/YapLog.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/Globals/YapEditorFuncs.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------

FString Yap::Tags::GetFilteredSubTag(const FString& Filter, const FGameplayTag& PropertyTag)
{
	return PropertyTag.ToString().RightChop(Filter.Len() + 1); // +1 for the '.' // TODO check if last char is actually a . or not?
}

// ------------------------------------------------------------------------------------------------

FString Yap::Tags::GetFilteredSubTag(const FGameplayTag& ParentContainer, const FGameplayTag& PropertyTag)
{
	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && PropertyTag.MatchesTag(ParentContainer))
	{
		return GetFilteredSubTag(ParentContainer.ToString(), PropertyTag);
	}

	return PropertyTag.ToString();
}

// ------------------------------------------------------------------------------------------------

FName Yap::Tags::GetLeafOfTag(const FGameplayTag& Tag)
{
	UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
	
	TSharedPtr<FGameplayTagNode> TagNode = GameplayTagsManager.FindTagNode(Tag);

	if (TagNode.IsValid())
	{
		return TagNode->GetSimpleTagName();
	}

	return NAME_None;
}

FGameplayTag Yap::Tags::GetTag(FString TagString)
{
	return UGameplayTagsManager::Get().RequestGameplayTag(FName(TagString), false);
}

// ------------------------------------------------------------------------------------------------

const FGameplayTag& Yap::Tags::GetOrAddTag(FString NewTagString, FString Comment)
{
	FGameplayTag NewTagSource = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);

	if (!NewTagSource.IsValid())
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(NewTagString, Comment, Yap::FileUtilities::GetTagConfigFileName());
	}

	TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);

	const FGameplayTag& Tag = Node->GetCompleteTag();

	UYapEditorSubsystem::RemoveTagPendingDeletion(Tag);

	return Tag;
}

void Yap::Tags::ParseChangedTag(FString OldTagString, FString NewTagString)
{
	
}

// ------------------------------------------------------------------------------------------------

void Yap::Tags::DeleteTags(TArray<FGameplayTag> TagsToErase, bool bShowPrompt, TWeakObjectPtr<UObject> Asset)
{
	if (!GEditor || !IsValid(GEditor))
	{
		return;
	}

	EAppReturnType::Type Choice;

	// Sanitize the input (remove None tags)
	for (auto It = TagsToErase.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrentSwap();
		}
	}
	
	if (bShowPrompt && TagsToErase.Num() > 1)
	{
		FText PromptDescription = FText::Format(LOCTEXT("DeleteObsoleteTags_Prompt", "Do you want to try to delete {0} gameplay {0}|plural(one=tag,other=tags)?"), TagsToErase.Num());
		FText PromptTitle = LOCTEXT("RedirectTags_Title", "Delete Gameplay Tags");
		
		Choice = FMessageDialog::Open(EAppMsgType::YesNoYesAll, PromptDescription);

		if (Choice == EAppReturnType::No)
		{
			return;
		}
	}
	
	FTimerDelegate DeleteTagsLambda = FTimerDelegate::CreateLambda( [TagsToErase, Choice, bShowPrompt] ()
	{
		if (!GEditor || !IsValid(GEditor))
		{
			return;
		}
				
		for (FGameplayTag Tag : TagsToErase)
		{
			if (bShowPrompt && Choice != EAppReturnType::YesAll)
			{
				FText EachTagChoiceText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "{0}\n\nThis tag isn't referenced anywhere anymore. Would you like to delete it?"), FText::FromName(Tag.GetTagName()));
				FText EachTagChoiceTitle = LOCTEXT("DeleteTagPrompt_Title", "Delete Old Gameplay Tag?");

				EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNo, EachTagChoiceText, EachTagChoiceTitle);

				if (RequestRedirectResponse == EAppReturnType::No)
				{
					continue;
				}
				else if (RequestRedirectResponse == EAppReturnType::Cancel)
				{
					break;
				}
			}
			
			TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(Tag);

			if (!ExistingTagNode.IsValid())
			{
				continue;
			}

			FName TagName = ExistingTagNode->GetCompleteTag().GetTagName();

			TArray<FAssetIdentifier> Refs = Yap::Tags::FindTagReferences(TagName);
			
			if (Refs.IsEmpty())
			{
				IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
			}
			else
			{
				if (bShowPrompt)
				{
					UE_LOG(LogYap, Warning, TEXT("Did not delete tag %s as it is still in use in \n%s and maybe others\n(%d total found assets referencing it)"), *Tag.ToString(), *Refs[0].PackageName.ToString(), Refs.Num());
				}
			}
		}
	});
	
	FTimerDelegate SaveLambda = FTimerDelegate::CreateLambda( [Asset, DeleteTagsLambda] ()
	{			
		if (!GEditor || !IsValid(GEditor))
		{
			return;
		}
				
		if (Asset.IsValid())
		{
			Yap::EditorFuncs::SaveAsset(Asset.Get());
		}

		GEditor->GetTimerManager()->SetTimerForNextTick(DeleteTagsLambda);
	});

	// I could just skip to calling the delete lambda... but for whatever reason if I don't wait at least two ticks, this doesn't work 
	GEditor->GetTimerManager()->SetTimerForNextTick(SaveLambda);
}

// ------------------------------------------------------------------------------------------------

void Yap::Tags::RedirectTags(TArray<TPair<FGameplayTag, FGameplayTag>> Redirects, bool bShowPrompt, TWeakObjectPtr<UObject> Asset)
{
	TArray<TPair<FString, FString>> RedirectStrings;

	for (auto Redirect : Redirects)
	{
		RedirectStrings.Add( {Redirect.Key.ToString(), Redirect.Value.ToString() } );
	}
	
	RedirectTags(RedirectStrings, bShowPrompt, Asset);
}

// ------------------------------------------------------------------------------------------------

void Yap::Tags::RedirectTags(TArray<TPair<FString, FString>> Redirects, bool bShowPrompt, TWeakObjectPtr<UObject> Asset)
{
	if (!GEditor || !IsValid(GEditor))
	{
		return;
	}

	EAppReturnType::Type Choice = EAppReturnType::YesAll;
	
	if (bShowPrompt && Redirects.Num() > 1)
	{
		FText PromptText = FText::Format(LOCTEXT("RedirectTags_Prompt", "Do you want to redirect {0} gameplay {0}|plural(one=tag,other=tags)?"), Redirects.Num());
		FText PromptTitle = LOCTEXT("RedirectTags_Title", "Redirect Gameplay Tags");
		
		Choice = FMessageDialog::Open(EAppMsgType::YesNoYesAll, PromptText);

		if (Choice == EAppReturnType::No)
		{
			for (auto& [OldTagString, NewTagString] : Redirects)
			{
				if (!OldTagString.IsEmpty())
				{
					FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(OldTagString));

					if (Tag.IsValid())
					{
						UYapEditorSubsystem::AddTagPendingDeletion(Tag);
					}
				}
			}
			return;
		}
	}
	
	FTimerDelegate RedirectTagsDelegate = FTimerDelegate::CreateLambda( [Redirects, Choice, bShowPrompt] ()
	{
		if (!GEditor || !IsValid(GEditor))
		{
			return;
		}

		IGameplayTagsEditorModule& Module = IGameplayTagsEditorModule::Get();
				
		for (TPair<FString, FString> Redirect : Redirects)
		{
			if ((bShowPrompt && Redirects.Num() == 1) || (Choice != EAppReturnType::YesAll))
			{
				const FText EmptyTagText = LOCTEXT("Tag_None", "<None>");

				const FString& OldTagString = Redirect.Key;
				const FString& NewTagString = Redirect.Value;
				
				FText OldTagText = OldTagString.IsEmpty() ? EmptyTagText : FText::FromString(OldTagString);
				FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

				FText EachTagChoiceText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "Would you like to add a tag redirect?\n\n(Warning: affected properties will not update until you restart Unreal!)"), OldTagText);
				FText EachTagChoiceTitle = LOCTEXT("RedirectTagPrompt_Title", "Add Gameplay Tag Redirect");

				EAppReturnType::Type TagChoice = FMessageDialog::Open(EAppMsgType::YesNoCancel, EachTagChoiceText, EachTagChoiceTitle);

				if (TagChoice == EAppReturnType::No)
				{
					if (!OldTagString.IsEmpty())
					{
						FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(OldTagString));

						if (Tag.IsValid())
						{
							UYapEditorSubsystem::AddTagPendingDeletion(Tag);
						}
					}
					continue;
				}
				else if (TagChoice == EAppReturnType::Cancel)
				{
					break;
				}
			}
			
			Module.RenameTagInINI(Redirect.Key, Redirect.Value);
		}
	});
	
	FTimerDelegate SaveDelegate = FTimerDelegate::CreateLambda( [Asset, RedirectTagsDelegate] ()
	{			
		if (!GEditor || !IsValid(GEditor))
		{
			return;
		}
				
		if (Asset.IsValid())
		{
			Yap::EditorFuncs::SaveAsset(Asset.Get());
		}

		GEditor->GetTimerManager()->SetTimerForNextTick(RedirectTagsDelegate);
	});

	if (Asset.IsValid())
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(SaveDelegate);
	}
	else
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(RedirectTagsDelegate);
	}
}

// ------------------------------------------------------------------------------------------------

TArray<FAssetIdentifier> Yap::Tags::FindTagReferences(FName TagName)
{
	// Verify references
	FAssetIdentifier TagId = FAssetIdentifier(FGameplayTag::StaticStruct(), TagName);
	TArray<FAssetIdentifier> Referencers;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().GetReferencers(TagId, Referencers, UE::AssetRegistry::EDependencyCategory::SearchableName);

	return Referencers;
}

#undef LOCTEXT_NAMESPACE
