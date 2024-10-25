// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"

namespace UnrealImGui
{
	IMGUI_WIDGETS_API bool ComboEnum(const char* Label, int64& EnumValue, const UEnum* EnumType, ImGuiComboFlags Flags = 0);
	template<typename T> requires std::is_enum_v<T>
	bool ComboEnum(const char* Label, T& Enum, ImGuiComboFlags Flags = 0)
	{
		return ComboEnum(Label, StaticEnum<T>(), Enum, Flags);
	}
}
