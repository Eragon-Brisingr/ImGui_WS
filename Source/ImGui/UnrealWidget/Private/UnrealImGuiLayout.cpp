// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLayout.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiPanel.h"

void UUnrealImGuiLayoutBase::ApplyPanelDockSettings(const FUnrealImGuiPanelBuilder& LayoutBuilder, TMap<int32, uint32> DockIdMap, const int32 DefaultDockId)
{
	for (UUnrealImGuiPanelBase* Panel : LayoutBuilder.Panels)
	{
		const UUnrealImGuiPanelBase::FDefaultDockLayout* DefaultDockLayout = Panel->DefaultDockSpace.Find(GetClass()->GetFName());
		const UUnrealImGuiPanelBase::FDefaultPanelState& DefaultPanelState = DefaultDockLayout ? *DefaultDockLayout : Panel->DefaultState;
		const int32 PanelDockKey = DefaultDockLayout ? DefaultDockLayout->DockId : DefaultDockId;
		const ImGuiID* MappedDockId = DockIdMap.Find(PanelDockKey);
		if (DefaultPanelState.bEnableDock && MappedDockId)
		{
			const FString PanelName = Panel->GetLayoutPanelName(LayoutName.ToString());
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
	if (DockSpaceId != INDEX_NONE && bIsCheckNewPanelLayout == false)
	{
		bIsCheckNewPanelLayout = true;

		for (const UUnrealImGuiPanelBase* Panel : LayoutBuilder.Panels)
		{
			const FString PanelName = Panel->GetLayoutPanelName(LayoutName.ToString());
			const ImGuiID WindowId = ImHashStr(TCHAR_TO_UTF8(*PanelName));
			const ImGuiWindowSettings* Settings = ImGui::FindWindowSettings(WindowId);
			if (Settings == nullptr)
			{
				LoadDefaultLayout(Owner, LayoutBuilder);
				break;
			}
		}
	}

	DockSpaceId = ImGui::GetID(TCHAR_TO_UTF8(*LayoutName.ToString()));
	ImGui::DockSpace(DockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}
