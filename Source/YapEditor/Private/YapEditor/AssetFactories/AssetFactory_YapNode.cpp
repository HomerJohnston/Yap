// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/AssetFactories/AssetFactory_YapNode.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "Yap/YapNodeBlueprint.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

UAssetFactory_YapNode::UAssetFactory_YapNode()
{
    SupportedClass = UFlowNode_YapDialogue::StaticClass();

    bCreateNew = true;

    bEditAfterNew = false;
}

UObject* UAssetFactory_YapNode::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
    // Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
    return FKismetEditorUtilities::CreateBlueprint(UFlowNode_YapDialogue::StaticClass(), InParent, InName, BPTYPE_Normal, UYapNodeBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext);
}
