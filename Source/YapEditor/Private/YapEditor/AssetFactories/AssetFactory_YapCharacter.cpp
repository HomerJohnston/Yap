// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/AssetFactories/AssetFactory_YapCharacter.h"

#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapEditorModule.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UAssetFactory_YapCharacter::UAssetFactory_YapCharacter()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYapCharacterAsset::StaticClass();
}

UObject* UAssetFactory_YapCharacter::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Uncomment one of these

	// Create an Object asset of this class (this creates an instance of the class in the Content Folder as a project asset)
	return NewObject<UYapCharacterAsset>(InParent, Class, Name, Flags | RF_Transactional);

	// Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
	//return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

#undef LOCTEXT_NAMESPACE

