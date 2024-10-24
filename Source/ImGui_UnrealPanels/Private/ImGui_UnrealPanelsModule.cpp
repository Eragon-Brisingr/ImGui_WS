#include "ImGui_UnrealPanelsModule.h"

#include "UnrealImGuiLogDevice.h"
#include "UnrealImGuiPropertyDetails.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_UnrealPanelsModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddLambda([]
	{
		UnrealImGui::UnrealPropertyCustomizeFactory::InitialDefaultCustomizer();
	});
	if (ensure(GLog))
	{
		GLog->AddOutputDevice(&UnrealImGui::GUnrealImGuiOutputDevice);
		FCoreDelegates::OnPreExit.AddLambda([]
		{
			if (GLog)
			{
				GLog->RemoveOutputDevice(&UnrealImGui::GUnrealImGuiOutputDevice);
			}
		});
	}
}

void FImGui_UnrealPanelsModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_UnrealPanelsModule, ImGui_UnrealPanels)