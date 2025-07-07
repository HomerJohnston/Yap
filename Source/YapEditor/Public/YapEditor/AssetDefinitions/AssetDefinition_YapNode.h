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


class YAPEDITOR_API FAssetTypeActions_YapNodeBlueprint : public FAssetTypeActions_Blueprint
{
public:
    virtual FText GetName() const override;
    //virtual uint32 GetCategories() override;
    virtual FColor GetTypeColor() const override { return FColor(255, 196, 128); }

    virtual UClass* GetSupportedClass() const override;

protected:
    // FAssetTypeActions_Blueprint
    virtual bool CanCreateNewDerivedBlueprint() const override { return false; }
    // --
};
