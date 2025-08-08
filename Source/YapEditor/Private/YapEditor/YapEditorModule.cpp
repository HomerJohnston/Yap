// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorModule.h"

#include "Yap/YapCharacterAsset.h"
#include "Yap/YapCharacterComponent.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapButtonCommands.h"
#include "YapEditor/AssetThumbnailRenderers/YapCharacterThumbnailRenderer.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/Customizations/DetailCustomization_YapProjectSettings.h"
#include "YapEditor/Customizations/DetailCustomization_YapCharacterAsset.h"
#include "YapEditor/Customizations/PropertyCustomization_YapCharacterDefinition.h"
#include "YapEditor/Customizations/PropertyCustomization_YapNodeConfigGroup_MoodTags.h"
#include "YapEditor/Customizations/PropertyCustomization_YapPortraitList.h"
#include "YapEditor/Customizations/PropertyCustomization_YapCharacterIdentity.h"
#include "YapEditor/Globals/YapEditorFuncs.h"

#define LOCTEXT_NAMESPACE "YapEditor"

EAssetTypeCategories::Type FYapEditorModule::YapAssetCategory = static_cast<EAssetTypeCategories::Type>(0);
FAssetCategoryPath FYapAssetCategoryPaths::Yap(LOCTEXT("Yap", "Yap"));

void FYapEditorModule::StartupModule()
{
	// FGPGEditorModuleBase implementation START
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };

	REGISTER_DETAIL_CUSTOMIZATION(UYapProjectSettings, FDetailCustomization_YapProjectSettings);
	REGISTER_DETAIL_CUSTOMIZATION(UYapCharacterAsset, FDetailCustomization_YapCharacterAsset);

	REGISTER_PROPERTY_CUSTOMIZATION(FYapCharacterStaticDefinition, FPropertyCustomization_YapCharacterDefinition);
	REGISTER_PROPERTY_CUSTOMIZATION(FYapNodeConfigGroup_MoodTags, FPropertyCustomization_YapNodeConfigGroup_MoodTags);
	REGISTER_PROPERTY_CUSTOMIZATION(FYapPortraitList, FPropertyCustomization_YapPortraitList);
	REGISTER_PROPERTY_CUSTOMIZATION(FYapCharacterIdentity, FPropertyCustomization_YapCharacterIdentity);

	REGISTER_THUMBNAIL_RENDERER(UYapCharacterAsset, UYapCharacterThumbnailRenderer);
	
	StartupModuleBase();
	// FGPGEditorModuleBase implementation END
	
	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Initialize();
	FYapEditorStyle::ReloadTextures();

	FYapButtonCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
	
	PluginCommands->MapAction(
		FYapButtonCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FYapEditorModule::OpenYapProjectSettings),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FYapEditorModule::RegisterMenus));
}

void FYapEditorModule::ShutdownModule()
{
	// FGPGEditorModuleBase implementation START
	ShutdownModuleBase();
	// FGPGEditorModuleBase implementation END
}

void FYapEditorModule::OpenYapProjectSettings()
{
	Yap::EditorFuncs::OpenProjectSettings();
}

void FYapEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
	FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FYapButtonCommands::Get().PluginAction));

	Entry.SetCommandList(PluginCommands);
}

IMPLEMENT_MODULE(FYapEditorModule, YapEditor)

#undef LOCTEXT_NAMESPACE