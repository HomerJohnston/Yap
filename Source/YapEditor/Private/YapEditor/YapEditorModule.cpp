// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorModule.h"

#include "YapEditor/AssetFactory_YapCharacter.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapCharacterThumbnailRenderer.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/Customizations/DetailCustomization_YapProjectSettings.h"
#include "YapEditor/Customizations/DetailCustomization_YapCharacter.h"
#include "YapEditor/Customizations/PropertyCustomization_YapGroupSettings.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FYapEditorModule::StartupModule()
{
	// FGPGEditorModuleBase implementation START
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };

	REGISTER_ASSET_TYPE_ACTION(FAssetTypeActions_FlowYapCharacter);

	REGISTER_DETAIL_CUSTOMIZATION(UYapProjectSettings, FDetailCustomization_YapProjectSettings);
	REGISTER_DETAIL_CUSTOMIZATION(UYapCharacterAsset, FDetailCustomization_YapCharacter);

	REGISTER_PROPERTY_CUSTOMIZATION(FYapTypeGroupSettings, FPropertyCustomization_YapGroupSettings);

	REGISTER_THUMBNAIL_RENDERER(UYapCharacterAsset, UYapCharacterThumbnailRenderer);
	
	StartupModuleBase();
	// FGPGEditorModuleBase implementation END
	
	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Get();
}

void FYapEditorModule::ShutdownModule()
{
	// FGPGEditorModuleBase implementation START
	ShutdownModuleBase();
	// FGPGEditorModuleBase implementation END
}

IMPLEMENT_MODULE(FYapEditorModule, YapEditor)

#undef LOCTEXT_NAMESPACE