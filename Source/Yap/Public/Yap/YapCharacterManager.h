// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "InstancedStruct.h"
#include "YapLog.h"
#include "Interfaces/IYapCharacterInterface.h"

#include "YapCharacterManager.generated.h"

class UYapCharacterAsset;
class IYapCharacterInterface;
struct FStreamableHandle;
struct FYapCharacterRuntimeDefinition;

USTRUCT()
struct FYapCharacterRegisteredInstance
{
	GENERATED_BODY()

	FYapCharacterRegisteredInstance() {}

	FYapCharacterRegisteredInstance(TSoftObjectPtr<UObject> CharacterAsset)
	{
		CharacterSoftPtr = CharacterAsset;
	}

	FYapCharacterRegisteredInstance(UObject* CharacterInstance)
	{
		CharacterHardPtr = CharacterInstance;
	}

private:
	UPROPERTY()
	TSoftObjectPtr<UObject> CharacterSoftPtr;

	UPROPERTY()
	TObjectPtr<UObject> CharacterHardPtr;

public:
	TSharedPtr<FStreamableHandle> RequestLoadAsync();

	TSharedPtr<FStreamableHandle> RequestLoad();

	// Used by speech functions; resolves the soft or hard ptr, whatever was set, loads and returns it
	UObject* GetLoadedCharacter();

	// Utility access for registration
	UObject* GetHardPtr() const { return CharacterHardPtr; }

	// Utility access for registration
	const TSoftObjectPtr<UObject>& GetSoftPtr() const { return CharacterSoftPtr; }
};

// ================================================================================================

/**
 * Intended to be a child object of the general UYapSubsystem, this class is where all speaker data is maintained during runtime.
 */
UCLASS()
class YAP_API UYapCharacterManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

protected:
	UPROPERTY()
	TMap<FName, FYapCharacterRegisteredInstance> RegisteredCharacters;

public:
	void RegisterCharacter(FName CharacterID, const TInstancedStruct<FYapCharacterRuntimeDefinition>& CharacterDefinition, bool bReplaceExisting = false);

	void RegisterCharacter(FName CharacterID, TSoftObjectPtr<UObject> Character, bool bReplaceExisting = false);
	
	/**
	 * 
	 * @param CharacterID ID of the character to register
	 * @param Character Character object which must implement IYapCharacterInterface; this can be an actor in the world, a blueprint CDO, or an asset
	 * @param bReplaceExisting If left unset, attempting to register a different character to an ID will throw an error
	 */
	void RegisterCharacter(FName CharacterID, UObject* Character, bool bReplaceExisting = false);

	template<typename T>
	bool RegisterCharacter_Check(FName CharacterID, T Character, bool bReplaceExisting)
	{
		if (CharacterID == NAME_None)
		{
			UE_LOG(LogYap, Error, TEXT("Tried to register a character ID of none, ignoring!"));
			return false;
		}
		
		FYapCharacterRegisteredInstance* Existing = RegisteredCharacters.Find(CharacterID);

		if (Existing)
		{
			if (Existing->GetHardPtr() == Character || Existing->GetSoftPtr() == Character)
			{
				// Nothing to do, we already haave this same character!
				return false;
			}

			if (!bReplaceExisting)
			{
				UE_LOG(LogYap, Error, TEXT("Tried to register character ID <%s>, but this ID was already registered!"), *CharacterID.ToString());
				return false;
			}
		
			UE_LOG(LogYap, Display, TEXT("New character registration for ID <%s> is stomping an existing character."), *CharacterID.ToString());
		}

		if (!Character->GetClass()->ImplementsInterface(UYapCharacterInterface::StaticClass()))
		{
			UE_LOG(LogYap, Error, TEXT("Tried to register character ID <%s> with object <%s> but this object does not implement IYapCharacterInterface!"), *CharacterID.ToString(), *Character->GetName());
			return false;
		}

		return true;
	}

	/**
	 * 
	 * @param CharacterID ID of the character to unregister
	 */
	void UnregisterCharacter(FName CharacterID);

	/***
	 * Use this function to locate a character asset for a character ID.
	 * 
	 * @param CharacterID ID of the character to try and find
	 */
	TScriptInterface<IYapCharacterInterface> FindCharacter(FName CharacterID);

	/** Initiates a load and gives back a handle. Caller is responsible to hold onto the handle while they're using the character. */
	TSharedPtr<FStreamableHandle> RequestLoadAsync(FName CharacterID);
};

// ================================================================================================

UCLASS(DisplayName = "Yap Character Manager")
class YAP_API UYapCharacterManager_BPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**
	 * 
	 * @param CharacterID ID of the character to register
	 * @param CharacterObject Character object which must implement IYapCharacterInterface; this can be an actor in the world, a blueprint CDO, or an asset
	 * @param bReplaceExisting If left unset, attempting to register a different character to an ID will throw an error
	 */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character", meta = (WorldContext = "WorldContext"))
	static void RegisterCharacter(UObject* WorldContext, FName CharacterID, UObject* CharacterObject, bool bReplaceExisting = false);

	/**
	 * 
	 * @param WorldContext
	 * @param CharacterID 
	 * @param CharacterDefinition 
	 * @param bReplaceExisting 
	 */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character", meta = (WorldContext = "WorldContext"))
	static void RegisterCharacter_Dynamic(UObject* WorldContext, FName CharacterID, FInstancedStruct CharacterDefinition, bool bReplaceExisting = false);

	/**
	 * 
	 * @param CharacterID ID of the character to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character", meta = (WorldContext = "WorldContext"))
	static void UnregisterCharacter(UObject* WorldContext, FName CharacterID);
	
	static UYapCharacterManager& GetCharacterManager(UObject* WorldContext);
};
