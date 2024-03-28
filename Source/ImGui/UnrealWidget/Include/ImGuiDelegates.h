// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImGuiContextDestroyed, struct ImGuiContext*);
	IMGUI_API extern FOnImGuiContextDestroyed OnImGuiContextDestroyed;
}
