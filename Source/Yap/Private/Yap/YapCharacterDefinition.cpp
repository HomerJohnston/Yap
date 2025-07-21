// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacterDefinition.h"

#include "Engine/StreamableManager.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/Enums/YapLoadContext.h"

UObject* FYapCharacterDefinition::GetCharacter(TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext) const
{
    if (!HasValidCharacterData())
    {
        return nullptr;
    }
    
    if (!CharacterAsset.IsNull())
    {
        switch (LoadContext)
        {
            case EYapLoadContext::Async:
            {
                Handle = FYapStreamableManager::Get().RequestAsyncLoad(CharacterAsset.ToSoftObjectPath());
                break;
            }
            case EYapLoadContext::AsyncEditorOnly:
            {
                FYapStreamableManager::Get().RequestAsyncLoad(CharacterAsset.ToSoftObjectPath());
                break;
            }
            case EYapLoadContext::Sync:
            {
                Handle = FYapStreamableManager::Get().RequestSyncLoad(CharacterAsset.ToSoftObjectPath());
                break;
            }
            default:
            {
                // Do not load
            }
        }

        if (UClass* CharacterClass = Cast<UClass>(CharacterAsset.Get()))
        {
            return CharacterClass->GetDefaultObject();
        }
        
        return CharacterAsset.Get();
    }

    return nullptr;
}

bool FYapCharacterDefinition::HasValidCharacterData() const
{
    if (CharacterAsset.IsNull())
    {
        return false;
    }

    return true;
}
