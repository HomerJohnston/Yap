#// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDomainConfig.h"

#include "GameplayTagsManager.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"

FYapDomainSettings_FlowGraphSettings::FYapDomainSettings_FlowGraphSettings()
{
}

FGameplayTagContainer FYapDomainConfig_MoodTags::GetAllMoodTags()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTagsParent);
}

UYapDomainConfig::UYapDomainConfig()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
    
    General.AllowableNodeTypes = static_cast<int32>(EYapDialogueNodeType::COUNT) - 1;
}

TArray<FString>& UYapDomainConfig::GetDefaultMoodTags()
{
    static TArray<FString> Tags
    {
        "Angry",
        "Calm",
        "Confused",
        "Disgusted",
        "Happy",
        "Injured",
        "Laughing",
        "Panicked",
        "Sad",
        "Scared",
        "Smirking",
        "Stressed",
        "Surprised",
        "Thinking",
        "Tired"
    };

    return Tags;
}

/*
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
#endif
*/

/*
#if WITH_EDITOR
FString UYapDomainConfig::GetMoodTagIconPath(FGameplayTag Key, FString FileExtension)
{
    int32 Index;

    FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

    if (KeyString.FindLastChar('.', Index))
    {
        KeyString = KeyString.RightChop(Index + 1);
    }

    if (MoodTags.MoodTagEditorIconsPath.Path == "")
    {
        static FString ResourcesDir = Yap::FileUtilities::GetPluginFolder();
		
        return Yap::FileUtilities::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodTags/{0}.{1}"), { KeyString, FileExtension });
    }
	
    return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { MoodTags.MoodTagEditorIconsPath.Path, KeyString, FileExtension });
}
#endif
*/

#if WITH_EDITOR
FGameplayTagContainer UYapDomainConfig::GetMoodTags()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTags.MoodTagsParent);
}
#endif
