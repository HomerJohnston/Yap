// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetFactories/AssetFactory_YapBroker.h"

#include "Yap/YapBroker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UAssetFactory_YapBroker::UAssetFactory_YapBroker()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYapBroker::StaticClass();
}

UObject* UAssetFactory_YapBroker::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Uncomment one of these

	// Create an Object asset of this class (this creates an instance of the class in the Content Folder as a project asset)
	return NewObject<UYapBroker>(InParent, Class, Name, Flags | RF_Transactional);

	// Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
	//return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

#undef LOCTEXT_NAMESPACE

