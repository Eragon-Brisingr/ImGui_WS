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
			"Slate",
			"DeveloperSettings",
			"RenderCore",
			"RHI",
		});

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop))
		{
			PrivateDependencyModuleNames.Add("DesktopPlatform");
		}
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "ImGuiLibrary/Public"),
			Path.Combine(ModuleDirectory, "ImPlotLibrary/Public"),
			Path.Combine(ModuleDirectory, "UnrealCore/Public"),
			Path.Combine(ModuleDirectory, "WidgetsLibrary/Public"),
		});

		PublicDefinitions.AddRange(new[]
		{
			"IMGUI_USER_CONFIG=\"ImGuiConfig.h\"",
			"IMPLOT_API=IMGUI_API",
		});
	}
}
