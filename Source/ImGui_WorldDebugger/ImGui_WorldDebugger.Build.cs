﻿using UnrealBuildTool;

public class ImGui_WorldDebugger : ModuleRules
{
    public ImGui_WorldDebugger(ReadOnlyTargetRules Target) : base(Target)
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
                "SlateCore",

                "ImGui",
				"ImGui_Widgets",
                "ImGui_UnrealLayout",
                "ImGui_UnrealPanels",
            }
        );

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}