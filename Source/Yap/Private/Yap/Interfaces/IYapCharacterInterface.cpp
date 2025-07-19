// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Interfaces/IYapCharacterInterface.h"

#include "GameplayTagContainer.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"

// ================================================================================================
// Default C++ implementations, can be overridden by C++ classes

FText IYapCharacterInterface::GetCharacterName() const
{
    return K2_GetYapCharacterName();
}

// ----------------------------------------------

FLinearColor IYapCharacterInterface::GetCharacterColor() const
{
    return K2_GetYapCharacterColor();
}

// ----------------------------------------------

const UTexture2D* IYapCharacterInterface::GetCharacterPortrait(const FGameplayTag& MoodTag) const
{
    return K2_GetYapCharacterPortrait(MoodTag);
}

#if WITH_EDITOR
bool IYapCharacterInterface::IsAsset_YapCharacter(const TSoftObjectPtr<UObject> AssetSoftPtr)
{
    const UObject* Asset = AssetSoftPtr.LoadSynchronous();

    if (Asset->Implements<UYapCharacterInterface>())
    {
        return true;
    }
    
    return false;
}

bool IYapCharacterInterface::IsAsset_YapCharacter(const TSoftClassPtr<UObject> Class)
{
    TSubclassOf<UObject> AssetClass = Class.LoadSynchronous();

    if (AssetClass->ImplementsInterface(UYapCharacterInterface::StaticClass()))
    {
        return true;
    }

    return false;
}
#endif

#if WITH_EDITOR
bool IYapCharacterInterface::IsAsset_YapCharacter(const FAssetData& AssetData)
{
    const UClass* Class = AssetData.GetClass();

    if (!Class)
    {
        return false;
    }

    if (Class->ImplementsInterface(UYapCharacterInterface::StaticClass()))
    {
        //UE_LOG(LogYapEditor, VeryVerbose, TEXT("Found valid speaker class from asset: %s"), *Class->GetName());
        return true;
    }
	
    const TArray<const UClass*> AllowableClasses = UYapProjectSettings::GetAllowableCharacterClasses();

    FString AllowableClassStringTemp;
	
    if (AllowableClasses.Num() > 0)
    {
        for (TSoftClassPtr<UObject> AllowableClass : AllowableClasses)
        {
            if (!AllowableClass)
            {
                continue;
            }
			
            const FString PackageName = AssetData.PackageName.ToString();
			
            AllowableClassStringTemp = AllowableClass->GetPackage()->GetName();
			
            if (PackageName == AllowableClassStringTemp)
            {
                //UE_LOG(LogYapEditor, VeryVerbose, TEXT("Found valid speaker class from package path comparison: %s"), *Class->GetName());
                return true;
            }
        }
    }
	
    return false;
}
#endif

// ================================================================================================
// Public API for C++ usage

FText IYapCharacterInterface::GetName(const UObject* Character)
{
    FText Name = FText::GetEmpty();

    if (IsValid(Character))
    {
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(Character))
        {
            Name = Speaker->GetCharacterName();
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
            Color = Speaker->GetCharacterColor();
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

const UTexture2D* IYapCharacterInterface::GetPortrait(const UObject* CharacterAsset, FGameplayTag MoodTag)
{
    const UTexture2D* Texture = nullptr;
    
    if (IsValid(CharacterAsset))
    {        
        if (const IYapCharacterInterface* Speaker = Cast<IYapCharacterInterface>(CharacterAsset))
        {
            Texture = Speaker->GetCharacterPortrait(MoodTag);
        }
        else if (CharacterAsset->Implements<UYapCharacterInterface>())
        {
            Texture = IYapCharacterInterface::Execute_K2_GetYapCharacterPortrait(CharacterAsset, MoodTag);
        }
        else
        {
            UE_LOG(LogYap, Error, TEXT("IYapCharacterInterface::GetPortrait failure - Object [%s] did not implement IYapCharacterInterface in C++ or blueprint!"), *CharacterAsset->GetName());
        }
    }

    return Texture;
}
