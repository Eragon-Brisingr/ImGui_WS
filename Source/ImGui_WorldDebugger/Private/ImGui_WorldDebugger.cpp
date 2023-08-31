#include "ImGui_WorldDebugger.h"

#include "ImGuiWorldDebuggerBase.h"

#define LOCTEXT_NAMESPACE "ImGui_WorldDebugger"

void FImGui_WorldDebuggerModule::StartupModule()
{
	OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&ImGuiWorldDebuggerBootstrap::PostWorldInitialization);
}

void FImGui_WorldDebuggerModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitializationHandle);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_WorldDebuggerModule, ImGui_WorldDebugger)