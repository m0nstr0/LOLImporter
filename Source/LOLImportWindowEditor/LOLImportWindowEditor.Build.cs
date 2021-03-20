// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LOLImportWindowEditor : ModuleRules
{
	public LOLImportWindowEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"CoreUObject",
				"ApplicationCore",
				"InputCore",
                "Slate",
                "SlateCore",
				"PropertyEditor",
				"MainFrame"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				
			}
            );


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
            }
            );
	}
}
