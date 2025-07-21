// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "YapCharacterComponent.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

UCLASS(meta=(BlueprintSpawnableComponent))
class YAP_API UYapCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Default")
	FGameplayTag Identity;

public:
	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	const FGameplayTag& GetCharacterTag() { return Identity; }
};

#undef LOCTEXT_NAMESPACE