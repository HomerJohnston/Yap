// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "FlowGraphNode_YapBase.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UFlowNode_YapDialogue;
class UAkAudioEvent;
class SGraphNode;
class FUICommandList;
struct FYapFragment;

#define LOCTEXT_NAMESPACE "YapEditor"

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode_YapBase
{
	// TODO should this just be a friend class of my dialogue node class?
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();
	
protected:
	TSharedPtr<FUICommandList> Commands;
	
public:	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;

	UFlowNode_YapDialogue* GetYapDialogueNode() const;

	FLinearColor GetNodeBodyTintColor() const override;

	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

public:
	void PreSave(FObjectPreSaveContext SaveContext) override;
	
	void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	void RecalculateTextOnAllFragments();

	void RegenerateNodeAudioID();

	void RegenerateAudioIDsOnAllFragments();

	void AutoAssignAudioOnAllNodes();
	
	void AutoAssignAudioOnAllFragments();
	
	void PostPlacedNewNode() override;
	
	void PostPasteNode() override;
	
	void RandomizeAudioID();
	
	void AddFragment(int32 InsertionIndex = INDEX_NONE);

protected:
	void GatherAllAudioAssets(TArray<FAssetData>& AllAudioAssets);

	void GroupAudioAssetsByTags(TMap<FString, TArray<FAssetData>>& AudioAssetsByAudioID);
};

#undef LOCTEXT_NAMESPACE
