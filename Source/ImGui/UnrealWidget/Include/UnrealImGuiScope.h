// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	struct IMGUI_API FWidgetDisableScope
	{
		FWidgetDisableScope(bool IsDisable);
		~FWidgetDisableScope();
	private:
		bool Disable;
	};
}