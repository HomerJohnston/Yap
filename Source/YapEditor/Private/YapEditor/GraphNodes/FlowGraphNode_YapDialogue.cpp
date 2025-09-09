// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapDialogueNodeCommands.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};

	EventBus << YapEditor::Event::DialogueNode::Test;
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	FYapDialogueNodeCommands::Register();

	Commands = MakeShared<FUICommandList>();
	Commands->MapAction(FYapDialogueNodeCommands::Get().RecalculateFragmentTextLengths, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments));
	Commands->MapAction(FYapDialogueNodeCommands::Get().RegenerateNodeAudioID, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::RegenerateNodeAudioID));
	Commands->MapAction(FYapDialogueNodeCommands::Get().RegenerateFragmentAudioIDs, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::RegenerateAudioIDsOnAllFragments));
	Commands->MapAction(FYapDialogueNodeCommands::Get().AutoAssignAudio, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments));
	Commands->MapAction(FYapDialogueNodeCommands::Get().AutoAssignAudioOnAll, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllNodes));
	return SNew(SFlowGraphNode_YapDialogueWidget, this);
}

bool UFlowGraphNode_YapDialogue::ShowPaletteIconOnNode() const
{
	return true;
}

UFlowNode_YapDialogue* UFlowGraphNode_YapDialogue::GetYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(GetFlowNodeBase());
}

FLinearColor UFlowGraphNode_YapDialogue::GetNodeBodyTintColor() const
{
	return 0.5 * (YapColor::LightGray + YapColor::Gray);
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon();
}

void UFlowGraphNode_YapDialogue::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	UFlowNode_YapDialogue* Node = GetYapDialogueNode();

	TMap<FYapFragment*, TArray<FName>> FragmentOptionalPins;
	FragmentOptionalPins.Reserve(Node->GetNumFragments());

	// TODO transaction here
	
	bool bDirty = false;
	
	for (FYapFragment& Fragment : Node->GetFragmentsMutable())
	{
		if (Fragment.UsesStartPin())
		{
			FName PinName = Fragment.GetStartPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetStartPin();
				bDirty = true;
			}
		}
		
		if (Fragment.UsesEndPin())
		{
			FName PinName = Fragment.GetEndPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetEndPin();
				bDirty = true;
			}
		}
	}

	if (bDirty)
	{
		ReconstructNode();
	}
}

void UFlowGraphNode_YapDialogue::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	const FYapDialogueNodeCommands& DialogeNodeCommands = FYapDialogueNodeCommands::Get();

	{
		FToolMenuSection& Section = Menu->AddSection("Yap", LOCTEXT("Yap", "Yap"));
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.RecalculateFragmentTextLengths, Commands);
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.RegenerateNodeAudioID, Commands);
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.RegenerateFragmentAudioIDs, Commands);
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.AutoAssignAudio, Commands);
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.AutoAssignAudioOnAll, Commands);
	}
}

void UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments()
{
	FGraphPanelSelectionSet Nodes = FFlowGraphUtils::GetFlowGraphEditor(GetGraph())->GetSelectedNodes();
	
	FYapScopedTransaction T(FName("Default"), FText::Format(LOCTEXT("RecalculateTextLength_Command","Recalculate text length on {0} {0}|plural(one=node,other=nodes)"), Nodes.Num()), nullptr);

	for (UObject* Node : Nodes)
	{
		if (UFlowGraphNode_YapDialogue* DialogeGraphNode = Cast<UFlowGraphNode_YapDialogue>(Node))
		{
			UFlowNode_YapDialogue* DialogueNode = DialogeGraphNode->GetYapDialogueNode();
			DialogueNode->Modify();

			for (FYapFragment& Fragment : DialogueNode->GetFragmentsMutable())
			{
				// TODO
				// Fragment.GetBitMutable().RecacheSpeakingTime();
			}
		}
	}
}

void UFlowGraphNode_YapDialogue::RegenerateNodeAudioID()
{
	FGraphPanelSelectionSet Nodes = FFlowGraphUtils::GetFlowGraphEditor(GetGraph())->GetSelectedNodes();
	
	// TODO message dialog asking if you want to forcefully apply the pattern even if audio has been assigned
	FYapScopedTransaction Transaction(NAME_None, LOCTEXT("", ""), GetFlowAsset());

	for (UObject* Node : Nodes)
	{
		if (UFlowGraphNode_YapDialogue* DialogeGraphNode = Cast<UFlowGraphNode_YapDialogue>(Node))
		{
			DialogeGraphNode->RandomizeAudioID();
		}
	}
}

void UFlowGraphNode_YapDialogue::RegenerateAudioIDsOnAllFragments()
{
	FGraphPanelSelectionSet Nodes = FFlowGraphUtils::GetFlowGraphEditor(GetGraph())->GetSelectedNodes();
	
	// TODO message dialog asking if you want to forcefully apply the pattern even if audio has been assigned
	FYapScopedTransaction Transaction(NAME_None, LOCTEXT("", ""), GetFlowAsset());

	for (UObject* Node : Nodes)
	{
		if (UFlowGraphNode_YapDialogue* DialogeGraphNode = Cast<UFlowGraphNode_YapDialogue>(Node))
		{
			UFlowNode_YapDialogue* DialogueNode = DialogeGraphNode->GetYapDialogueNode();
			DialogueNode->Modify();
			DialogueNode->UpdateFragmentAudioIDs();
		}
	}
}

void UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllNodes()
{
	{
		FYapTransactions::BeginModify(INVTEXT("TODO"), GetFlowAsset());
		{
			TArray<UFlowGraphNode_YapDialogue*> Nodes;
			GetFlowAsset()->GetGraph()->GetNodesOfClass(Nodes);

			for (UFlowGraphNode_YapDialogue* Node : Nodes)
			{
				Node->Modify();
				Node->AutoAssignAudioOnAllFragments();
			}
		}
		
		FYapTransactions::EndModify();
	}
}

void UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments()
{
	UFlowAsset* Asset = GetFlowAsset();

	FString FlowAssetPath;
	UPackage* Package;
	
	if (Asset)
	{
		Package = Asset->GetPackage();

		if (Package)
		{
			FlowAssetPath = Package->GetName();
		}
	}

	// Yap does not prevent conflicts between audio IDs across the whole project; only within a single Flow asset. Conflicts are segregated by folders for different Flow assets.
	TMap<FString, TArray<FAssetData>> AudioAssetsByAudioID;
	GroupAudioAssetsByTags(AudioAssetsByAudioID);

	FYapTransactions::BeginModify(INVTEXT("TODO"), GetFlowAsset());

	const FDirectoryPath& FlowAssetsRoot = GetYapDialogueNode()->GetNodeConfig().Audio.FlowAssetsRootFolder;

	FString RootRoot = FlowAssetsRoot.Path;

	UE_LOG(LogTemp, Display, TEXT("%s"), *RootRoot);
	
	return;
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		int32 AudioIDLen = GetYapDialogueNode()->GetAudioIDRoot().Len();
		int32 FragmentIDLen = 3; // TODO magic number move this to project settings or some other constant
	
		FYapFragment& Fragment = GetYapDialogueNode()->Fragments[FragmentIndex];

		FNumberFormattingOptions Args;
		Args.UseGrouping = false;
		Args.MinimumIntegralDigits = 3; // TODO magic number move this to project settings or some other constant
		
		FString AudioID = GetYapDialogueNode()->GetAudioIDRoot() + "-" + (FText::AsNumber(FragmentIndex, &Args)).ToString(); // TODO build some way for users to define their own sequencing. Maybe move this to a default in the Broker?

		TArray<FAssetData>* CandidateAudioAssets = AudioAssetsByAudioID.Find(AudioID);

		if (CandidateAudioAssets)
		{
			bool bAssignedMature = false;
			bool bAssignedChildSafe = false;

			// All of these assets have the correct audio ID. We need to find ones that are in the correct subfolder for this dialogue node.
			for (const FAssetData& Data : *CandidateAudioAssets)
			{
				FRegexPattern ChildSafe(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}-Safe"), {AudioIDLen, FragmentIDLen})); // TODO documentation and configurable suffixes for child safe stuff or for mature stuff
				FRegexMatcher ChildSafeMatch(ChildSafe, Data.GetObjectPathString());

				if (ChildSafeMatch.FindNext())
				{
					if (bAssignedChildSafe)
					{
						UE_LOG(LogYap, Error, TEXT("Found multiple child-safe audio assets for %s"), *AudioID);
					}
					else
					{
						GetYapDialogueNode()->Modify();
						Fragment.ChildSafeBit.SetDialogueAudioAsset(Data.GetAsset());
						bAssignedChildSafe = true;
					}
				}
				else
				{
					if (bAssignedMature)
					{
						UE_LOG(LogYap, Error, TEXT("Found multiple mature audio assets for %s"), *AudioID);
					}
					else
					{
						GetYapDialogueNode()->Modify();
						Fragment.MatureBit.SetDialogueAudioAsset(Data.GetAsset());
						bAssignedMature = true;
					}
				}	
			}
		}
	}
	
	FYapTransactions::EndModify();
}

void UFlowGraphNode_YapDialogue::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	RandomizeAudioID();

	GetYapDialogueNode()->UpdateFragmentAudioIDs();
}

void UFlowGraphNode_YapDialogue::PostPasteNode()
{
	Super::PostPasteNode();

	GetYapDialogueNode()->DialogueID = NAME_None;
	GetYapDialogueNode()->InvalidateFragmentTags();
	
	RandomizeAudioID();
}

void UFlowGraphNode_YapDialogue::RandomizeAudioID()
{	
	if (UFlowNode_YapDialogue* DialogueNode = GetYapDialogueNode())// && TODO UYapProjectSettings::GetGenerateDialogueNodeTags()*/)
	{
		const UYapBroker& Broker = UYapBroker::GetInEditor();

		FString Tag = Broker.GenerateDialogueAudioID(DialogueNode);

		DialogueNode->AudioID = Tag;
	}
}

void UFlowGraphNode_YapDialogue::AddFragment(int32 InsertionIndex)
{
	if (GetYapDialogueNode()->Fragments.Num() >= 99)
	{
		Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("MaxFragmentLimitWarning_Title", "Maximum fragment limit reached!"), LOCTEXT("MaxFragmentLimitWarning_Description", "There is a limit of 99 fragments per node."));
		return;
	}

	UFlowNode_YapDialogue* Node = GetYapDialogueNode();
	
	FYapScopedTransaction Transaction(NAME_None, LOCTEXT("", ""), Node);
	Node->Modify();
	
	if (InsertionIndex == INDEX_NONE)
	{
		InsertionIndex = GetYapDialogueNode()->Fragments.Num();
	}

	FYapFragment NewFragment;
	uint8 CopyFragmentIndex = InsertionIndex == 0 ? InsertionIndex : InsertionIndex - 1;

	if (GetYapDialogueNode()->Fragments.IsValidIndex(CopyFragmentIndex))
	{
		const FYapFragment& PreviousFragment = GetYapDialogueNode()->GetFragmentByIndex(CopyFragmentIndex);
		NewFragment.SetSpeaker(PreviousFragment.GetSpeakerTag());
		NewFragment.SetMoodTag(PreviousFragment.GetMoodTag());
	}

	Node->InsertFragment(NewFragment, InsertionIndex);
}

void UFlowGraphNode_YapDialogue::GatherAllAudioAssets(TArray<FAssetData>& AllAudioAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Discover all audio assets 
	const UYapNodeConfig& Config = GetYapDialogueNode()->GetNodeConfig();

	const FDirectoryPath& FlowAssetsRoot = Config.GetFlowAssetsRootFolder();
	
	const TArray<TSoftClassPtr<UObject>>& AudioAssetClasses = UYapProjectSettings::GetAudioAssetClasses();
	TArray<FTopLevelAssetPath> AudioAssetClassPaths;

	auto AlgoTransform = [] (const TSoftClassPtr<UObject>& Obj) { return Obj->GetClassPathName(); };
	Algo::Transform(AudioAssetClasses, AudioAssetClassPaths, AlgoTransform);
	
	FARFilter Filter;
	Filter.ClassPaths = AudioAssetClassPaths;

	AssetRegistry.GetAssets(Filter, AllAudioAssets, true);
}

void UFlowGraphNode_YapDialogue::GroupAudioAssetsByTags(TMap<FString, TArray<FAssetData>>& AudioAssetsByAudioID)
{
	TArray<FAssetData> AllAudioAssets;
	
	GatherAllAudioAssets(AllAudioAssets);
	
	int32 AudioIDLen = GetYapDialogueNode()->GetAudioIDRoot().Len();
	int32 FragmentIDLen = 3; // TODO magic number move this to project settings or some other constant
	
	// Matches AAA-555, and allows for the start or end to be 'end of string' or a 'non-alphanumeric' character
	const FRegexPattern Regex(FString::Format(TEXT("(^| |[^a-zA-Z\\d\\s:])[a-zA-Z]{{0}}-\\d{{1}}([^a-zA-Z\\d\\s:]| |$)"), {AudioIDLen, FragmentIDLen} ));
	
	// Matches AAA-555 specifically
	const FRegexPattern RegexActual(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}"), {AudioIDLen, FragmentIDLen}));
	
	for (const FAssetData& AssetData : AllAudioAssets)
	{
		FString ObjectPathString = AssetData.GetObjectPathString();
		
		FRegexMatcher Matcher(Regex, *ObjectPathString);
		
		if (Matcher.FindNext())
		{
			FRegexMatcher ID(RegexActual, Matcher.GetCaptureGroup(0));

			if (ID.FindNext())
			{
				FString AudioID(ID.GetCaptureGroup(0));

				TArray<FAssetData>& AudioAssetsWithTag = AudioAssetsByAudioID.FindOrAdd(AudioID);
				AudioAssetsWithTag.Add(AssetData);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
