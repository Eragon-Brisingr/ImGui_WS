#include "ImGui_WorldDebugger.h"

#include "ImGuiDelegates.h"
#include "ImGuiWorldDebuggerBase.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_WorldDebuggerModule::StartupModule()
{
	OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&ImGuiWorldDebuggerBootstrap::PostWorldInitialization);

	OnImGui_WS_EnableHandle = FImGuiDelegates::OnImGui_WS_Enable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireCreateDebugger);
	OnImGui_WS_DisableHandle = FImGuiDelegates::OnImGui_WS_Disable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireDestroyDebugger);
	OnImGuiLocalPanelEnableHandle = FImGuiDelegates::OnImGuiLocalPanelEnable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireCreateDebugger);
	OnImGuiLocalPanelDisableHandle = FImGuiDelegates::OnImGuiLocalPanelDisable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireDestroyDebugger);
}

void FImGui_WorldDebuggerModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitializationHandle);

	FImGuiDelegates::OnImGui_WS_Enable.Remove(OnImGui_WS_EnableHandle);
	FImGuiDelegates::OnImGui_WS_Disable.Remove(OnImGui_WS_DisableHandle);
	FImGuiDelegates::OnImGuiLocalPanelEnable.Remove(OnImGuiLocalPanelEnableHandle);
	FImGuiDelegates::OnImGuiLocalPanelDisable.Remove(OnImGuiLocalPanelDisableHandle);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_WorldDebuggerModule, ImGui_WorldDebugger)