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

// ------------------------------------------------------------------------------------------------

UYapCharacterAsset::UYapCharacterAsset() :
	EntityColor(0.5, 0.5, 0.5, 1.0)
{
}

// ------------------------------------------------------------------------------------------------

const UTexture2D* UYapCharacterAsset::GetCharacterPortrait(const FGameplayTag& MoodTag) const
{
	if (MoodTag.IsValid())
	{
		const FYapPortraitList* PortraitList = PortraitsMap.Find(MoodTag.RequestDirectParent().GetTagName());

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

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
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
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapCharacterAsset::GetHasWarnings() const
{
	return GetHasObsoletePortraitData(); // || other future conditions?
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapCharacterAsset::GetHasObsoletePortraitData() const
{
	return Portraits.Num() > 0;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
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
		if (!PortraitsMap.Contains(Tag.GetTagName()))
		{
			return true;
		}
	}
	
	// Check if all of the children match for each root mood tag
	for (const FGameplayTag& RootTag : MoodTagRoots)
	{
		const FYapPortraitList& PortraitsList = PortraitsMap[RootTag.GetTagName()];

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

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE