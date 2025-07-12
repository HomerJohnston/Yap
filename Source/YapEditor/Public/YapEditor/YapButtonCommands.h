// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "YapEditorStyle.h"

class FYapButtonCommands : public TCommands<FYapButtonCommands>
{
public:

    FYapButtonCommands()
        : TCommands<FYapButtonCommands>(TEXT("YapEditor"), NSLOCTEXT("Contexts", "YapEditor", "Yap Project Settings"), NAME_None, FYapEditorStyle::GetStyleSetName())
    {
        
    }

    void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> PluginAction;
};
