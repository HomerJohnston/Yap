// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorModule.h"

#include "YapEditor/AssetFactory_YapCharacter.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapCharacterThumbnailRenderer.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/Customizations/DetailCustomization_YapProjectSettings.h"
#include "YapEditor/Customizations/DetailCustomization_YapCharacter.h"
#include "YapEditor/Customizations/PropertyCustomization_YapGroupSettings.h"

#define LOCTEXT_NAMESPACE "YapEditor"

#define REGISTER_ASSET_TYPE_ACTION(NAME) AssetTypeActions.Add(MakeShared<NAME>())
#define REGISTER_DETAIL_CUSTOMIZATION(CLASSNAME, CUSTOMIZATIONNAME) DetailCustomizations.Append({{ CLASSNAME::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&CUSTOMIZATIONNAME::MakeInstance)}})
#define REGISTER_PROPERTY_CUSTOMIZATION(CLASSNAME, CUSTOMIZATIONNAME) PropertyCustomizations.Append({{ *CLASSNAME::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&CUSTOMIZATIONNAME::MakeInstance) }});
#define REGISTER_THUMBNAIL_RENDERER(CLASSNAME, THUMBNAILRENDERERNAME) ClassThumbnailRenderers.Append({{ CLASSNAME::StaticClass(), THUMBNAILRENDERERNAME::StaticClass() }});

void FYapEditorModule::StartupModule()
{
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };

	REGISTER_ASSET_TYPE_ACTION(FAssetTypeActions_FlowYapCharacter);

	REGISTER_DETAIL_CUSTOMIZATION(UYapProjectSettings, FDetailCustomization_YapProjectSettings);

	REGISTER_DETAIL_CUSTOMIZATION(UYapCharacter, FDetailCustomization_YapCharacter);

	REGISTER_PROPERTY_CUSTOMIZATION(FYapGroupSettings, FPropertyCustomization_YapGroupSettings);

	REGISTER_THUMBNAIL_RENDERER(UYapCharacter, UYapCharacterThumbnailRenderer);


	
	
	StartupModuleBase();

	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Get();
}

void FYapEditorModule::ShutdownModule()
{
	ShutdownModuleBase();
}

IMPLEMENT_MODULE(FYapEditorModule, FlowYapEditor)

#undef LOCTEXT_NAMESPACE