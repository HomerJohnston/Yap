// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Globals/YapMoodTags.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Yap/YapNodeBlueprint.h"
#include "Yap/YapNodeConfig.h"
#include "UObject/UObjectIterator.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

TArray<UFlowNode_YapDialogue*> Yap::GetYapNodeTypes()
{
    TArray<UFlowNode_YapDialogue*> NodeTypes;

    // Get blueprint assets of Yap Nodes
	
    FARFilter AssetFilter;
				
    AssetFilter.ClassPaths.Add(FTopLevelAssetPath(UYapNodeBlueprint::StaticClass()->GetClassPathName()));
				
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetData> NodeAssets;
	
    AssetRegistryModule.Get().GetAssets(AssetFilter, NodeAssets);

    for (const FAssetData& AssetData : NodeAssets)
    {
        if (AssetData.GetSoftObjectPath().IsValid())
        {
            UYapNodeBlueprint* NodeBlueprint = Cast<UYapNodeBlueprint>(AssetData.GetAsset()); // TODO: careful, this does NOT handle redirects!
						
            if (NodeBlueprint && NodeBlueprint->IsA<UYapNodeBlueprint>() && NodeBlueprint->GeneratedClass)
            {
                UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(NodeBlueprint->GeneratedClass.GetDefaultObject());

                if (DialogueNode)
                {
                    NodeTypes.Add(DialogueNode);
                }
            }
        }
    }

    // Get C++ classes of Yap Nodes
	
    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (It->IsChildOf(UFlowNode_YapDialogue::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
        {
            UFlowNode_YapDialogue* DialogueNode = It->GetDefaultObject<UFlowNode_YapDialogue>();

            if (DialogueNode)
            {
                NodeTypes.Add(DialogueNode);
            }
        }
    }

    return NodeTypes;
}

FGameplayTagContainer Yap::GetMoodTagRoots()
{
    TArray<UFlowNode_YapDialogue*> NodeTypes = GetYapNodeTypes();

    TSet<FGameplayTag> RootTagsSet;
    
    for (UFlowNode_YapDialogue* NodeType : NodeTypes)
    {
        if (!NodeType)
        {
            continue;
        }
        
        const UYapNodeConfig& NodeConfig = NodeType->GetNodeConfig();

        if (NodeConfig.MoodTags.bDisableMoodTags)
        {
            continue;
        }
        
        if (NodeConfig.MoodTags.MoodTagsRoot.IsValid())
        {
            RootTagsSet.Add(NodeConfig.MoodTags.MoodTagsRoot);
        }
    }

    TArray<FGameplayTag> RootTagsArray(RootTagsSet.Array());

    RootTagsArray.Sort();
    
    FGameplayTagContainer MoodTagsRoots;

    for (const FGameplayTag& Root : RootTagsArray)
    {
        MoodTagsRoots.AddTag(Root);
    }

    return MoodTagsRoots;
}

const UYapNodeConfig& Yap::GetConfigUsingMoodRoot(const FGameplayTag& Root)
{
    TArray<UFlowNode_YapDialogue*> Nodes = GetYapNodeTypes();
    
    for (UFlowNode_YapDialogue* Type : Nodes)
    {
        const UYapNodeConfig& Config = Type->GetNodeConfig();

        if (Config.MoodTags.MoodTagsRoot == Root)
        {
            return Config;
        }
    }

    return *UYapNodeConfig::StaticClass()->GetDefaultObject<UYapNodeConfig>();
}


FGameplayTagContainer Yap::GetAllMoodTags()
{
    return GetAllMoodTagsUnder(GetMoodTagRoots());
}

FGameplayTagContainer Yap::GetAllMoodTagsUnder(const FGameplayTagContainer& Roots)
{

    FGameplayTagContainer AllMoodTags;
	
    for (const FGameplayTag& Root : Roots)
    {
        // Get all child tags
        AllMoodTags.AppendTags(UGameplayTagsManager::Get().RequestGameplayTagChildren(Root));
    }

    return AllMoodTags;
}

FGameplayTagContainer Yap::GetAllMoodTagsUnder(const FGameplayTag& Root)
{
    return UGameplayTagsManager::Get().RequestGameplayTagChildren(Root);
}
