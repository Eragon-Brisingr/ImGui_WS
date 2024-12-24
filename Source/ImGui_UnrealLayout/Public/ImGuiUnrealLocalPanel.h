// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Object.h"
#include "ImGuiUnrealLocalPanel.generated.h"

class UUnrealImGuiPanelBase;

UENUM()
enum class EImGuiLocalPanelMode : uint8
{
	GameViewport = 0,
	SingleWindow = 1,
	DockWindow = 2,
};

namespace ImGui_WS::LocalPanel
{
	IMGUI_UNREALLAYOUT_API bool IsLocalWindowOpened(const UWorld* World);
	IMGUI_UNREALLAYOUT_API bool IsLocalWindowOpened(const UWorld* World, EImGuiLocalPanelMode PanelMode);
	IMGUI_UNREALLAYOUT_API void OpenLocalWindow(UWorld* World);
	IMGUI_UNREALLAYOUT_API void OpenLocalWindow(UWorld* World, EImGuiLocalPanelMode PanelMode);
	IMGUI_UNREALLAYOUT_API void CloseLocalWindow(UWorld* World);
	IMGUI_UNREALLAYOUT_API void CloseLocalWindow(UWorld* World, EImGuiLocalPanelMode PanelMode);
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
	UPROPERTY()
	bool bMaximize = false;
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

	void SaveSettings();
};

UCLASS(Abstract)
class IMGUI_UNREALLAYOUT_API UImGuiLocalPanelManagerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = ImGui)
	FString Title = TEXT("ImGui Panel Manager");

	virtual void DrawMenu() { ReceiveDrawMenu(); }
	UFUNCTION(BlueprintImplementableEvent, Category = ImGui)
	void ReceiveDrawMenu();
	
	virtual void DrawContent() { ReceiveDrawContent(); }
	UFUNCTION(BlueprintImplementableEvent, Category = ImGui)
	void ReceiveDrawContent();
};

UCLASS()
class IMGUI_UNREALLAYOUT_API UImGuiLocalPanelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = ImGui, meta = (WorldContext = WorldContextObject, DevelopmentOnly))
	static bool IsLocalWindowOpened(const UObject* WorldContextObject, EImGuiLocalPanelMode PanelMode)
	{
#if !UE_BUILD_SHIPPING
		return ImGui_WS::LocalPanel::IsLocalWindowOpened(WorldContextObject->GetWorld(), PanelMode);
#else
		return false;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = ImGui, meta = (WorldContext = WorldContextObject, DevelopmentOnly))
	static void OpenLocalWindow(const UObject* WorldContextObject, EImGuiLocalPanelMode Mode)
	{
#if !UE_BUILD_SHIPPING
		ImGui_WS::LocalPanel::OpenLocalWindow(WorldContextObject->GetWorld(), Mode);
#endif
	}

	UFUNCTION(BlueprintCallable, Category = ImGui, meta = (WorldContext = WorldContextObject, DevelopmentOnly))
	static void CloseLocalWindow(const UObject* WorldContextObject, EImGuiLocalPanelMode Mode)
	{
#if !UE_BUILD_SHIPPING
		ImGui_WS::LocalPanel::CloseLocalWindow(WorldContextObject->GetWorld(), Mode);
#endif
	}
};
