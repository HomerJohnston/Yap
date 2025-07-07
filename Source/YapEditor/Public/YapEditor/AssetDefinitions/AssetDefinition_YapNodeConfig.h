// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "AssetDefinitionDefault.h"

#include "AssetDefinition_YapNodeConfig.generated.h"

/**
 * 
 */
UCLASS()
class YAPEDITOR_API UAssetDefinition_YapNodeConfig : public UAssetDefinitionDefault
{
    GENERATED_BODY()

public:
    FText GetAssetDisplayName() const override;
    FLinearColor GetAssetColor() const override;
    TSoftClassPtr<UObject> GetAssetClass() const override;
    TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
    FAssetSupportResponse CanLocalize(const FAssetData& InAsset) const override;

    EAssetCommandResult OpenAssets(const FAssetOpenArgs& OpenArgs) const override;
    EAssetCommandResult PerformAssetDiff(const FAssetDiffArgs& DiffArgs) const override;
};
