// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Interfaces/IYapCharacterInterface.h"

#include "GameplayTagContainer.h"

// ================================================================================================
// Default K2 implementations call the C++ implementations

FText IYapCharacterInterface::K2_GetYapCharacterName_Implementation() const
{
    return GetYapCharacterName();
}

FLinearColor IYapCharacterInterface::K2_GetYapCharacterColor_Implementation() const
{
    return GetYapCharacterColor();
};

FGameplayTag IYapCharacterInterface::K2_GetYapCharacterTag_Implementation() const
{
    return GetYapCharacterTag();
};

const UTexture2D* IYapCharacterInterface::K2_GetYapCharacterPortrait_Implementation(const FGameplayTag& MoodTag) const
{
    return GetYapCharacterPortrait(MoodTag);
};

// ================================================================================================
// Default C++ implementations need to be overridden

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


// ================================================================================================
// Public API for actual use

FText IYapCharacterInterface::GetName(const UObject* Character)
{
    FText Name = FText::GetEmpty();

    if (IsValid(Character))
    {
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(Character))
        {
            Name = Speaker->GetYapCharacterName();
        }
        else if (Character->Implements<UYapCharacterInterface>())
        {
            Name = IYapCharacterInterface::Execute_K2_GetYapCharacterName(Character);
        }
    }

    return Name;
}

FLinearColor IYapCharacterInterface::GetColor(const UObject* Character)
{
    FLinearColor Color(0.030f, 0.030f, 0.030f, 1.0f);

    if (IsValid(Character))
    {
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(Character))
        {
            Color = Speaker->GetYapCharacterColor();
        }
        else if (Character->Implements<UYapCharacterInterface>())
        {
            Color = IYapCharacterInterface::Execute_K2_GetYapCharacterColor(Character);
        }
    }

    return Color;
}

FGameplayTag IYapCharacterInterface::GetTag(const UObject* Character)
{
    FGameplayTag Tag = FGameplayTag::EmptyTag;

    if (IsValid(Character))
    {
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(Character))
        {
            Tag = Speaker->GetYapCharacterTag();
        }
        else if (Character->Implements<UYapCharacterInterface>())
        {
            Tag = IYapCharacterInterface::Execute_K2_GetYapCharacterTag(Character);
        }
    }

    return Tag;
}

const UTexture2D* IYapCharacterInterface::GetPortrait(const UObject* Character, FGameplayTag MoodTag)
{
    const UTexture2D* Texture = nullptr;
    
    if (IsValid(Character))
    {
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(Character))
        {
            Texture = Speaker->GetYapCharacterPortrait(MoodTag);
        }
        else if (Character->Implements<UYapCharacterInterface>())
        {
            Texture = IYapCharacterInterface::Execute_K2_GetYapCharacterPortrait(Character, MoodTag);
        }
    }

    return Texture;
}
