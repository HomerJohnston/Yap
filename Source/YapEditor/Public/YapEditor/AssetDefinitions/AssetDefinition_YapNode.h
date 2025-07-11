// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

#include "AssetDefinition_YapNode.generated.h"

/**
 * 
 */
UCLASS()
class YAPEDITOR_API UAssetDefinition_YapNode : public UAssetDefinitionDefault
{
    GENERATED_BODY()

public:
    FText GetAssetDisplayName() const override;
    FLinearColor GetAssetColor() const override;
    TSoftClassPtr<UObject> GetAssetClass() const override;
    TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
    FAssetSupportResponse CanLocalize(const FAssetData& InAsset) const override;
};



// Note: this doesn't do anything. The FAssetTypeActions_FlowNodeBlueprint overrides it. Eventually if FAssetTypeActions_YapNodeBlueprint is updated to use UAssetDefinition then this should suddenly start working.
// All it really does is give Yap Node assets an asset title of "Yap Node Blueprint" instead of "Flow Node Blueprint"

/**
 * 
 */
UCLASS()
class YAPEDITOR_API UAssetDefinition_YapNodeBlueprint : public UAssetDefinitionDefault
{
    GENERATED_BODY()

public:
    FText GetAssetDisplayName() const override;
    FLinearColor GetAssetColor() const override;
    TSoftClassPtr<UObject> GetAssetClass() const override;
    TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
    FAssetSupportResponse CanLocalize(const FAssetData& InAsset) const override;
};
