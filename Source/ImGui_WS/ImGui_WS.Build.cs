// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ImGui_WS : ModuleRules
{
	public ImGui_WS(ReadOnlyTargetRules Target) : base(Target)
	{
		// 打包后也能读取字体文件
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/zpix.ttf"), StagedFileType.NonUFS);
		// 打包后也能加载网页
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/HTML/index.html"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/HTML/imgui-ws.js"), StagedFileType.NonUFS);
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Resources/HTML/draw-mouse-pos.js"), StagedFileType.NonUFS);
		
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
	        "WebSocketNetworking",
		});
	}
}
