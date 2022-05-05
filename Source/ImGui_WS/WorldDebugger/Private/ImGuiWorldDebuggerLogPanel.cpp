// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerLogPanel.h"

#include "imgui.h"
#include "ImGuiWorldDebuggerLayout.h"

UImGuiWorldDebuggerLogPanel::UImGuiWorldDebuggerLogPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = NSLOCTEXT("ImGuiWorldDebugger", "Log", "Log");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Utils }
	};
}

void UImGuiWorldDebuggerLogPanel::Register(AImGuiWorldDebuggerBase* WorldDebugger)
{
	LogDevice.Register();
}

void UImGuiWorldDebuggerLogPanel::Unregister(AImGuiWorldDebuggerBase* WorldDebugger)
{
	LogDevice.Unregister();
}

void UImGuiWorldDebuggerLogPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	LogDevice.Draw(this);
	CmdDevice.Draw(this);
}
