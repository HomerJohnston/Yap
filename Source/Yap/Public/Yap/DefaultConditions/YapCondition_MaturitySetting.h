// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Yap/YapCondition.h"

#include "YapCondition_MaturitySetting.generated.h"

enum class EYapMaturitySetting : uint8;

UCLASS(DisplayName = "Maturity Setting")
class YAP_API UYapCondition_MaturitySetting : public UYapCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Default")
	EYapMaturitySetting RequiredSetting;

public:
	bool EvaluateCondition_Implementation() const override;

#if WITH_EDITOR
	FLinearColor GetColor_Implementation() const override;
	
	FText GetTitle_Implementation() const override;
#endif
};