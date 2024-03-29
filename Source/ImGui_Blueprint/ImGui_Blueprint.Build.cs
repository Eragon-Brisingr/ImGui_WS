using UnrealBuildTool;

public class ImGui_Blueprint : ModuleRules
{
    public ImGui_Blueprint(ReadOnlyTargetRules Target) : base(Target)
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
                "BlueprintGraph",
                "UnrealEd",
                "KismetCompiler",

                "ImGui",
            }
        );
    }
}