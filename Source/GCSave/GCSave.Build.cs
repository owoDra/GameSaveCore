// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GCSave : ModuleRules
{
	public GCSave(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GCSave",
                ModuleDirectory + "/GCSave/GlobalSave",
                ModuleDirectory + "/GCSave/PlayerSave",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "DeveloperSettings",
            }
        );
    }
}
