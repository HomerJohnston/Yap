// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagFilterHelper.h"

#include "YapCharacterComponent.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

USTRUCT()
struct FYapCharacterIdentity
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FPropertyCustomization_YapCharacterIdentity;
#endif
	
private:
	UPROPERTY(EditAnywhere)
	FGameplayTag Identity_Tag;
	
	UPROPERTY(EditAnywhere)
	FName Identity_Name;

public:
	FName Get() const
	{
		if (Identity_Tag.IsValid())
		{
			return Identity_Tag.GetTagName();
		}

		return Identity_Name;
	}
};

UCLASS(meta=(BlueprintSpawnableComponent))
class YAP_API UYapCharacterComponent : public UActorComponent, public FGameplayTagFilterHelper<UYapCharacterComponent>
{
	GENERATED_BODY()

public:
	UYapCharacterComponent();
	
protected:
	/** Identity of this character. Use this for project hero characters which are scripted in a Flow Graph. Create character mappings in Yap's Project Settings panel. */
    UPROPERTY(EditAnywhere, Category = "Yap Character")
	FYapCharacterIdentity Identity;
	
public:
	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	const FName GetCharacterIdentity() { return Identity.Get(); }
	
	static const FGameplayTag& GetIdentityRootTag();
};

#undef LOCTEXT_NAMESPACE
