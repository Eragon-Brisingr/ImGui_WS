// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct ImGuiStyle;

namespace UnrealImGui
{
	namespace EStyle
	{
		enum Type : int32
		{
			Default,
			UnrealDark,
			ModernColors,
			MaterialYouColors,
			FluentUI,
			CatppuccinMochaColors,
			ImGuiDark,
			ImGuiLight,
			ImGuiClassic,
		};
	}
	
	IMGUI_API bool ShowStyleSelector(const char* Label = nullptr);

	IMGUI_API void DefaultStyle(ImGuiStyle* dst = nullptr);

	IMGUI_API void SetStyle(EStyle::Type Style, ImGuiStyle* dst = nullptr);
}
