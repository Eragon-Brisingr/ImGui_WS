// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct ImGuiStyle;

namespace UnrealImGui
{
	IMGUI_API bool ShowStyleSelector(const char* Label = nullptr);

	IMGUI_API void DefaultStyle(ImGuiStyle* dst = nullptr);
}
