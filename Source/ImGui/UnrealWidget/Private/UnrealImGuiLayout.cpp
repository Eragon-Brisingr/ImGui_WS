// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLayout.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiDelegates.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiPanel.h"

void UUnrealImGuiLayoutBase::Register(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	UnrealImGui::OnImGuiContextDestroyed.AddUObject(this, &ThisClass::WhenImGuiContextDestroyed);
}

void UUnrealImGuiLayoutBase::Unregister(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	UnrealImGui::OnImGuiContextDestroyed.RemoveAll(this);
}

void UUnrealImGuiLayoutBase::ApplyPanelDockSettings(const FUnrealImGuiPanelBuilder& LayoutBuilder, const TMap<int32, uint32>& DockIdMap, const int32 DefaultDockId)
{
	for (UUnrealImGuiPanelBase* Panel : LayoutBuilder.Panels)
	{
		const UUnrealImGuiPanelBase::FDefaultDockLayout* DefaultDockLayout = Panel->DefaultDockSpace.Find(GetClass()->GetFName());
		const UUnrealImGuiPanelBase::FDefaultPanelState& DefaultPanelState = DefaultDockLayout ? static_cast<const UUnrealImGuiPanelBase::FDefaultPanelState&>(*DefaultDockLayout) : Panel->DefaultState;
		const int32 PanelDockKey = DefaultDockLayout ? DefaultDockLayout->DockId : DefaultDockId;
		const ImGuiID* MappedDockId = DockIdMap.Find(PanelDockKey);
		if (DefaultPanelState.bEnableDock && MappedDockId)
		{
			const FString PanelName = Panel->GetLayoutPanelName(GetName());
			ImGui::DockBuilderDockWindow(TCHAR_TO_UTF8(*PanelName), *MappedDockId);
			
			Panel->PanelOpenState.Add(GetClass()->GetFName(), Panel->bIsOpen);
			Panel->SetOpenState(DefaultPanelState.bOpen);
		}
		else
		{
			Panel->PanelOpenState.Add(GetClass()->GetFName(), Panel->bIsOpen);
			Panel->SetOpenState(DefaultPanelState.bOpen);
		}
	}
}

void UUnrealImGuiLayoutBase::CreateDockSpace(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	uint32& DockSpaceId = DockSpaceIdMap.FindOrAdd(ImGui::GetCurrentContext(), INDEX_NONE);
	if (DockSpaceId == INDEX_NONE)
	{
		const_cast<uint32&>(DockSpaceId) = ImGui::GetID(TCHAR_TO_UTF8(*GetName()));

		for (const UUnrealImGuiPanelBase* Panel : LayoutBuilder.Panels)
		{
			const FString PanelName = Panel->GetLayoutPanelName(GetName());
			const ImGuiID WindowId = ImHashStr(TCHAR_TO_UTF8(*PanelName));
			const ImGuiWindowSettings* Settings = ImGui::FindWindowSettingsByID(WindowId);
			if (Settings == nullptr)
			{
				LoadDefaultLayout(Owner, LayoutBuilder);
				break;
			}
		}
	}

	ImGui::DockSpace(DockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void UUnrealImGuiLayoutBase::WhenImGuiContextDestroyed(ImGuiContext* Context)
{
	DockSpaceIdMap.Remove(Context);
}
