// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetDefinitions/AssetDefinition_YapNode.h"

#include "Nodes/FlowNodeBlueprint.h"
#include "Yap/YapNodeBlueprint.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_YapNode"

FText UAssetDefinition_YapNode::GetAssetDisplayName() const
{
    return LOCTEXT("YapNodeAssetName", "Yap Node");
}

FLinearColor UAssetDefinition_YapNode::GetAssetColor() const
{
    return FColor(255, 196, 128);
}

TSoftClassPtr<UObject> UAssetDefinition_YapNode::GetAssetClass() const
{
    return UFlowNode_YapDialogue::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapNode::GetAssetCategories() const
{
    static const auto Categories = { FYapAssetCategoryPaths::Yap };

    return Categories;
}

FAssetSupportResponse UAssetDefinition_YapNode::CanLocalize(const FAssetData& InAsset) const
{
    return FAssetSupportResponse::Supported();
}




FText UAssetDefinition_YapNodeBlueprint::GetAssetDisplayName() const
{
    return LOCTEXT("YapNodeAssetName", "Yap Node Blueprint");
}

FLinearColor UAssetDefinition_YapNodeBlueprint::GetAssetColor() const
{
    return FColor(255, 196, 128);
}

TSoftClassPtr<UObject> UAssetDefinition_YapNodeBlueprint::GetAssetClass() const
{
    return UFlowNodeBlueprint::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapNodeBlueprint::GetAssetCategories() const
{
    static const auto Categories = { FYapAssetCategoryPaths::Yap };

    return Categories;
}

FAssetSupportResponse UAssetDefinition_YapNodeBlueprint::CanLocalize(const FAssetData& InAsset) const
{
    return FAssetSupportResponse::Supported();
}

#undef LOCTEXT_NAMESPACE
