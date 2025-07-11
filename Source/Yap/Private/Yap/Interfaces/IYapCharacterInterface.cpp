// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Interfaces/IYapCharacterInterface.h"

#include "GameplayTagContainer.h"
#include "Yap/YapLog.h"

// ================================================================================================
// Default C++ implementations, can be overridden by C++ classes

FText IYapCharacterInterface::GetYapCharacterName() const
{
    return K2_GetYapCharacterName();
}

// ----------------------------------------------

FLinearColor IYapCharacterInterface::GetYapCharacterColor() const
{
    return K2_GetYapCharacterColor();
}

// ----------------------------------------------

FGameplayTag IYapCharacterInterface::GetYapCharacterTag() const
{
    return K2_GetYapCharacterTag();
}

// ----------------------------------------------

const UTexture2D* IYapCharacterInterface::GetYapCharacterPortrait(const FGameplayTag& MoodTag) const
{
    return K2_GetYapCharacterPortrait(MoodTag);
}

// ================================================================================================
// Public API for C++ usage

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
        else
        {
            UE_LOG(LogYap, Error, TEXT("IYapCharacterInterface::GetName failure - Object [%s] did not implement IYapCharacterInterface in C++ or blueprint!"), *Character->GetName());
        }
    }

    return Name;
}

// ----------------------------------------------

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
        else
        {
            UE_LOG(LogYap, Error, TEXT("IYapCharacterInterface::GetColor failure - Object [%s] did not implement IYapCharacterInterface in C++ or blueprint!"), *Character->GetName());
        }
    }

    return Color;
}

// ----------------------------------------------

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
        else
        {
            UE_LOG(LogYap, Error, TEXT("IYapCharacterInterface::GetTag failure - Object [%s] did not implement IYapCharacterInterface in C++ or blueprint!"), *Character->GetName());
        }
    }

    return Tag;
}

// ----------------------------------------------

const UTexture2D* IYapCharacterInterface::GetPortrait(const UObject* CharacterAsset, FGameplayTag MoodTag)
{
    const UTexture2D* Texture = nullptr;
    
    if (IsValid(CharacterAsset))
    {
        const UObject* TargetObject = CharacterAsset;
        
        if (const UBlueprint* Blueprint = Cast<UBlueprint>(TargetObject))
        {
            TargetObject = Blueprint->GeneratedClass.GetDefaultObject();
        }
        
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(TargetObject))
        {
            Texture = Speaker->GetYapCharacterPortrait(MoodTag);
        }
        else if (TargetObject->Implements<UYapCharacterInterface>())
        {
            Texture = IYapCharacterInterface::Execute_K2_GetYapCharacterPortrait(TargetObject, MoodTag);
        }
        else if (const UBlueprint* Blueprint = Cast<UBlueprint>(TargetObject))
        {
            UE_LOG(LogYap, Error, TEXT("IYapCharacterInterface::GetPortrait failure - Object [%s] did not implement IYapCharacterInterface in C++ or blueprint!"), *CharacterAsset->GetName());
        }
    }

    return Texture;
}
