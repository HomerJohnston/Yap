// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacterManager.h"

#include "Yap/YapCharacterAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Interfaces/IYapCharacterInterface.h"
#include "Yap/YapCharacterRuntimeDefinition.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================

TSharedPtr<FStreamableHandle> FYapCharacterRegisteredInstance::RequestLoadAsync()
{
	if (IsValid(CharacterHardPtr))
	{
		return nullptr;
	}

	if (CharacterSoftPtr.IsNull())
	{
		return nullptr;
	}

	return FYapStreamableManager::Get().RequestAsyncLoad(CharacterSoftPtr.ToSoftObjectPath());
}

TSharedPtr<FStreamableHandle> FYapCharacterRegisteredInstance::RequestLoad()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogYap, Display, TEXT("Sync-loading character <%s>."), *CharacterSoftPtr.GetAssetName());
#endif

	return FYapStreamableManager::Get().RequestSyncLoad(CharacterSoftPtr.ToSoftObjectPath());
}

// ------------------------------------------------------------------------------------------------

UObject* FYapCharacterRegisteredInstance::GetLoadedCharacter()
{
	if (IsValid(CharacterHardPtr))
	{
		return CharacterHardPtr;
	}

	if (CharacterSoftPtr.IsNull())
	{
		return nullptr;
	}

	if (CharacterSoftPtr.IsPending())
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogYap, Warning, TEXT("Sync-loading character <%s>, this will cause a hitch! Try to request loading sooner."), *CharacterSoftPtr.GetAssetName());
#endif
	}

	return CharacterSoftPtr.LoadSynchronous();
}

// ================================================================================================

void UYapCharacterManager::Initialize()
{
	// Pull all of the project's characters from the project settings and register them

	const TMap<FGameplayTag, FYapCharacterStaticDefinition>& Characters = UYapProjectSettings::GetCharacters();

	for (auto& [Tag, CharacterDefinition] : Characters)
	{
		if (Tag.IsValid() && !CharacterDefinition.GetCharacter_Soft().IsNull())
		{
			RegisteredCharacters.Emplace(Tag.GetTagName(), { CharacterDefinition.GetCharacter_Soft() });
		}
	}
}

// ------------------------------------------------------------------------------------------------

void UYapCharacterManager::RegisterCharacter(FName CharacterID,	const TInstancedStruct<FYapCharacterRuntimeDefinition>& CharacterDefinition, bool bReplaceExisting)
{
	if (CharacterID == NAME_None)
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a character ID of none, ignoring!"));
		return;
	}

	if (!CharacterDefinition.IsValid())
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register character ID <%s> but Character Definition was not set!"), *CharacterID.ToString());
		return;
	}

	FYapCharacterRegisteredInstance* Existing = RegisteredCharacters.Find(CharacterID);

	if (Existing)
	{
		if (!bReplaceExisting)
		{
			UE_LOG(LogYap, Error, TEXT("Tried to register character ID <%s>, but this ID was already registered!"), *CharacterID.ToString());
			return;
		}
		
		UE_LOG(LogYap, Display, TEXT("New character registration for ID <%s> is stomping an existing character."), *CharacterID.ToString());
	}

	UYapCharacterAsset* NewCharacter = NewObject<UYapCharacterAsset>(this);

	CharacterDefinition.Get().InitializeCharacter(NewCharacter);

	RegisteredCharacters.Add(CharacterID, NewCharacter);
}

// ------------------------------------------------------------------------------------------------

void UYapCharacterManager::RegisterCharacter(FName CharacterID, TSoftObjectPtr<UObject> Character, bool bReplaceExisting)
{
	if (!RegisterCharacter_Check<TSoftObjectPtr<UObject>>(CharacterID, Character, bReplaceExisting))
	{
		return;
	}
	
	RegisteredCharacters.Add(CharacterID, Character);
}

// ------------------------------------------------------------------------------------------------

void UYapCharacterManager::RegisterCharacter(FName CharacterID, UObject* Character, bool bReplaceExisting)
{
	if (!RegisterCharacter_Check<UObject*>(CharacterID, Character, bReplaceExisting))
	{
		return;
	}
	
	RegisteredCharacters.Add(CharacterID, Character);
}

// ------------------------------------------------------------------------------------------------

void UYapCharacterManager::UnregisterCharacter(FName CharacterID)
{
	RegisteredCharacters.Remove(CharacterID);
}

// ------------------------------------------------------------------------------------------------

TScriptInterface<IYapCharacterInterface> UYapCharacterManager::FindCharacter(FName CharacterID)
{
	FYapCharacterRegisteredInstance* Existing = RegisteredCharacters.Find(CharacterID);

	if (Existing)
	{
		return TScriptInterface<IYapCharacterInterface>(Existing->GetLoadedCharacter());
	}

	return TScriptInterface<IYapCharacterInterface>(nullptr);
}

// ------------------------------------------------------------------------------------------------

TSharedPtr<FStreamableHandle> UYapCharacterManager::RequestLoadAsync(FName CharacterID)
{
	FYapCharacterRegisteredInstance* Existing = RegisteredCharacters.Find(CharacterID);

	if (Existing)
	{
		return Existing->RequestLoadAsync();
	}

	return nullptr;
}

// ================================================================================================

void UYapCharacterManager_BPFL::RegisterCharacter(UObject* WorldContext, FName CharacterID, UObject* CharacterObject, bool bReplaceExisting)
{
	UYapCharacterManager& CharacterManager = GetCharacterManager(WorldContext);

	CharacterManager.RegisterCharacter(CharacterID, CharacterObject, bReplaceExisting);
}

void UYapCharacterManager_BPFL::RegisterCharacter_Dynamic(UObject* WorldContext, FName CharacterID, FInstancedStruct CharacterDefinition, bool bReplaceExisting)
{
	UYapCharacterManager& CharacterManager = GetCharacterManager(WorldContext);

	if (CharacterDefinition.GetScriptStruct()->IsChildOf(FYapCharacterRuntimeDefinition::StaticStruct()))
	{
		auto TCharacterDefinition = TInstancedStruct<FYapCharacterRuntimeDefinition>::Make(CharacterDefinition.Get<FYapCharacterRuntimeDefinition>());
		
		CharacterManager.RegisterCharacter(CharacterID, TCharacterDefinition, bReplaceExisting);
	}
}

// ------------------------------------------------------------------------------------------------

void UYapCharacterManager_BPFL::UnregisterCharacter(UObject* WorldContext, FName CharacterID)
{
	UYapCharacterManager& CharacterManager = GetCharacterManager(WorldContext);

	CharacterManager.UnregisterCharacter(CharacterID);
}

// ------------------------------------------------------------------------------------------------

UYapCharacterManager& UYapCharacterManager_BPFL::GetCharacterManager(UObject* WorldContext)
{
	UYapSubsystem* Subsystem = UYapSubsystem::Get(WorldContext);

	if (!IsValid(Subsystem))
	{
		checkNoEntry();
	}

	return Subsystem->GetCharacterManager(WorldContext);
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE