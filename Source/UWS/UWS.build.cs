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
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/uWebSockets/uWS-win-x64.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/libuv/lib/libuv-win-x64.lib"));
			PublicSystemLibraries.Add("UserEnv.Lib");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/uWebSockets/uWS-linux-x64.a"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "prebuilt/libuv/lib/libuv-linux-x86_64.a"));
		}
	}
}
