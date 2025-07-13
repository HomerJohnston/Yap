// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagsManager.h"

#include "YapGameplayTagFilters.generated.h"

enum class EYap_TagFilter : uint8
{
    Conditions,
    Prompts,
    Characters,
};

UCLASS()
class UYapGameplayTagFilters : public UObject
{
    GENERATED_BODY()

    UYapGameplayTagFilters()
    {
        if (!IsTemplate())
        {
            return;
        }

        UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
        
        GameplayTagsManager.OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &UYapGameplayTagFilters::Process);
    }

    void Process(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
    {
        
    }
};
