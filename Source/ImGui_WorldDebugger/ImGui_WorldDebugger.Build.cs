using UnrealBuildTool;

public class ImGui_WorldDebugger : ModuleRules
{
    public ImGui_WorldDebugger(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.Platform != UnrealTargetPlatform.Win64 && Target.Platform != UnrealTargetPlatform.Linux)
        {
            Type = ModuleType.External;
        }

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
                "ImGui_WS",
            }
        );
    }
}