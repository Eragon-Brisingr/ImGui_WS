// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerDetailsPanel.h"

#include "imgui.h"
#include "UnrealImGuiPropertyDetails.h"
#include "ImGuiWorldDebuggerBase.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerViewportPanel.h"

UImGuiWorldDebuggerDetailsPanel::UImGuiWorldDebuggerDetailsPanel()
	: bDisplayAllProperties(false)
	, bEnableEditVisibleProperty(false)
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = NSLOCTEXT("ImGuiWorldDebugger", "Details", "Details");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Details }
	};
}

void UImGuiWorldDebuggerDetailsPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	const UImGuiWorldDebuggerViewportPanel* Viewport = WorldDebugger->PanelBuilder.FindPanel<UImGuiWorldDebuggerViewportPanel>();
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
			Extent->DrawDetailsPanel(WorldDebugger, this);
		}
	}
}
