﻿using UnrealBuildTool;

public class ImGui_UnrealLayout : ModuleRules
{
    public ImGui_UnrealLayout(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "UMG",
                "SlateCore",
                "InputCore",

                "ImGui",
                "ImGui_Slate",
            }
        );

        if (Target.bBuildEditor)
        {
	        PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}