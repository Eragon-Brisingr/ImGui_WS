// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerDetailsPanel.h"

#include "imgui.h"
#include "UnrealImGuiPropertyDetails.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerViewportPanel.h"
#include "UnrealImGuiPanelBuilder.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

UImGuiWorldDebuggerDetailsPanel::UImGuiWorldDebuggerDetailsPanel()
	: bDisplayAllProperties(false)
	, bEnableEditVisibleProperty(false)
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = LOCTEXT("Details", "Details");
	Categories = { LOCTEXT("ViewportCategory", "Viewport") };
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Details }
	};
}

void UImGuiWorldDebuggerDetailsPanel::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	const UImGuiWorldDebuggerViewportPanel* Viewport = Builder->FindPanel<UImGuiWorldDebuggerViewportPanel>();
	if (Viewport == nullptr)
	{
		return;
	}

	bool IsConfigDirty = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Detail Settings"))
		{
			{
				bool Value = bDisplayAllProperties;
				if (ImGui::Checkbox("Display All Properties", &Value))
				{
					bDisplayAllProperties = Value;
					IsConfigDirty |= true;
				}
			}
			{
				bool Value = bEnableEditVisibleProperty;
				if (ImGui::Checkbox("Enable Edit Visible Property", &Value))
				{
					bEnableEditVisibleProperty = Value;
					IsConfigDirty |= true;
				}
			}
			ImGui::EndMenu();
		}
		if (IsConfigDirty)
		{
			SaveConfig();
		}
		ImGui::EndMenuBar();
	}
	if (IsConfigDirty)
	{
		SaveConfig();
	}

	TGuardValue<bool> GDisplayAllPropertiesGuard(UnrealImGui::GlobalValue::GDisplayAllProperties, bDisplayAllProperties);
	TGuardValue<bool> GEnableEditVisiblePropertyGuard(UnrealImGui::GlobalValue::GEnableEditVisibleProperty, bEnableEditVisibleProperty);

	for (UUnrealImGuiViewportExtentBase* Extent : Viewport->Extents)
	{
		if (Extent->bEnable)
		{
			Extent->DrawDetailsPanel(Owner, this);
		}
	}
}

#undef LOCTEXT_NAMESPACE
