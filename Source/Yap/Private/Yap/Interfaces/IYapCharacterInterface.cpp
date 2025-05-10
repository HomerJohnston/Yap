// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Interfaces/IYapCharacterInterface.h"

#include "GameplayTagContainer.h"

FText IYapCharacterInterface::GetYapCharacterName() const
{
    return K2_GetYapCharacterName();
}

FLinearColor IYapCharacterInterface::GetYapCharacterColor() const
{
    return K2_GetYapCharacterColor();
}

FGameplayTag IYapCharacterInterface::GetYapCharacterTag() const
{
    return K2_GetYapCharacterTag();
}

const UTexture2D* IYapCharacterInterface::GetYapCharacterPortrait(const FGameplayTag& MoodTag) const
{
    return K2_GetYapCharacterPortrait(MoodTag);
}
