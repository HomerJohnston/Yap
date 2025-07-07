// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/AssetFactories/AssetFactory_YapNodeConfig.h"

#include "Yap/YapNodeConfig.h"

UAssetFactory_YapNodeConfig::UAssetFactory_YapNodeConfig()
{
    SupportedClass = UYapNodeConfig::StaticClass();

    bCreateNew = true;

    bEditAfterNew = false;
}

UObject* UAssetFactory_YapNodeConfig::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
    return NewObject<UYapNodeConfig>(InParent, InClass, InName, Flags);
}
