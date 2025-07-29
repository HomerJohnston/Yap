// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapCharacterRuntimeDefinition.generated.h"

class UYapCharacterAsset;

USTRUCT(BlueprintType)
struct FYapCharacterRuntimeDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* DefaultPortrait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;

	void InitializeCharacter(UYapCharacterAsset* Character) const;
};
