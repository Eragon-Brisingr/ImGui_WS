// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct IMGUI_API FImGuiDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnImGuiContextDestroyed, struct ImGuiContext*);
	static FOnImGuiContextDestroyed OnImGuiContextDestroyed;

	DECLARE_MULTICAST_DELEGATE(FOnImGui_WS_Enable);
	static FOnImGui_WS_Enable OnImGui_WS_Enable;
	DECLARE_MULTICAST_DELEGATE(FOnImGui_WS_Disable);
	static FOnImGui_WS_Disable OnImGui_WS_Disable;

	DECLARE_MULTICAST_DELEGATE(FOnImGuiLocalPanelEnable);
	static FOnImGuiLocalPanelEnable OnImGuiLocalPanelEnable;
	DECLARE_MULTICAST_DELEGATE(FOnImGuiLocalPanelDisable);
	static FOnImGuiLocalPanelDisable OnImGuiLocalPanelDisable;
};
