// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ImGui_WS : ModuleRules
{
	public ImGui_WS(ReadOnlyTargetRules Target) : base(Target)
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
	        "DeveloperSettings",
	        
	        "ImGui",
	        "Incppect",
		});

		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/...*.ttf"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/HTML/...*.html"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/HTML/...*.js"), StagedFileType.NonUFS);
	}
}
