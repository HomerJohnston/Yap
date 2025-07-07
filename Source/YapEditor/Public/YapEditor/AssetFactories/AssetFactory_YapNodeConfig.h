// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Factories/Factory.h"

#include "AssetFactory_YapNodeConfig.generated.h"

UCLASS(HideCategories = (Object))
class UAssetFactory_YapNodeConfig : public UFactory
{
    GENERATED_BODY()
    
public:
    UAssetFactory_YapNodeConfig();
    
    UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
};