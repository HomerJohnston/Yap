#// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapGroupSettings.h"

#include "GameplayTagsManager.h"
#include "Yap/YapProjectSettings.h"

FYapGroupSettings::FYapGroupSettings()
{
}

FYapGroupSettings::FYapGroupSettings(bool bDefaultIn)
{
    bDefault = bDefaultIn;
}

TArray<FString>& FYapGroupSettings::GetDefaultMoodTags()
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

const FYapGroupSettings& FYapGroupSettings::Default()
{
    return UYapProjectSettings::GetTypeGroup(FGameplayTag::EmptyTag);
}
