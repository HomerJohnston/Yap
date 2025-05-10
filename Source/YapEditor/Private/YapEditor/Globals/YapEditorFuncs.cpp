// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Globals/YapEditorFuncs.h"

#include "FileHelpers.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "ISettingsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "UObject/SavePackage.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Yap/YapProjectSettings.h"

void Yap::EditorFuncs::OpenDeveloperSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Yap", FName(UYapDeveloperSettings::StaticClass()->GetName()));
	}
}

void Yap::EditorFuncs::OpenProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Yap", FName(UYapProjectSettings::StaticClass()->GetName()));
	}
}

void Yap::EditorFuncs::PostNotificationInfo_Warning(FText Title, FText Description, float Duration)
{
	Yap::Editor::PostNotificationInfo_Warning(Title, Description, Duration);
}

bool Yap::EditorFuncs::SaveAsset(UObject* Asset)
{
	UPackage* Package = Asset->GetPackage();
	const FString PackageName = Package->GetName();
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	
	// This is specified just for example
	{
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
	}

	const bool bSucceeded = UEditorLoadingAndSavingUtils::SavePackages( {Package}, false );

	if (!bSucceeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Package '%s' wasn't saved!"), *PackageName)
		return false;
	}

	return true;
}
