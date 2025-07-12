// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#endif
#include "Engine/AssetManager.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/Globals/YapFileUtilities.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
TMap<TSoftObjectPtr<UObject>, FGameplayTag> UYapProjectSettings::ReversedCharacterMap;
#endif

UYapProjectSettings::UYapProjectSettings()
{
#if WITH_EDITORONLY_DATA
	TagContainers =
	{
		//{ EYap_TagFilter::Prompts, &DefaultGroup.DialogueTagsParent }
		{ EYap_TagFilter::Characters, &CharacterTagRoot }
	};
#endif

	DefaultAssetAudioClasses = { USoundBase::StaticClass() };

	DefaultCharacterClasses = { UYapCharacterAsset::StaticClass() };

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		
	DefaultPortraitTexture = FSoftObjectPath("/Yap/T_Avatar_Missing.T_Avatar_Missing");

	DefaultNodeConfig = UYapNodeConfig_Default::StaticClass();
	
#if WITH_EDITOR
	TagsManager.OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
const UYapBroker* UYapProjectSettings::GetEditorBrokerDefault()
{ 
	TSoftClassPtr<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass();

	if (BrokerClass.IsNull())
	{
		UE_LOG(LogYap, Error, TEXT("No broker class set! Set a Yap Broker class in project settings."));
		return nullptr;
	}

	return BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();
}

const TArray<const UClass*> UYapProjectSettings::GetAdditionalCharacterClasses()
{
	TArray<const UClass*> Classes;
	Classes.Reserve(Get().AdditionalCharacterClasses.Num() + Get().DefaultCharacterClasses.Num());

	for (auto& ClassSoftPtr : Get().DefaultCharacterClasses)
	{
		UClass* LoadedClass = ClassSoftPtr.LoadSynchronous();
		UE_LOG(LogTemp, Display, TEXT(":::::::::: %s"), *LoadedClass->GetFullName());
		Classes.Add(LoadedClass);
	}
	
	for (auto& ClassSoftPtr : Get().AdditionalCharacterClasses)
	{
		UClass* LoadedClass = ClassSoftPtr.LoadSynchronous();
		UE_LOG(LogTemp, Display, TEXT(":::::::::: %s"), *LoadedClass->GetFullName());
		Classes.Add(LoadedClass);
	}

	return Classes;
}
#endif

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
	Get().AdditionalCharacterClasses.Add(Class);
}
#endif

#if WITH_EDITOR
void UYapProjectSettings::UpdateReversedCharacterMap()
{
	auto& CharMap = Get().CharacterMap;
	auto& RevCharMap = Get().ReversedCharacterMap;
	
	Get().ReversedCharacterMap.Empty(Get().CharacterMap.Num());
	
	for (const TTuple<FGameplayTag, TSoftObjectPtr<>>& Pair : Get().CharacterMap)
	{
		Get().ReversedCharacterMap.Add(Pair.Value, Pair.Key);
	}
}

void UYapProjectSettings::RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter)
{
	TMap<UClass*, EYap_TagFilter>& ClassFiltersForProperty = Get().TagFilterSubscriptions.FindOrAdd(PropertyName);

	ClassFiltersForProperty.Add(ClassSource->GetClass(), Filter);
}
#endif

#if WITH_EDITOR
void UYapProjectSettings::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle)
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* PropertyOuter : OuterObjects)
	{
		if (PropertyOuter != this)
		{
			continue;
		}

		//static const FName CharacterTagRootName = GET_MEMBER_NAME_CHECKED(ThisClass, CharacterTagRoot);
		
		if (PropertyHandle->GetProperty()->GetFName() == "CharacterTag")
		{
			MetaString = CharacterTagRoot.ToString();			
		}
	}
	
	/*
	const TMap<UClass*, EYap_TagFilter>* ClassFilters = TagFilterSubscriptions.Find(PropertyHandle->GetProperty()->GetFName());

	if (!ClassFilters)
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* PropertyOuter : OuterObjects)
	{
		const EYap_TagFilter* Filter = ClassFilters->Find(PropertyOuter->GetClass());

		if (!Filter)
		{
			continue;
		}

		MetaString = TagContainers[*Filter]->ToString();
	}
	*/
}

void UYapProjectSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	static FName CharacterArrayName = GET_MEMBER_NAME_CHECKED(ThisClass, CharacterArray);

	if (PropertyChangedEvent.GetPropertyName() == CharacterArrayName || PropertyChangedEvent.GetMemberPropertyName() == CharacterArrayName)
	{
		if (PropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayMove)
		{
			RebuildCharacterMap();
		}
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UYapProjectSettings::PostLoad()
{
	Super::PostLoad();
	
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, CharacterTagRoot), EYap_TagFilter::Characters);
}

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
		FYapCharacterDefinition& CharacterDefinition = CharacterArray[i];

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
		FYapCharacterDefinition& CharacterDefinition = CharacterArray[i];

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

		if (CharacterDefinition.CharacterAsset.IsNull())
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
			CharacterMap.Add(CharacterDefinition.CharacterTag, CharacterDefinition.CharacterAsset);
		}
	}
}

void UYapProjectSettings::RebuildCharacterMap()
{
	Modify();

	ProcessCharacterArray(true);
	
	TryUpdateDefaultConfigFile();
}
#endif

#if WITH_EDITOR
// TODO someone posted a nicer way to do this in Slackers without this... something about simple name? using the node?? can't remember
FString UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag)
{
	const FGameplayTag& ParentContainer = *Get().TagContainers[Filter];
	
	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && PropertyTag.MatchesTag(ParentContainer))
	{
		return PropertyTag.ToString().RightChop(ParentContainer.ToString().Len() + 1);
	}

	if (PropertyTag == FGameplayTag::EmptyTag)
	{
		return "";
	}
	
	return PropertyTag.ToString();
}
#endif

#undef LOCTEXT_NAMESPACE
