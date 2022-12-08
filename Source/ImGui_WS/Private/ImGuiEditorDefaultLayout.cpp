// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiEditorDefaultLayout.h"

#include "imgui.h"
#include "imgui_internal.h"

void UImGuiEditorDefaultLayout::LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	Super::LoadDefaultLayout(Owner, LayoutBuilder);

	const ImGuiID DockId = ImGui::DockBuilderAddNode(DockSpaceId, ImGuiDockNodeFlags_AutoHideTabBar);

	ImGuiID RemainAreaId;
	ImGuiID ViewportId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Left, 0.7f, nullptr, &RemainAreaId);
	const ImGuiID UtilsId = ImGui::DockBuilderSplitNode(ViewportId, ImGuiDir_Down, 0.3f, nullptr, &ViewportId);
	const ImGuiID OutlinerId = ImGui::DockBuilderSplitNode(RemainAreaId, ImGuiDir_Up, 0.3f, nullptr, &RemainAreaId);
	const ImGuiID DetailsId = ImGui::DockBuilderSplitNode(RemainAreaId, ImGuiDir_Down, 0.7f, nullptr, &RemainAreaId);
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

void FImGuiEditorDefaultLayoutBuilder::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(Layouts);
	Collector.AddReferencedObjects(Panels);
}

FString FImGuiEditorDefaultLayoutBuilder::GetReferencerName() const
{
	return TEXT("ImGuiEditorDefaultLayoutBuilder");
}

FImGuiEditorDefaultLayoutBuilder::FImGuiEditorDefaultLayoutBuilder()
{
	DockSpaceName = TEXT("ImGuiEditorDefaultLayoutDockSpace");
	SupportLayoutTypes.Add(UImGuiEditorDefaultLayoutBase::StaticClass());
	SupportPanelTypes.Add(UImGuiEditorDefaultPanelBase::StaticClass());
	Register(GetTransientPackage());
}

void FImGuiEditorDefaultLayoutBuilder::Draw(float DeltaSeconds)
{
	UObject* Owner = GetTransientPackage();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Layout"))
		{
			DrawLayoutStateMenu(Owner);
			if (ImGui::MenuItem("Reset"))
			{
				LoadDefaultLayout(Owner);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			DrawPanelStateMenu(Owner);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	
	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(Viewport->WorkPos);
	ImGui::SetNextWindowSize(Viewport->WorkSize);
	ImGui::SetNextWindowViewport(Viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDocking;
	WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (ImGui::Begin("Background", nullptr, WindowFlags))
	{
		DrawPanels(Owner, DeltaSeconds);
		ImGui::End();
	}
}
