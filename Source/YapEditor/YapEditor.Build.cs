using UnrealBuildTool;

public class YapEditor : ModuleRules
{
    public YapEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseUnity = false;
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "Slate", "FlowEditor", "GameplayTags", "GameplayTagsEditor",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                
                "UnrealEd",
                "PropertyEditor",
                "AssetTools",
                "DetailCustomizations",
                
                "Flow",
                "FlowEditor",
                "Yap", 
                
                "KismetWidgets", 
                
                "EditorStyle",
                "GraphEditor",
                
                "EditorSubsystem",
                "InputCore",
                
                "PropertyEditor",
                "ToolMenus",
                
                "UMG",
                "UMGEditor",
                
                "DeveloperSettings",
                
                "BlueprintGraph", "LiveCoding",
                
                "EditorWidgets",
                "ToolWidgets",
                "KismetWidgets",
                
                "Projects",
                
                "AssetRegistry",
                "AssetDefinition"
            }
        );
    }
}