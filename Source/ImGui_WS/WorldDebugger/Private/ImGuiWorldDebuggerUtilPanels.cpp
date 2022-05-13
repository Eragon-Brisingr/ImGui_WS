// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerUtilPanels.h"

#include "imgui.h"
#include "ImGuiWorldDebuggerLayout.h"

#define LOCTEXT_NAMESPACE "ImGuiWorldDebugger"

UImGuiWorldDebuggerStatPanel::UImGuiWorldDebuggerStatPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = LOCTEXT("Stat", "Stat");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), { UImGuiWorldDebuggerDefaultLayout::EDockId::Utils, false } }
	};
}

void UImGuiWorldDebuggerStatPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	StatDevice.Draw(this);
}

UImGuiWorldDebuggerObjectBrowserPanel::UImGuiWorldDebuggerObjectBrowserPanel()
{
	Title = LOCTEXT("Object Browser", "Object Browser");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), { UImGuiWorldDebuggerDefaultLayout::EDockId::Utils, false } }
	};
}

void UImGuiWorldDebuggerObjectBrowserPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	ObjectBrowser.Draw(this);
}

#undef LOCTEXT_NAMESPACE
