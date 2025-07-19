// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"

#include "AssetFactory_YapCharacter.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// TODO: update this to new asset definitions system, use conditional compilation for 5.5+
UCLASS()
class UAssetFactory_YapCharacter : public UFactory
{
	GENERATED_BODY()
public:
	UAssetFactory_YapCharacter();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

// ================================================================================================
class FAssetTypeActions_FlowYapCharacter : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};

#undef LOCTEXT_NAMESPACE