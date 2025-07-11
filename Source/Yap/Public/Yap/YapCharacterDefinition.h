// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

#include "YapCharacterDefinition.generated.h"

enum class EYapCharacterDefinitionErrorState : uint8
{
    OK              = 0,
    TagConflict         = 1 << 2,
    AssetConflict       = 1 << 4
};

ENUM_CLASS_FLAGS(EYapCharacterDefinitionErrorState);

USTRUCT()
struct FYapCharacterDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FGameplayTag CharacterTag;

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UObject> CharacterAsset;

    EYapCharacterDefinitionErrorState ErrorState;
};