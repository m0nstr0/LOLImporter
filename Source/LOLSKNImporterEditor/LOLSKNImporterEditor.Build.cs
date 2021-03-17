// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LOLSKNImporterEditor : ModuleRules
{
	public LOLSKNImporterEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"LOLCoreEditor",
				"InputCore", //ui
                "Slate", //ui
                "SlateCore", //ui
				"PropertyEditor", //ui
				"MainFrame", //ui
				
				"ApplicationCore",
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"DesktopPlatform",
				"EditorStyle",
				"MeshDescription",
				"StaticMeshDescription",
				"AssetRegistry",
				"MeshBuilder"
				// ... add private dependencies that you statically link with here ...	
			}
            );


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
            }
            );
	}
}
