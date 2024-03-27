// Fill out your copyright notice in the Description page of Project Settings.

#include "ImGuiModule.h"

#include "UnrealImGuiLogDevice.h"
#include "UnrealImGuiPropertyDetails.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "ImGui"

void FImGuiModule::StartupModule()
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


void FImGuiModule::ShutdownModule()
{

}

IMPLEMENT_MODULE(FImGuiModule, ImGui);

#undef LOCTEXT_NAMESPACE
