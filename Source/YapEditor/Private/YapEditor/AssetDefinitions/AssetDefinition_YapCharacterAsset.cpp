// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetDefinitions/AssetDefinition_YapCharacterAsset.h"

#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorModule.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FText UAssetDefinition_YapCharacterAsset::GetAssetDisplayName() const
{
    return LOCTEXT("YapCharacterAssetName", "Yap Character");
}

FLinearColor UAssetDefinition_YapCharacterAsset::GetAssetColor() const
{
    return YapColor::CharacterAssetColor;
}

TSoftClassPtr<UObject> UAssetDefinition_YapCharacterAsset::GetAssetClass() const
{
    return UYapCharacterAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapCharacterAsset::GetAssetCategories() const
{
    static const auto Categories = { FYapAssetCategoryPaths::Yap };

    return Categories;
}

FAssetSupportResponse UAssetDefinition_YapCharacterAsset::CanLocalize(const FAssetData& InAsset) const
{
    return FAssetSupportResponse::Supported();
}

EAssetCommandResult UAssetDefinition_YapCharacterAsset::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
    return Super::OpenAssets(OpenArgs);
}

EAssetCommandResult UAssetDefinition_YapCharacterAsset::PerformAssetDiff(const FAssetDiffArgs& DiffArgs) const
{
    return Super::PerformAssetDiff(DiffArgs);
}

#undef LOCTEXT_NAMESPACE
