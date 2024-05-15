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
	        "ApplicationCore",
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

		string FontFileName;
		{
			ConfigHierarchy Config = ConfigCache.ReadHierarchy(ConfigHierarchyType.Game, Target.ProjectFile != null ? Target.ProjectFile.Directory : null, Target.Platform);
			if (!Config.GetString("/Script/ImGui.ImGuiSettings", "FontFileName", out FontFileName))
			{
				FontFileName = "zpix.ttf";
			}
		}
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources", FontFileName), StagedFileType.NonUFS);
	}
}
