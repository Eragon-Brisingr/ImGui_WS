// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class UWS : ModuleRules
{
	public UWS(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "uWebSockets/src"),
			Path.Combine(ModuleDirectory, "uWebSockets/uSockets/src"),
			Path.Combine(ModuleDirectory, "prebuilt/libuv/include"),
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/uWS.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/libuv/lib/uv.lib"));
			
			RuntimeDependencies.Add("$(BinaryOutputDir)/uv.dll", Path.Combine(ModuleDirectory, "prebuilt/libuv/lib/uv.dll"));
		}
	}
}
