// Fill out your copyright notice in the Description page of Project Settings.

#include "ImGui_WS.h"

#include "ImGuiWorldDebuggerBase.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_WSModule::StartupModule()
{
	OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&ImGuiWorldDebuggerBootstrap::PostWorldInitialization);
}


void FImGui_WSModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitializationHandle);
}

IMPLEMENT_MODULE(FImGui_WSModule, ImGui_WS);

#undef LOCTEXT_NAMESPACE
