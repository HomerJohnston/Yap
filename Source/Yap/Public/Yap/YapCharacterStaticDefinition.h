// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

#include "YapCharacterStaticDefinition.generated.h"

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
struct YAP_API FYapCharacterStaticDefinition
{
    GENERATED_BODY()

    friend class UYapProjectSettings;

#if WITH_EDITOR
    friend class FPropertyCustomization_YapCharacterDefinition;
    friend class FDetailCustomization_YapProjectSettings;
#endif

public:
    FYapCharacterStaticDefinition()
    {
    }

    FYapCharacterStaticDefinition(FGameplayTag InCharacterTag)
        : CharacterTag(InCharacterTag)
    {
    }

protected:
    /**  */
    UPROPERTY(EditAnywhere)
    FGameplayTag CharacterTag;

    /**  */
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UObject> CharacterAsset;

public:

    const FGameplayTag& GetCharacterTag() const { return CharacterTag; }
    
    TSoftObjectPtr<UObject> GetCharacter_Soft() const { return CharacterAsset; };
    
    // TODO this should probably be deprecated. Move into the character manager stuff.
    UObject* GetCharacter(TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext) const;

    UPROPERTY(Transient)
    EYapCharacterDefinitionErrorState ErrorState = EYapCharacterDefinitionErrorState::OK;

    bool HasValidCharacterData() const;
    
    bool operator< (const FYapCharacterStaticDefinition& OtherCharacter) const
    {
        return CharacterTag < OtherCharacter.CharacterTag;
    }
    
    friend uint32 GetTypeHash(const FYapCharacterStaticDefinition& This)
    {
        return GetTypeHash(This.CharacterTag);
    }

    friend bool operator==(const FYapCharacterStaticDefinition& This, const FYapCharacterStaticDefinition& Other)
    {
        return This.CharacterTag == Other.CharacterTag;
    }
};
