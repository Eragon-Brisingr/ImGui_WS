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
	        "AIModule",
	        "Navmesh",
	        "NavigationSystem",
	        "Projects",
	        "DeveloperSettings",
	        "zlib",
	        "OpenSSL",
	        
	        "ImGui",
	        "UWS",
		});
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "WorldDebugger/Include"),
		});
		
		// 打包后也能读取字体文件
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/Zfull-GB.ttf"), StagedFileType.NonUFS);
		// 打包后也能加载网页
		RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "HTML/index.html"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "HTML/imgui-ws.js"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "HTML/draw-mouse-pos.js"), StagedFileType.NonUFS);
	}
}
