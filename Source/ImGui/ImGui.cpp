// Fill out your copyright notice in the Description page of Project Settings.

#include "ImGui.h"

#include "UnrealImGuiLogDevice.h"
#include "UnrealImGuiPropertyDetails.h"

#define LOCTEXT_NAMESPACE "ImGui"

void FImGuiModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddLambda([]
	{
		UnrealImGui::UnrealPropertyCustomizeFactory::InitialDefaultCustomizer();
	});

	UnrealImGui::GUnrealImGuiOutputDevice = MakeShared<UnrealImGui::FUnrealImGuiOutputDevice>();
}


void FImGuiModule::ShutdownModule()
{
	UnrealImGui::GUnrealImGuiOutputDevice.Reset();
}

IMPLEMENT_MODULE(FImGuiModule, ImGui);

#undef LOCTEXT_NAMESPACE
