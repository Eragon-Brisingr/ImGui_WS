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
		const int32* DockIdPtr = Panel->DefaultDockSpace.Find(GetClass()->GetFName());
		const int32 PanelDockKey = DockIdPtr ? *DockIdPtr : DefaultDockId;
		const ImGuiID* MappedDockId = DockIdMap.Find(PanelDockKey);
		if (MappedDockId && *MappedDockId != UUnrealImGuiPanelBase::NoDock)
		{
			const FString PanelName = Panel->GetLayoutPanelName(LayoutName.ToString());
			ImGui::DockBuilderDockWindow(TCHAR_TO_UTF8(*PanelName), DockIdMap[PanelDockKey]);
			
			Panel->bIsOpen = true;
			Panel->PanelOpenState.Add(GetClass()->GetFName(), true);
		}
		else
		{
			Panel->bIsOpen = false;
			Panel->PanelOpenState.Add(GetClass()->GetFName(), false);
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
