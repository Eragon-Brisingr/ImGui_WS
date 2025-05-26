using UnrealBuildTool;

public class ImGui_UnrealPanelsEditor : ModuleRules
{
    public ImGui_UnrealPanelsEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "WorkspaceMenuStructure",
                "ToolMenus",
                
                "ImGui",
                "ImGui_Slate",
                "ImGui_UnrealLayout",
            }
        );
    }
}