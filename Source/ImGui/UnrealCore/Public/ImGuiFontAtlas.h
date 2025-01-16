// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct ImFontAtlas;

namespace UnrealImGui
{
	IMGUI_API ImFontAtlas& GetDefaultFontAtlas();
	constexpr uint32 FontTextId = 0;

	IMGUI_API float GetSystemDPIScale();
	IMGUI_API float GetGlobalDPIScale();

	IMGUI_API void DrawGlobalDPISettings();
}
