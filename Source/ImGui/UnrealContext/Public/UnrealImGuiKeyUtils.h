// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FKey;
enum ImGuiKey : int32;

namespace UnrealImGui
{
	IMGUI_API ImGuiKey ConvertKey(const FKey& Key);
	IMGUI_API FKey ConvertKey(const ImGuiKey& Key);
}
