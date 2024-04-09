// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ImGuiUnrealLocalPanel.generated.h"

class UUnrealImGuiPanelBase;

namespace ImGui_WS::LocalPanel
{
	enum class ELocalPanelMode : int32
	{
		GameViewport = 0,
		SingleWindow = 1,
		DockWindow = 2,
	};
	IMGUI_SLATE_API bool IsLocalWindowOpened(const UWorld* World);
	IMGUI_SLATE_API void OpenLocalWindow();
	IMGUI_SLATE_API void CloseLocalWindow();
}

USTRUCT()
struct FImGuiLocalPanelConfig
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector2f Pos{ 0.1f, 0.1f };
	UPROPERTY()
	FVector2f Size{ 0.4f, 0.4f };
};

UCLASS(Config = ImGuiPanelUserConfig, PerObjectConfig)
class UImGuiLocalPanelManagerConfig : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(Config)
	FImGuiLocalPanelConfig ManagerConfig;
	UPROPERTY(Config)
	bool bManagerCollapsed = true;

	UPROPERTY(Config)
	TMap<TSoftClassPtr<UUnrealImGuiPanelBase>, FImGuiLocalPanelConfig> PanelConfigMap;

	UPROPERTY(Config)
	FImGuiLocalPanelConfig ViewportConfig{ { 0.1f, 0.1f }, { 0.8f, 0.8f } };

	UPROPERTY(Config)
	TArray<TSoftClassPtr<UUnrealImGuiPanelBase>> RecentlyPanels;
};
