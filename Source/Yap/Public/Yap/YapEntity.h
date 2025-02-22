// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

#include "YapEntity.generated.h"

UCLASS(BlueprintType)
class YAP_API UYapEntity : public UObject
{
	GENERATED_BODY()
public:
	UYapEntity();
	
protected:
	/** Human-readable name of this character or entity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Name")
	FText EntityName;

	/** Color for display. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Color")
	FLinearColor EntityColor;
	
	/** Used to find this actor in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag IdentityTag;

public:
	FText GetEntityName() const { return EntityName; }

	FLinearColor GetEntityColor() const { return EntityColor; }

	FGameplayTag GetIdentityTag() const { return IdentityTag; }
};
