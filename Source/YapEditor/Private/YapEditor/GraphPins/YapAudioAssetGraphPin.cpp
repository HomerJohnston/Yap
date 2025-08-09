// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphPins/YapAudioAssetGraphPin.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
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
	
	// Check with the node to see if there is any "AllowClasses" or "DisallowedClasses" metadata for the pin
	FString AllowedClassesFilterString = GraphPinObj->GetOwningNode()->GetPinMetaData(GraphPinObj->PinName, FName(TEXT("AllowedClasses")));
	if( !AllowedClassesFilterString.IsEmpty() )
	{
		// Clear out the allowed class names and have the pin's metadata override.
		AssetPickerConfig.Filter.ClassPaths.Empty();

		// Parse and add the classes from the metadata
		TArray<FString> AllowedClassesFilterNames;
		AllowedClassesFilterString.ParseIntoArrayWS(AllowedClassesFilterNames, TEXT(","), true);
		for(const FString& AllowedClassesFilterName : AllowedClassesFilterNames)
		{
			ensureAlwaysMsgf(!FPackageName::IsShortPackageName(AllowedClassesFilterName), TEXT("Short class names are not supported as AllowedClasses on pin \"%s\": class \"%s\""), *GraphPinObj->PinName.ToString(), *AllowedClassesFilterName);
			AssetPickerConfig.Filter.ClassPaths.Add(FTopLevelAssetPath(AllowedClassesFilterName));
		}
	}

	FString DisallowedClassesFilterString = GraphPinObj->GetOwningNode()->GetPinMetaData(GraphPinObj->PinName, FName(TEXT("DisallowedClasses")));
	if(!DisallowedClassesFilterString.IsEmpty())
	{
		TArray<FString> DisallowedClassesFilterNames;
		DisallowedClassesFilterString.ParseIntoArrayWS(DisallowedClassesFilterNames, TEXT(","), true);
		for(const FString& DisallowedClassesFilterName : DisallowedClassesFilterNames)
		{
			ensureAlwaysMsgf(!FPackageName::IsShortPackageName(DisallowedClassesFilterName), TEXT("Short class names are not supported as DisallowedClasses on pin \"%s\": class \"%s\""), *GraphPinObj->PinName.ToString(), *DisallowedClassesFilterName);
			AssetPickerConfig.Filter.RecursiveClassPathsExclusionSet.Add(FTopLevelAssetPath(DisallowedClassesFilterName));
		}
	}

	return
	SNew(SBox)
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
