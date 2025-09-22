﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GPGEditorModuleBase.h"

class IAssetTypeActions;
class IAssetTools;
class FUICommandList;

#define LOCTEXT_NAMESPACE "YapEditor"

struct YAPEDITOR_API FYapAssetCategoryPaths : EAssetCategoryPaths
{
    static FAssetCategoryPath Yap;
};

class FYapEditorModule : public IModuleInterface, public FGPGEditorModuleBase
{
public:
	static EAssetTypeCategories::Type YapAssetCategory;
    
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    //

public:
    void OpenYapProjectSettings();

    void FlushMoodTagIcons();
    
private:
    void RegisterMenus();

    static TSharedRef< SWidget > GenerateYapComboMenuContent( TSharedRef<FUICommandList> InCommandList );
    
private:
    TSharedPtr<FUICommandList> PluginCommands;

    TSharedPtr<FUICommandList> YapComboActions;
};

#undef LOCTEXT_NAMESPACE