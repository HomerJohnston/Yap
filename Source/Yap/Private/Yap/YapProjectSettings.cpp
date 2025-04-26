// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#endif
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
		{ EYap_TagFilter::Prompts, &DefaultGroup.DialogueTagsParent }
	};
#endif

	DefaultAssetAudioClasses = { USoundBase::StaticClass() };

	DefaultGroup = FYapTypeGroupSettings(true);

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	DefaultGroup.DialogueTagsParent = TagsManager.AddNativeGameplayTag("Yap.Dialogue");

	MoodTagsParent = TagsManager.AddNativeGameplayTag("Yap.Mood");

	TagsManager.AddNativeGameplayTag("Yap.TypeGroup");
	
	DefaultPortraitTexture = FSoftObjectPath("/Yap/T_Avatar_Missing.T_Avatar_Missing");
	
#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UYapProjectSettings::GetMoodTagIconPath(FGameplayTag Key, FString FileExtension)
{
	int32 Index;

	FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

	if (KeyString.FindLastChar('.', Index))
	{
		KeyString = KeyString.RightChop(Index + 1);
	}

	if (Get().MoodTagEditorIconsPath.Path == "")
	{
		static FString ResourcesDir = Yap::FileUtilities::GetPluginFolder();
		
		return Yap::FileUtilities::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodTags/{0}.{1}"), { KeyString, FileExtension });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { Get().MoodTagEditorIconsPath.Path, KeyString, FileExtension });
}

#endif

#if WITH_EDITOR
FGameplayTagContainer UYapProjectSettings::GetMoodTags()
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Get().MoodTagsParent);
}
#endif

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
const FString UYapProjectSettings::GetAudioAssetRootFolder(FGameplayTag TypeGroup)
{
	const FYapTypeGroupSettings* Group = GetTypeGroupPtr(TypeGroup);

	if (!Group)
	{
		return "";
	}
	
	if (Group->AudioAssetsRootFolder.Path.IsEmpty())
	{
		return "";
	}
	
	return Group->AudioAssetsRootFolder.Path;
}
#endif

#if WITH_EDITOR
FString UYapProjectSettings::GetMoodTagIconPath()
{
	// Recache the path if it was never calculated, or if the setting is set and the cached path is not equal to it
	if (Get().MoodTagEditorIconsPath.Path.IsEmpty())
	{
		return Yap::FileUtilities::GetResourcesFolder() / TEXT("DefaultMoodTags");
	}
	else
	{
		return FPaths::ProjectDir() / Get().MoodTagEditorIconsPath.Path;
	}
}

FLinearColor UYapProjectSettings::GetGroupColor(FGameplayTag GroupName)
{
	return GetTypeGroup(GroupName).GetGroupColor();
}

void UYapProjectSettings::RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter)
{
	TMap<UClass*, EYap_TagFilter>& ClassFiltersForProperty = Get().TagFilterSubscriptions.FindOrAdd(PropertyName);

	ClassFiltersForProperty.Add(ClassSource->GetClass(), Filter);
}

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

const FYapTypeGroupSettings& UYapProjectSettings::GetTypeGroup(FGameplayTag TypeGroup)
{
	if (!TypeGroup.IsValid())
	{
		return Get().DefaultGroup;
	}

	FYapTypeGroupSettings* Group = Get().NamedGroups.Find(TypeGroup);

	if (!Group)
	{
		UE_LOG(LogYap, Error, TEXT("Yap group [%s] not found!"), *TypeGroup.ToString());
		
		return Get().DefaultGroup;
	}
	
	return *Group;
}

const FYapTypeGroupSettings* UYapProjectSettings::GetTypeGroupPtr(FGameplayTag TypeGroup)
{
	if (!TypeGroup.IsValid())
	{
		return &Get().DefaultGroup;
	}

	FYapTypeGroupSettings* Group = Get().NamedGroups.Find(TypeGroup);

	if (!Group)
	{
		UE_LOG(LogYap, Error, TEXT("Yap group [%s] not found!"), *TypeGroup.ToString());
		
		return nullptr;
	}
	
	return Group;
}

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
