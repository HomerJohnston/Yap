// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapAudioIDFormat.generated.h"

USTRUCT(BlueprintType)
struct FYapAudioIDFormat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Default")
	FString NodeIDFormat = "***";

	UPROPERTY(EditAnywhere, Category = "Default")
	FString Separator = "-";

	UPROPERTY(EditAnywhere, Category = "Default")
	int32 MinDigits = 3;

	UPROPERTY(EditAnywhere, Category = "Default")
	int32 DefaultIncrement = 10;
	
	UPROPERTY(EditAnywhere, Category = "Default")
	FString Format = "{NodeID}{Separator}{FragmentIndex}";
	
	FString ParseFragmentID(int32 FragmentID) const;
};
