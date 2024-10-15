#include "ImGui_SlateModule.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_SlateModule::StartupModule()
{
	
}

void FImGui_SlateModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_SlateModule, ImGui_Slate)