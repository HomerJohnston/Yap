// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapButtonCommands.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FYapButtonCommands::RegisterCommands()
{
    UI_COMMAND(PluginAction, "YapEditor", "Open Yap Project Settings", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE