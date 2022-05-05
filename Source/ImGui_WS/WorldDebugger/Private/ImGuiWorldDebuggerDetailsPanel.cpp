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
	TGuardValue<bool> GDisplayAllPropertiesGuard(UnrealImGui::GlobalValue::GDisplayAllProperties, bDisplayAllProperties);
	TGuardValue<bool> GEnableEditVisiblePropertyGuard(UnrealImGui::GlobalValue::GEnableEditVisibleProperty, bEnableEditVisibleProperty);

	UnrealImGui::TObjectArray<AActor> FilteredSelectedActors;
	{
		FilteredSelectedActors.Reset(Viewport->SelectedActors.Num());
		for (const TWeakObjectPtr<AActor>& ActorPtr : Viewport->SelectedActors)
		{
			if (AActor* Actor = ActorPtr.Get())
			{
				FilteredSelectedActors.Add(Actor);
			}
		}
	}

	AActor* FirstActor = FilteredSelectedActors.Num() >= 1 ? FilteredSelectedActors[0] : nullptr;
	if (FilteredSelectedActors.Num() == 1)
	{
		ImGui::Text("Actor Name: %s", TCHAR_TO_UTF8(*FirstActor->GetName()));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text(TCHAR_TO_UTF8(*FirstActor->GetName()));
			ImGui::EndTooltip();
		}
	}
	else
	{
		ImGui::Text("%d Actors", FilteredSelectedActors.Num());
	}
	if (FirstActor)
	{
		UnrealImGui::DrawDetailTable("Actor", UnrealImGui::GetTopClass(FilteredSelectedActors), FilteredSelectedActors);
	}

	if (IsConfigDirty)
	{
		SaveConfig();
	}
}
