using UnrealBuildTool;

public class ImGui_UnrealPanels : ModuleRules
{
    public ImGui_UnrealPanels(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Navmesh",
                "NavigationSystem",
                "EngineSettings",

                "ImGui",
	            "ImGui_Widgets",
                "ImGui_UnrealLayout",
            }
        );
    }
}