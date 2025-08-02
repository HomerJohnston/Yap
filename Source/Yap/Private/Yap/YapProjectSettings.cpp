// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#endif
#include "Engine/AssetManager.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "Yap/YapCharacterStaticDefinition.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
TMap<FYapCharacterStaticDefinition, FGameplayTag> UYapProjectSettings::ReversedCharacterMap;
#endif

#include "Sound/SoundBase.h"

UYapProjectSettings::UYapProjectSettings()
{
	DefaultAssetAudioClasses = { USoundBase::StaticClass() };

	DefaultCharacterClasses = { UYapCharacterAsset::StaticClass() };

	DefaultPortraitTexture = FSoftObjectPath("/Yap/T_Avatar_Missing.T_Avatar_Missing");

	AddGameplayTagFilter("CharacterArray.CharacterTag", FGameplayTagFilterDelegate::CreateStatic(&UYapProjectSettings::GetCharacterRootTag));
}

#if WITH_EDITOR
const TArray<const UClass*> UYapProjectSettings::GetAllowableCharacterClasses()
{
	TArray<const UClass*> Classes;
	
	// Classes.Reserve(Get().AdditionalCharacterClasses.Num() + Get().DefaultCharacterClasses.Num());

	for (auto& ClassSoftPtr : Get().DefaultCharacterClasses)
	{
		UClass* LoadedClass = ClassSoftPtr.LoadSynchronous();
		Classes.Add(LoadedClass);
	}

	return Classes;
}

/*
TArray<const UClass*> UYapProjectSettings::GetCharacterClasses_SyncLoad()
{
	TArray<const UClass*> LoadedClasses;

	TArray<TSoftClassPtr<UObject>> SourceClasses;

	if (Get().AdditionalCharacterClasses.Num() > 0)
	{
		SourceClasses.Reserve(Get().AdditionalCharacterClasses.Num() + 1);
		SourceClasses = Get().AdditionalCharacterClasses;
		SourceClasses.Append(Get().DefaultCharacterClasses);
	}

	for (TSoftClassPtr<UObject> ClassPtr : SourceClasses)
	{
		if (!ClassPtr.IsNull())
		{
			LoadedClasses.Add(ClassPtr.LoadSynchronous());
		}
	}

	return LoadedClasses;
}
*/
#endif

const TSubclassOf<UYapBroker> UYapProjectSettings::GetBrokerClass()
{
	const TSoftClassPtr<UYapBroker> BrokerClassSoftPtr = Get().BrokerClass;

	TSubclassOf<UYapBroker> BrokerClass = BrokerClassSoftPtr.LoadSynchronous();

	if (BrokerClass == nullptr)
	{
		return UYapBroker::StaticClass();
	}
	
	return BrokerClass;
}

const TArray<TSoftClassPtr<UObject>>& UYapProjectSettings::GetAudioAssetClasses()
{
	if (Get().AudioAssetClasses.Num() > 0)
	{
		return Get().AudioAssetClasses;
	}

	return Get().DefaultAssetAudioClasses;
}

#if WITH_EDITOR
void UYapProjectSettings::AddAdditionalCharacterClass(TSoftClassPtr<UObject> Class)
{
	//Get().AdditionalCharacterClasses.Add(Class);
}
#endif

#if WITH_EDITOR
const UObject* UYapProjectSettings::FindCharacter(FGameplayTag CharacterTag, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext)
{
	FYapCharacterStaticDefinition* CharacterDefinition = Get().CharacterMap.Find(CharacterTag);

	if (CharacterDefinition)
	{
		return CharacterDefinition->GetCharacter(Handle, LoadContext);
	}

	return nullptr;
}
#endif

#if WITH_EDITOR
// TODO delete this in 2026. It's only for migrating fragments from old data to new data.
const FGameplayTag& UYapProjectSettings::FindCharacterTag(const TSoftObjectPtr<UObject>& Character)
{
	for (const TTuple<FGameplayTag, FYapCharacterStaticDefinition>& Pair : Get().CharacterMap)
	{
		TSharedPtr<FStreamableHandle> TempHandle;
		
		if (Pair.Value.GetCharacter(TempHandle, EYapLoadContext::Sync) == Character)
		{
			return Pair.Key;
		}
	}
	
	return FGameplayTag::EmptyTag;
}
#endif

#if WITH_EDITOR
void UYapProjectSettings::UpdateReversedCharacterMap()
{
	auto& CharMap = Get().CharacterMap;
	auto& RevCharMap = Get().ReversedCharacterMap;
	
	Get().ReversedCharacterMap.Empty(Get().CharacterMap.Num());
	
	for (const TPair<FGameplayTag, FYapCharacterStaticDefinition>& Pair : Get().CharacterMap)
	{
		Get().ReversedCharacterMap.Add(Pair.Value, Pair.Key);
	}
}
#endif

#if WITH_EDITOR
void UYapProjectSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	static FName CharacterArrayName = GET_MEMBER_NAME_CHECKED(ThisClass, CharacterArray);
	static FName CharacterTagRootName = GET_MEMBER_NAME_CHECKED(ThisClass, CharacterTagRoot);
	
	if (PropertyChangedEvent.GetPropertyName() == CharacterTagRootName)
	{
	}
	
	if (PropertyChangedEvent.GetPropertyName() == CharacterArrayName || PropertyChangedEvent.GetMemberPropertyName() == CharacterArrayName)
	{
		if (PropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayMove)
		{
			RebuildCharacterMap();
		}
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UYapProjectSettings::PostInitProperties()
{
	Super::PostInitProperties();
	
	ProcessCharacterArray(true);
}

void UYapProjectSettings::ProcessCharacterArray(bool bUpdateMap)
{
	TSet<FGameplayTag> FoundCharacterTags;
	TSet<TSoftObjectPtr<UObject>> FoundCharacterAssets;
	TSet<FGameplayTag> DuplicateCharacterTags;
	TSet<TSoftObjectPtr<UObject>> DuplicateCharacterAssets;
	
	FoundCharacterTags.Reserve(CharacterArray.Num());
	FoundCharacterAssets.Reserve(CharacterArray.Num());
	DuplicateCharacterTags.Reserve(CharacterArray.Num());
	DuplicateCharacterAssets.Reserve(CharacterArray.Num());
	
	for (int32 i = 0; i < CharacterArray.Num(); ++i)
	{
		FYapCharacterStaticDefinition& CharacterDefinition = CharacterArray[i];

		if (CharacterDefinition.CharacterTag.IsValid())
		{
			bool bTagAlreadyExists = false;
			FoundCharacterTags.Add(CharacterDefinition.CharacterTag, &bTagAlreadyExists);
			
			if (bTagAlreadyExists)
			{
				DuplicateCharacterTags.Add(CharacterDefinition.CharacterTag);
			}
		}

		if (!CharacterDefinition.CharacterAsset.IsNull())
		{			
			bool bAssetAlreadyExists = false;
			FoundCharacterAssets.Add(CharacterDefinition.CharacterAsset, &bAssetAlreadyExists);
			
			if (bAssetAlreadyExists)
			{
				DuplicateCharacterAssets.Add(CharacterDefinition.CharacterAsset);
			}
		}
	}

	if (bUpdateMap)
	{
		CharacterMap.Empty(CharacterArray.Num());
	}
	
	for (int32 i = 0; i < CharacterArray.Num(); ++i)
	{
		FYapCharacterStaticDefinition& CharacterDefinition = CharacterArray[i];
		
		bool bAddToMap = true;
		
		CharacterDefinition.ErrorState = EYapCharacterDefinitionErrorState::OK;
		
		if (!CharacterDefinition.CharacterTag.IsValid())
		{
			bAddToMap = false;
		}
		else if (DuplicateCharacterTags.Contains(CharacterDefinition.CharacterTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Marking tag"));
			
			CharacterDefinition.ErrorState |= EYapCharacterDefinitionErrorState::TagConflict;
			bAddToMap = false;
		}

		if (!CharacterDefinition.HasValidCharacterData())
		{
			bAddToMap = false;
		}
		else if (DuplicateCharacterAssets.Contains(CharacterDefinition.CharacterAsset))
		{
			UE_LOG(LogTemp, Warning, TEXT("Marking asset"));
			
			CharacterDefinition.ErrorState |= EYapCharacterDefinitionErrorState::AssetConflict;
			bAddToMap = false;
		}

		if (bAddToMap && bUpdateMap)
		{
			CharacterMap.Add(CharacterDefinition.CharacterTag, CharacterDefinition);
		}
	}
}

void UYapProjectSettings::RebuildCharacterMap()
{
	Modify();

	ProcessCharacterArray(true);

#if WITH_EDITOR
	TryUpdateDefaultConfigFile();
#endif
}

#undef LOCTEXT_NAMESPACE
