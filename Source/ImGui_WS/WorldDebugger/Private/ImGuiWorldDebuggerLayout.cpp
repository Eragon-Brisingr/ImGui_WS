// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerLayout.h"

#include "imgui.h"
#include "imgui_internal.h"

#define LOCTEXT_NAMESPACE "ImGuiWorldDebugger"

UImGuiWorldDebuggerDefaultLayout::UImGuiWorldDebuggerDefaultLayout()
{
	LayoutName = LOCTEXT("Default", "Default");
}

void UImGuiWorldDebuggerDefaultLayout::LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	const ImGuiID DockId = ImGui::DockBuilderAddNode(DockSpaceId, ImGuiDockNodeFlags_None);

	ImGuiID ViewportId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Left, 0.7f, nullptr, &DockSpaceId);
	const ImGuiID UtilsId = ImGui::DockBuilderSplitNode(ViewportId, ImGuiDir_Down, 0.3f, nullptr, &ViewportId);
	const ImGuiID OutlinerId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Up, 0.3f, nullptr, &DockSpaceId);
	const ImGuiID DetailsId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Down, 0.7f, nullptr, &DockSpaceId);
	const TMap<int32, ImGuiID> DockIdMap
	{
		{ Viewport, ViewportId },
		{ Outliner, OutlinerId },
		{ Details, DetailsId },
		{ Utils, UtilsId },
	};
	ApplyPanelDockSettings(LayoutBuilder, DockIdMap, EDockId::Utils);

	ImGui::DockBuilderFinish(DockId);
}

#undef LOCTEXT_NAMESPACE
