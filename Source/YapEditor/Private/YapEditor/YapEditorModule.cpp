// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorModule.h"

#include "ToolMenus.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapCharacterComponent.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapAudioAssetPinFactory.h"
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
	
	// create your factory and shared pointer to it.
	TSharedPtr<FYapAudioAssetPinFactory> Factory = MakeShareable(new FYapAudioAssetPinFactory());
	// and now register it.
	FEdGraphUtilities::RegisterVisualPinFactory(Factory);
	
	StartupModuleBase();
	// FGPGEditorModuleBase implementation END
	
	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Initialize();
	FYapEditorStyle::ReloadTextures();

	FYapButtonCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
	
	PluginCommands->MapAction(
		FYapButtonCommands::Get().OpenYapProjectSettingsAction,
		FExecuteAction::CreateRaw(this, &FYapEditorModule::OpenYapProjectSettings),
		FCanExecuteAction());

	YapComboActions = MakeShareable(new FUICommandList);

	YapComboActions->MapAction(
		FYapButtonCommands::Get().RebuildMoodTags,
		FExecuteAction::CreateRaw(this, &FYapEditorModule::FlushMoodTagIcons),
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

void FYapEditorModule::FlushMoodTagIcons()
{
	UYapNodeConfig::FlushMoodTagIconBrushes();

	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	
	Yap::EditorFuncs::PostNotificationInfo_Info(LOCTEXT("RefreshMoodTagsNotification_Title", "Mood Tag Icons"), LOCTEXT("RefreshMoodTagsNotification_Description", "Refresh complete. You may need to reopen or refresh any open assets."));
}

void FYapEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Yap");
	
	FToolMenuEntry& Entry1 = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FYapButtonCommands::Get().OpenYapProjectSettingsAction));
	Entry1.StyleNameOverride = FName("Toolbar.BackplateLeft");
	Entry1.Icon = YapIcons.FunctionButton;
	
	FUIAction YapOptionsMenuAction;
	
	FNewToolMenuChoice YapOptionsComboEntry = FOnGetContent::CreateStatic(&GenerateYapComboMenuContent, YapComboActions.ToSharedRef());
	
	FToolMenuEntry& Entry2 = Section.AddEntry(FToolMenuEntry::InitComboButton("YapComboButton", YapOptionsMenuAction, YapOptionsComboEntry, INVTEXT(""), LOCTEXT("YapComboButton_ToolTipText", "Yap Functions")));
	Entry2.StyleNameOverride = FName("Toolbar.BackplateRightCombo");
	
	Entry1.SetCommandList(PluginCommands);
}

TSharedRef<SWidget> FYapEditorModule::GenerateYapComboMenuContent(TSharedRef<FUICommandList> InCommandList)
{
	static const FName MenuName("Yap.EditorCommands");

	if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu(MenuName);

		struct FLocal
		{
			static void AddMenuEntry(FToolMenuSection& Section)
			{
				
			}
		};
		
		{
			FToolMenuSection& Section = Menu->AddSection("YapFunctions", INVTEXT("Functions"));
			Section.AddMenuEntry(FYapButtonCommands::Get().RebuildMoodTags);
		}
	}

	
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
	
	FToolMenuContext MenuContext(InCommandList, MenuExtender);
	
	return UToolMenus::Get()->GenerateWidget(MenuName, MenuContext);
}

IMPLEMENT_MODULE(FYapEditorModule, YapEditor)

#undef LOCTEXT_NAMESPACE