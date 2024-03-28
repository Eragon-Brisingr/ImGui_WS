// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ImGui : ModuleRules
{
	public ImGui(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "Projects",
			"EngineSettings",
			"Slate",
			"DesktopPlatform",
			"Navmesh",
			"NavigationSystem",
			"DeveloperSettings",
		});
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "ImGuiLibrary/Include"),
			Path.Combine(ModuleDirectory, "ImPlotLibrary/Include"),
			Path.Combine(ModuleDirectory, "UnrealWidget/Include"),
			Path.Combine(ModuleDirectory, "WidgetsLibrary/Include"),
		});

		PublicDefinitions.AddRange(new[]
		{
			"IMGUI_USER_CONFIG=\"ImGuiConfig.h\"",
			"IMPLOT_API=IMGUI_API",
		});
	}
}
