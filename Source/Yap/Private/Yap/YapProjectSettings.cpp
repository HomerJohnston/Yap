// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#endif
#include "Yap/YapCharacterAsset.h"
#include "Yap/Globals/YapFileUtilities.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
#endif

UYapProjectSettings::UYapProjectSettings()
{
#if WITH_EDITORONLY_DATA
	TagContainers =
	{
		//{ EYap_TagFilter::Prompts, &DefaultGroup.DialogueTagsParent }
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
}

void UYapProjectSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	static FName CharacterArrayName = GET_MEMBER_NAME_CHECKED(ThisClass, CharacterArray);

	if (PropertyChangedEvent.GetPropertyName() == CharacterArrayName || PropertyChangedEvent.GetMemberPropertyName() == CharacterArrayName)
	{
		RebuildCharacterMap();
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UYapProjectSettings::ProcessCharacterArray()
{
	TSet<FGameplayTag> FoundCharacterTags;
	TSet<TSoftObjectPtr<UObject>> FoundCharacterAssets;
	TSet<FGameplayTag> DuplicateCharacterTags;
	TSet<TSoftObjectPtr<UObject>> DuplicateCharacterAssets;
	TSet<int32> UndefinedCharacters;
	
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

	for (int32 i = 0; i < CharacterArray.Num(); ++i)
	{
		FYapCharacterDefinition& CharacterDefinition = CharacterArray[i];

		CharacterDefinition.ErrorState = EYapCharacterDefinitionErrorState::OK;

		if (!CharacterDefinition.CharacterTag.IsValid())
		{
			UndefinedCharacters.Add(i);
		}
		else if (DuplicateCharacterTags.Contains(CharacterDefinition.CharacterTag))
		{
			CharacterDefinition.ErrorState |= EYapCharacterDefinitionErrorState::TagConflict;
		}

		if (CharacterDefinition.CharacterAsset.IsNull())
		{
			UndefinedCharacters.Add(i);
		}
		else if (DuplicateCharacterAssets.Contains(CharacterDefinition.CharacterAsset))
		{
			CharacterDefinition.ErrorState |= EYapCharacterDefinitionErrorState::AssetConflict;
		}
	}

	CharacterMap.Empty(CharacterArray.Num());

	for (int32 i = 0; i < CharacterArray.Num(); ++i)
	{
		if (UndefinedCharacters.Contains(i))
		{
			continue;
		}
		
		const FYapCharacterDefinition& CharacterDefinition = CharacterArray[i];

		if (CharacterDefinition.ErrorState == EYapCharacterDefinitionErrorState::OK)
		{
			CharacterMap.Add(CharacterDefinition.CharacterTag, CharacterDefinition.CharacterAsset);
		}
	}
}

void UYapProjectSettings::RebuildCharacterMap()
{
	Modify();

	ProcessCharacterArray();
	
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
