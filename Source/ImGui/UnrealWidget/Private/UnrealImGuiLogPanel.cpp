// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLogPanel.h"

#include "imgui.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

UUnrealImGuiLogPanel::UUnrealImGuiLogPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = LOCTEXT("Log", "Log");
	DefaultState = FDefaultPanelState{ true, true };
}

void UUnrealImGuiLogPanel::Register(UObject* Owner)
{
	LogDevice.Register();
}

void UUnrealImGuiLogPanel::Unregister(UObject* Owner)
{
	LogDevice.Unregister();
}

void UUnrealImGuiLogPanel::Draw(UObject* Owner, float DeltaSeconds)
{
	LogDevice.Draw(this);
	CmdDevice.Draw(this);
}

#undef LOCTEXT_NAMESPACE
