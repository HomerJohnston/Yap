// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphPins/YapAudioAssetGraphPin.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Layout/SBox.h"
#include "Yap/YapProjectSettings.h"

TSharedRef<SWidget> SYapAudioAssetGraphPin::GenerateAssetPicker()
{
	// This class and its children are the classes that we can show objects for
	TArray<UClass*> AllowedClasses;

	for (const TSoftClassPtr<UObject>& ClassSoft : UYapProjectSettings::GetAudioAssetClasses())
	{
		if (!ClassSoft.IsNull())
		{
			AllowedClasses.Add(ClassSoft.LoadSynchronous());
		}
	}
	
	if (AllowedClasses.Num() == 0)
	{
		AllowedClasses.Add(UObject::StaticClass());
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SYapAudioAssetGraphPin::OnAssetSelectedFromPicker);
	AssetPickerConfig.OnAssetEnterPressed = FOnAssetEnterPressed::CreateSP(this, &SYapAudioAssetGraphPin::OnAssetEnterPressedInPicker);
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
	AssetPickerConfig.bAllowDragging = false;

	for (const UClass* AllowedClass : AllowedClasses)
	{
		AssetPickerConfig.Filter.ClassPaths.Add(AllowedClass->GetClassPathName());
	}
	
	return SNew(SBox)
	.HeightOverride(300)
	.WidthOverride(300)
	[
		SNew(SBorder)
		.BorderImage( FAppStyle::GetBrush("Menu.Background") )
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		]
	];
}
