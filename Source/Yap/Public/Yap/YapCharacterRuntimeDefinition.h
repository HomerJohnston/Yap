// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Math/Color.h"
#include "Internationalization/Text.h"

#include "YapCharacterRuntimeDefinition.generated.h"

class UYapCharacterAsset;
class UTexture2D;

USTRUCT(BlueprintType)
struct FYapCharacterRuntimeDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Default", BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Default", BlueprintReadWrite)
	UTexture2D* DefaultPortrait;

	UPROPERTY(EditAnywhere, Category = "Default", BlueprintReadWrite)
	FLinearColor Color;

	void InitializeCharacter(UYapCharacterAsset* Character) const;
};
