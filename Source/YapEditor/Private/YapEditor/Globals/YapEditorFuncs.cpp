// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Globals/YapEditorFuncs.h"

#include "FileHelpers.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "ISettingsModule.h"
#include "SGameplayTagPicker.h"
#include "UObject/SavePackage.h"
#include "Yap/YapProjectSettings.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void Yap::EditorFuncs::OpenDeveloperSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Yap", FName(UYapDeveloperSettings::StaticClass()->GetName()));
	}
}

void Yap::EditorFuncs::OpenGameplayTagsEditor()
{
	OpenGameplayTagsEditor(FGameplayTag::EmptyTag);
}

void Yap::EditorFuncs::OpenGameplayTagsEditor(const FGameplayTag& Root)
{
	FGameplayTagContainer Container;

	if (Root.IsValid())
	{
		Container.AddTag(Root);
	}

	OpenGameplayTagsEditor(Container);
}

void Yap::EditorFuncs::OpenGameplayTagsEditor(const FGameplayTagContainer& Roots)
{
	FGameplayTagManagerWindowArgs Args;

	Args.bRestrictedTags = false;

	if (Roots.Num() > 0)
	{
		FString RootsTitle = FString::JoinBy(Roots, TEXT(", "), [] (const FGameplayTag& Tag) { return Tag.ToString(); });
		
		Args.Title = FText::Format(LOCTEXT("GameplayTags_WindowTitle_Filtered", "Tags ({0})"), FText::FromString(RootsTitle));
		
		Args.Filter = FString::JoinBy(Roots, TEXT(","), [] (const FGameplayTag& Tag) { return Tag.ToString(); });
	}
	else
	{
		Args.Title = LOCTEXT("GameplayTags_WindowTitle", "Gameplay Tags");
	}

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
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

#undef LOCTEXT_NAMESPACE