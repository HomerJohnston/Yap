// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

#include "YapCharacterDefinition.generated.h"

struct FStreamableHandle;
enum class EYapLoadContext : uint8;

UENUM()
enum class EYapCharacterDefinitionErrorState : uint8
{
    OK              = 0,
    TagConflict         = 1 << 2,
    AssetConflict       = 1 << 4
};

ENUM_CLASS_FLAGS(EYapCharacterDefinitionErrorState);

USTRUCT()
struct YAP_API FYapCharacterDefinition
{
    GENERATED_BODY()

    friend class UYapProjectSettings;

#if WITH_EDITOR
    friend class FPropertyCustomization_YapCharacterDefinition;
    friend class FDetailCustomization_YapProjectSettings;
#endif
    
    FYapCharacterDefinition()
    {
    }

    FYapCharacterDefinition(FGameplayTag InCharacterTag)
        : CharacterTag(InCharacterTag)
    {
    }

    UPROPERTY(EditAnywhere)
    FGameplayTag CharacterTag;

protected:
    // If an asset is selected, this will store it
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UObject> CharacterAsset;

public:
    UObject* GetCharacter(TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext) const;

    UPROPERTY(Transient)
    EYapCharacterDefinitionErrorState ErrorState = EYapCharacterDefinitionErrorState::OK;

    bool HasValidCharacterData() const;
    
    bool operator< (const FYapCharacterDefinition& OtherCharacter) const
    {
        return CharacterTag < OtherCharacter.CharacterTag;
    }
    
    friend uint32 GetTypeHash(const FYapCharacterDefinition& This)
    {
        return GetTypeHash(This.CharacterTag);
    }

    friend bool operator==(const FYapCharacterDefinition& This, const FYapCharacterDefinition& Other)
    {
        return This.CharacterTag == Other.CharacterTag;
    }
};
