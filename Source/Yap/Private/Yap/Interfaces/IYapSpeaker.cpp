// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Interfaces/IYapSpeaker.h"

#include "GameplayTagContainer.h"

FText IYapSpeaker::Yap_GetSpeakerName() const
{
    return K2Yap_GetSpeakerName();
}

FLinearColor IYapSpeaker::Yap_GetSpeakerColor() const
{
    return K2Yap_GetSpeakerColor();
}

FGameplayTag IYapSpeaker::Yap_GetSpeakerTag() const
{
    return K2Yap_GetSpeakerTag();
}

const UTexture2D* IYapSpeaker::Yap_GetSpeakerPortrait(const FGameplayTag& MoodTag) const
{
    return K2Yap_GetSpeakerPortrait(MoodTag);
}
