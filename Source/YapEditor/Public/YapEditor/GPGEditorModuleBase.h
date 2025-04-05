// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// This is a drop-in header to assist with basic module definition, customization registration/deregistration, etc.
// It is intended to be copy/pasted in any module that wishes to use it.
// For usage, see comments in SETTINGS section below.

#pragma once

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"

class UObject;
class UThumbnailRenderer;
class IAssetTypeActions;
class IAssetTools;

using UAssetClass = TSubclassOf<UObject>;
using UThumbnailClass = TSubclassOf<UThumbnailRenderer>;

#define REGISTER_ASSET_TYPE_ACTION(NAME) AssetTypeActions.Add(MakeShared<NAME>())
#define REGISTER_DETAIL_CUSTOMIZATION(CLASSNAME, CUSTOMIZATIONNAME) DetailCustomizations.Append({{ CLASSNAME::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&CUSTOMIZATIONNAME::MakeInstance)}})
#define REGISTER_PROPERTY_CUSTOMIZATION(CLASSNAME, CUSTOMIZATIONNAME) PropertyCustomizations.Append({{ *CLASSNAME::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&CUSTOMIZATIONNAME::MakeInstance) }});
#define REGISTER_THUMBNAIL_RENDERER(CLASSNAME, THUMBNAILRENDERERNAME) ClassThumbnailRenderers.Append({{ CLASSNAME::StaticClass(), THUMBNAILRENDERERNAME::StaticClass() }});

class FGPGEditorModuleBase
{
	// ============================================================================================
	// SETTINGS
	/* Assign info to any of these four variables in your StartupModule(), and then call StartupModuleBase(). Call ShutdownModuleBase() in your ShutdownModule(). */
protected:
	TPair<FName, FText> AssetCategory;
	TArray<TSharedRef<IAssetTypeActions>> AssetTypeActions;
	TArray<TPair<TSubclassOf<UObject>, const FOnGetDetailCustomizationInstance>> DetailCustomizations;
	TArray<TPair<const UScriptStruct&, const FOnGetPropertyTypeCustomizationInstance>> PropertyCustomizations;
	TArray<TPair<UAssetClass, UThumbnailClass>> ClassThumbnailRenderers;
	/* Example, this code would be in StartupModule():
	// --------
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };

	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_Type()));
	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_Type()));
	
	DetailCustomizations.Add(UClassType::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_ClassType::MakeInstance));
	DetailCustomizations.Add(UClassType::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_ClassType::MakeInstance));
	
	PropertyCustomizations.Add(*FStructType::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_StructType::MakeInstance));
	PropertyCustomizations.Add(*UClassType::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_ClassType::MakeInstance));

	StartupModuleBase();
	// --------
	*/

	// ============================================================================================
	// STATE
private:
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TSet<FName> RegisteredCustomClassLayouts;
	TSet<FName> RegisteredCustomStructLayouts;
	static inline EAssetTypeCategories::Type RegisteredAssetCategory;

public: 
    static EAssetTypeCategories::Type GetAssetCategory()
    {
	    return RegisteredAssetCategory;
    };
	
protected:
	void StartupModuleBase()
	{
		if (AssetCategory.Key != NAME_None && !AssetCategory.Value.IsEmpty())
		{
			RegisteredAssetCategory = RegisterAssetCategory(AssetCategory.Key, AssetCategory.Value);
		}
	
		RegisterAssetTypeActions();
		RegisterDetailCustomizations();
		RegisterPropertyCustomizations();
		RegisterThumbnailRenderers();
		
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}
	
	void ShutdownModuleBase()
	{
		UnregisterAssetTypeActions();
		UnregisterDetailCustomizations();
		UnregisterPropertyCustomizations();
		UnregisterThumbnailRenderers();		
	}
	
	// ========================================== 
	// REGISTRATION OF CATEGORY
	
private:
	// ------------------------------------------
	EAssetTypeCategories::Type RegisterAssetCategory(FName Name, FText Text)
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			return AssetTools.RegisterAdvancedAssetCategory(Name, Text);
		}

		return EAssetTypeCategories::None;
	}

	// ========================================== 
	// REGISTRATION OF TYPES
	
private:
	// ------------------------------------------
	void RegisterAssetTypeActions()
	{
		if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (TSharedRef<IAssetTypeActions> Action : AssetTypeActions)
			{
				RegisterAssetTypeAction(AssetTools, Action);
			}
		}
	}
	
	void RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	// ------------------------------------------
	void RegisterDetailCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (TPair<TSubclassOf<UObject>, const FOnGetDetailCustomizationInstance>& Customization : DetailCustomizations)
			{
				RegisterCustomClassLayout(Customization.Key, Customization.Value);
			}
		
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}

	void RegisterCustomClassLayout(const TSubclassOf<UObject> Class, const FOnGetDetailCustomizationInstance DetailLayout)
	{
		if (!Class)
		{
			return;
		}

		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			PropertyEditorModule->RegisterCustomClassLayout(Class->GetFName(), DetailLayout);
			RegisteredCustomClassLayouts.Add(Class->GetFName());
		}
	}
	
	// ------------------------------------------
	void RegisterPropertyCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (TPair<const UScriptStruct&, const FOnGetPropertyTypeCustomizationInstance>& Customization : PropertyCustomizations)
			{
				RegisterCustomStructLayout(Customization.Key, Customization.Value);
			}
		
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}

	void RegisterCustomStructLayout(const UScriptStruct& Struct, const FOnGetPropertyTypeCustomizationInstance DetailLayout)
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			PropertyEditorModule->RegisterCustomPropertyTypeLayout(Struct.GetFName(), DetailLayout);
			RegisteredCustomStructLayouts.Add(Struct.GetFName());
		}
	}

	// ------------------------------------------
	void RegisterThumbnailRenderers()
	{
		UThumbnailManager& ThumbnailManager = UThumbnailManager::Get();

		for (TPair<UAssetClass, UThumbnailClass>& Pair : ClassThumbnailRenderers)
		{
			ThumbnailManager.RegisterCustomRenderer(Pair.Key, Pair.Value);
		}
	}
	
	// ========================================== 
	// DE-REGISTRATION OF TYPES
	
private:
	// ------------------------------------------
	void UnregisterAssetTypeActions()
	{
		if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (const TSharedRef<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	
		RegisteredAssetTypeActions.Empty();
	}

	// ------------------------------------------
	void UnregisterDetailCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (auto It = RegisteredCustomClassLayouts.CreateConstIterator(); It; ++It)
			{
				if (It->IsValid())
				{
					PropertyEditorModule->UnregisterCustomClassLayout(*It);
				}
			}
		}
	}
	
	// ------------------------------------------
	void UnregisterPropertyCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (auto It = RegisteredCustomStructLayouts.CreateConstIterator(); It; ++It)
			{
				if (It->IsValid())
				{
					PropertyEditorModule->UnregisterCustomPropertyTypeLayout(*It);
				}
			}
		}
	}
	
	// ------------------------------------------
	void UnregisterThumbnailRenderers()
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			UThumbnailManager& ThumbnailManager = UThumbnailManager::Get();

			for (TPair<UAssetClass, UThumbnailClass>& Pair : ClassThumbnailRenderers)
			{
				ThumbnailManager.RegisterCustomRenderer(Pair.Key, Pair.Value);
			}
		}
	}
};

