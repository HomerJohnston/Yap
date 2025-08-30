// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagFilterHelper.h"
#include "YapCharacterRuntimeDefinition.h"
#include "GameFramework/Actor.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
	#include "InstancedStruct.h"
#else
	#include "StructUtils/InstancedStruct.h"
#endif

#include "YapCharacterComponent.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

struct FYapCharacterRuntimeDefinition;

USTRUCT()
struct FYapCharacterIdentity
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FPropertyCustomization_YapCharacterIdentity;
	friend class UYapCharacterComponent;
#endif
	
private:
	UPROPERTY(EditAnywhere, Category = "Default")
	FName Identity_Name;
	
	UPROPERTY(EditAnywhere, Category = "Default")
	FGameplayTag Identity_Tag;

	UPROPERTY(VisibleAnywhere, Transient)
	FName Identity_Auto;
	
public:
	FName Get(UObject* Owner)
	{
		if (Identity_Name != NAME_None)
		{
			return Identity_Name;
		}
		
		if (Identity_Tag.IsValid())
		{
			return Identity_Tag.GetTagName();
		}

		if (Identity_Auto == NAME_None)
		{
			FGuid GUID = FGuid::NewGuid();
			
			Identity_Auto = FName(Owner->GetFName().ToString() + "_" + GUID.ToString()); // TODO I need to investigate if this is stable long-term (e.g. after saving/destroying lots of actors or 
		}

		return Identity_Auto;
	}

	bool UsesProjectCharacter()
	{
		return Identity_Tag.IsValid();
	}
};

UCLASS(meta=(BlueprintSpawnableComponent), HideCategories = ("Sockets", "ComponentTick", "Cooking", "Events", "Asset UserData", "Navigation"))
class YAP_API UYapCharacterComponent : public UActorComponent, public FGameplayTagFilterHelper<UYapCharacterComponent>
{
	GENERATED_BODY()

	// CONSTRUCTION
public:
	UYapCharacterComponent();

	// SETTINGS
protected:
	/** Identity of this character. Use this for project hero characters which are scripted in a Flow Graph. Create character mappings in Yap's Project Settings panel. */
    UPROPERTY(EditAnywhere, Category = "Yap Character")
	FYapCharacterIdentity Identity;

	/** If set, the character will not be automatically registered to Yap during BeginPlay. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Yap Character")
	bool bPreventAutoRegisterComponent = false;

	/** If set, the character will not be automatically registered to Yap during BeginPlay. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Yap Character")
	bool bPreventAutoRegisterCharacterDefinition = false;

	/** When this character speaks, this data will be used for its name, portrait info, etc.; this should normally only be used for dynamic characters (NPCs) and not for project characters. */
	UPROPERTY(EditAnywhere, Category = "Yap Character")
	FYapCharacterRuntimeDefinition CharacterDefinition;

	// STATE
protected:
	
	UPROPERTY(Transient)
	bool bComponentRegistered = false;

	UPROPERTY(Transient)
	bool bDefinitionRegistered = false;

	// PUBLIC API
public:
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void RegisterCharacterDefinition();

	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void UnregisterCharacterDefinition();

	const FName GetCharacterID() { return Identity.Get(GetOwner()); }

	// OTHER API
protected:
	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
	static const FGameplayTag& GetIdentityRootTag();
};

#undef LOCTEXT_NAMESPACE
