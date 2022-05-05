// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiUtils.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace UnrealImGui
{
	FWidgetDisableScope::FWidgetDisableScope(bool IsDisable)
		: Disable(IsDisable)
	{
		if (Disable)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
	}

	FWidgetDisableScope::~FWidgetDisableScope()
	{
		if (Disable)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
}

