// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "FlowEditorCommands.h"
#include "FlowEditorStyle.h"
#include "Graph/FlowGraphSchema_Actions.h"

#include "Nodes/FlowNode.h"

#include "Misc/ConfigCacheIni.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

class YAPEDITOR_API FYapDialogueNodeCommands : public TCommands<FYapDialogueNodeCommands>
{
public:
	FYapDialogueNodeCommands();

	TSharedPtr<FUICommandInfo> RecalculateFragmentTextLengths;
	TSharedPtr<FUICommandInfo> RegenerateNodeAudioID;
	TSharedPtr<FUICommandInfo> RegenerateFragmentAudioIDs;
	TSharedPtr<FUICommandInfo> AutoAssignAudio;
	TSharedPtr<FUICommandInfo> AutoAssignAudioOnAll;

	void RegisterCommands() override;
};

#undef LOCTEXT_NAMESPACE