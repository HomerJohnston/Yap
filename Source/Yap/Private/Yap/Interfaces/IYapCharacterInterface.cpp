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

FGameplayTag IYapCharacterInterface::GetYapCharacterTag() const
{
    return K2_GetYapCharacterTag();
}

// ----------------------------------------------

const UTexture2D* IYapCharacterInterface::GetCharacterPortrait(const FGameplayTag& MoodTag) const
{
    return K2_GetYapCharacterPortrait(MoodTag);
}

bool IYapCharacterInterface::IsAsset_YapCharacter(const TSoftObjectPtr<UObject> AssetSoftPtr)
{
    const UObject* Asset = AssetSoftPtr.LoadSynchronous();

    if (Asset->Implements<UYapCharacterInterface>())
    {
        return true;
    }

    if (const UBlueprint* AssetBlueprint = Cast<UBlueprint>(Asset))
    {
        // If the blueprint's generated class implements the interface, return true
        if (AssetBlueprint->GeneratedClass && AssetBlueprint->GeneratedClass->ImplementsInterface(UYapCharacterInterface::StaticClass()))
        {
            return true;
        }
    }
    
    return false;
}

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
            Texture = Speaker->GetCharacterPortrait(MoodTag);
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
