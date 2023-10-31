// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiScope.h"

#include "imgui.h"

namespace UnrealImGui
{
	FWidgetDisableScope::FWidgetDisableScope(bool IsDisable)
		: Disable(IsDisable)
	{
		if (Disable)
		{
			ImGui::BeginDisabled(true);
		}
	}

	FWidgetDisableScope::~FWidgetDisableScope()
	{
		if (Disable)
		{
			ImGui::EndDisabled();
		}
	}
}

