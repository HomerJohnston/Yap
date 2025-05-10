// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacterAsset.h"

#include "Yap/YapProjectSettings.h"

#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "Yap"

UYapCharacterAsset::UYapCharacterAsset() :
	EntityColor(0.5, 0.5, 0.5, 1.0)
{
}

const UTexture2D* UYapCharacterAsset::GetYapCharacterPortrait(const FGameplayTag& MoodTag) const
{
	if (bUseSinglePortrait)
	{
		return Portrait;
	}
	
	FGameplayTag MoodTagToFind = MoodTag.IsValid() ? MoodTag : UYapProjectSettings::GetDefaultMoodTag();

	const TObjectPtr<UTexture2D>* TexturePtr = Portraits.Find(MoodTagToFind.GetTagName());

	return TexturePtr ? *TexturePtr : nullptr;
}

#if WITH_EDITOR
void UYapCharacterAsset::PostLoad()
{
	Super::PostLoad();
}
#endif

#if WITH_EDITOR
void UYapCharacterAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITOR
// TODO I need validation code to check if the character's portrait keys array matches the project or not to throw warnings during packaging?
void UYapCharacterAsset::RefreshPortraitList()
{
	FGameplayTagContainer MoodTags = UYapProjectSettings::GetMoodTags();

	TSet<FName> MoodTagsAsNames;
	TSet<FName> CharacterMoodTagsAsNames;

	for (const FGameplayTag& Tag : MoodTags)
	{
		MoodTagsAsNames.Add(Tag.GetTagName());
	}
	
	// Iterate through all existing keys. Remove any which are no longer in use.
	for (auto It = Portraits.CreateIterator(); It; ++It)
	{
		FName MoodTag = It.Key();
		
		if (!MoodTagsAsNames.Contains(MoodTag))
		{
			It.RemoveCurrent();
		}
	}

	// Iterate through all project keys. Add any which are missing.
	for (FName MoodTag : MoodTagsAsNames)
	{
		if (!Portraits.Contains(MoodTag))
		{
			Portraits.Add(MoodTag, nullptr);
		}
	}

	// Sort the map for better display.
	Portraits.KeySort(FNameLexicalLess());
}
#endif

#undef LOCTEXT_NAMESPACE