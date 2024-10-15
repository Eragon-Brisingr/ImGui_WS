using UnrealBuildTool;

public class ImGui_ViewportEditor : ModuleRules
{
    public ImGui_ViewportEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "UnrealEd",
                
                "ImGui",
                "ImGui_Slate",
                "ImGui_Viewport",
            }
        );
    }
}