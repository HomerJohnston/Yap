// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacterAsset.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Yap/YapProjectSettings.h"

#include "Engine/Texture2D.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/YapNodeBlueprint.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "Yap/Globals/YapMoodTags.h"

#define LOCTEXT_NAMESPACE "Yap"

UYapCharacterAsset::UYapCharacterAsset() :
	EntityColor(0.5, 0.5, 0.5, 1.0)
{
}

const UTexture2D* UYapCharacterAsset::GetCharacterPortrait(const FGameplayTag& MoodTag) const
{
	if (MoodTag.IsValid())
	{
		const FYapPortraitList* PortraitList = PortraitsMap.Find(MoodTag.RequestDirectParent());

		if (PortraitList)
		{
			const TObjectPtr<UTexture2D>* TexturePtr = PortraitList->Map.Find(MoodTag.GetTagName());
			
			if (TexturePtr)
			{
				return *TexturePtr;
			}
		}
	
	}
	
	return Portrait;
}

#if WITH_EDITOR
void UYapCharacterAsset::PostLoad()
{
	Super::PostLoad();
}

void UYapCharacterAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	UObject::PreSave(SaveContext);
}
#endif

#if WITH_EDITOR
void UYapCharacterAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITOR
void UYapCharacterAsset::RefreshPortraitList()
{
	FGameplayTagContainer MoodRoots = Yap::GetMoodTagRoots();

	for (auto It = PortraitsMap.CreateIterator(); It; ++It)
	{
		TPair<FGameplayTag, FYapPortraitList>& KVPair = *It;

		if (!KVPair.Key.IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		if (!MoodRoots.HasTagExact(KVPair.Key))
		{
			It.RemoveCurrent();
			continue;
		}
	}
	
	for (const FGameplayTag& MoodRoot : MoodRoots)
	{
		FYapPortraitList& List = PortraitsMap.FindOrAdd(MoodRoot);

		FGameplayTagContainer Moods = UGameplayTagsManager::Get().RequestGameplayTagChildren(MoodRoot);

		for (const FGameplayTag& MoodTag : Moods)
		{
			if (!List.Map.Contains(MoodTag.GetTagName()))
			{
				List.Map.Add(MoodTag.GetTagName(), nullptr);
			}
		}
	}

	PortraitsMap.KeySort( [] (const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.GetTagName().LexicalLess(B.GetTagName());
	});
	

	/*
	FGameplayTagContainer AllMoodTags = GetAllMoodTags();

	for (auto It = Portraits.CreateIterator(); It; ++It)
	{
		TPair<FGameplayTag, UTexture2D*> KVPair = *It;

		if (!KVPair.Key.IsValid())
		{
			It.RemoveCurrent();
		}
	}
	
	for (const FGameplayTag& MoodTag : AllMoodTags)
	{
		if (!Portraits.Contains(MoodTag))
		{
			Portraits.Add(MoodTag, nullptr);
		}
	}
*/
	/*
	Portraits.KeySort( [] (const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});
	*/
}

bool UYapCharacterAsset::HasAnyMoodTags() const
{
	for (const auto& KVP : PortraitsMap)
	{
		if (KVP.Key.IsValid())
		{
			return true;
		}
	}

	return false;
}

bool UYapCharacterAsset::GetHasWarnings() const
{
	return GetHasObsoletePortraitData(); // || other conditions
}

bool UYapCharacterAsset::GetHasObsoletePortraitData() const
{
	return Portraits.Num() > 0;
}

bool UYapCharacterAsset::GetPortraitsOutOfDate() const
{
	FGameplayTagContainer MoodTagRoots = Yap::GetMoodTagRoots();

	// Check for different number of root mood tags
	if (PortraitsMap.Num() != MoodTagRoots.Num())
	{
		return true;
	}

	// Check if one of the root mood tags has a different number of children
	for (const FGameplayTag& Tag : MoodTagRoots)
	{
		if (!PortraitsMap.Contains(Tag))
		{
			return true;
		}
	}
	
	// Check if all of the children match for each root mood tag
	for (const FGameplayTag& RootTag : MoodTagRoots)
	{
		const FYapPortraitList& PortraitsList = PortraitsMap[RootTag];

		FGameplayTagContainer MoodTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(RootTag);

		// Counts do not match, early out
		if (PortraitsList.Map.Num() != MoodTags.Num())
		{
			return true;
		}

		// In-depth check for each tag
		for (const FGameplayTag& ChildTag : MoodTags)
		{
			if (!PortraitsList.Map.Contains(ChildTag.GetTagName()))
			{
				return true;
			}
		}
	}

	return false;
}

#endif

#undef LOCTEXT_NAMESPACE