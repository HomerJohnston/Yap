// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetDefinitions/AssetDefinition_YapNodeConfig.h"

#include "Yap/YapNodeConfig.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_YapNodeConfig"

FText UAssetDefinition_YapNodeConfig::GetAssetDisplayName() const
{
	return LOCTEXT("YapNodeConfigAssetName", "Yap Node Config");
}

FLinearColor UAssetDefinition_YapNodeConfig::GetAssetColor() const
{
	return YapColor::DialogueNodeConfigColor;
}

TSoftClassPtr<UObject> UAssetDefinition_YapNodeConfig::GetAssetClass() const
{
	return UYapNodeConfig::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapNodeConfig::GetAssetCategories() const
{
	static const auto Categories = { FYapAssetCategoryPaths::Yap };

	return Categories;
}

FAssetSupportResponse UAssetDefinition_YapNodeConfig::CanLocalize(const FAssetData& InAsset) const
{
	return FAssetSupportResponse::Supported();
}

EAssetCommandResult UAssetDefinition_YapNodeConfig::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	return Super::OpenAssets(OpenArgs);
}

EAssetCommandResult UAssetDefinition_YapNodeConfig::PerformAssetDiff(const FAssetDiffArgs& DiffArgs) const
{
	return Super::PerformAssetDiff(DiffArgs);
}

#undef LOCTEXT_NAMESPACE
