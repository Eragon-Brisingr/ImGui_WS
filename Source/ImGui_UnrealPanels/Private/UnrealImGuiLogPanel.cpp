// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLogPanel.h"

#include "imgui.h"

UUnrealImGuiLogPanel::UUnrealImGuiLogPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = TEXT("Log");
	Categories = { TEXT("Viewport") };
	DefaultState = { true, true };
}

void UUnrealImGuiLogPanel::Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	LogDevice.Register();
}

void UUnrealImGuiLogPanel::Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	LogDevice.Unregister();
}

void UUnrealImGuiLogPanel::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	LogDevice.Draw(this);
	CmdDevice.Draw(this);
}
