// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetDefinitions//AssetDefinition_YapBroker.h"

#include "Yap/YapBroker.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorModule.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FText UAssetDefinition_YapBroker::GetAssetDisplayName() const
{
	return LOCTEXT("YapBrokerName", "Yap Broker");
}

FLinearColor UAssetDefinition_YapBroker::GetAssetColor() const
{
    return YapColor::BrokerBlueprintColor;
}

TSoftClassPtr<UObject> UAssetDefinition_YapBroker::GetAssetClass() const
{
	return UYapBroker::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapBroker::GetAssetCategories() const
{
	static const auto Categories = { FYapAssetCategoryPaths::Yap };

	return Categories;
}

FAssetSupportResponse UAssetDefinition_YapBroker::CanLocalize(const FAssetData& InAsset) const
{
    return FAssetSupportResponse::Supported();
}

EAssetCommandResult UAssetDefinition_YapBroker::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	return Super::OpenAssets(OpenArgs);
}

EAssetCommandResult UAssetDefinition_YapBroker::PerformAssetDiff(const FAssetDiffArgs& DiffArgs) const
{
	return Super::PerformAssetDiff(DiffArgs);
}

#undef LOCTEXT_NAMESPACE