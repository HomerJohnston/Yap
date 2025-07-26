// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

struct FGameplayTag;
struct FGameplayTagContainer;

namespace Yap::EditorFuncs
{
	static void OpenDeveloperSettings();

	void OpenGameplayTagsEditor();
	
	void OpenGameplayTagsEditor(const FGameplayTag& Root);
	
	void OpenGameplayTagsEditor(const FGameplayTagContainer& Roots);
	
	void OpenProjectSettings();

	void PostNotificationInfo_Warning(FText Title, FText Description, float Duration = 6.0f);

	bool SaveAsset(UObject* Asset);
	
	bool IsValidEntryName(const FName InName, FText& OutErrorText);

	bool IsValidEntryNameString(const FString& InStringView, FText& OutErrorText);
}
