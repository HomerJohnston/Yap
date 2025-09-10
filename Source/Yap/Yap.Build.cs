// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Yap : ModuleRules
{
	public Yap(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bUseUnity = true;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Flow",
				"GameplayTags",
				"Projects",
				"AIModule",
				"UMG"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"DeveloperSettings", 
				// ... add private dependencies that you statically link with here ...	
			}
			);

		if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion < 5)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"StructUtils" // This was rolled into engine core in 5.5
				}
			);
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
        
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd"
				}
			);
		}
	}
}
