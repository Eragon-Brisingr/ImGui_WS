// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiPanel.generated.h"

class UUnrealImGuiLayoutBase;

/**
 * 
 */
UCLASS(Abstract, config = ImGuiPanelConfig, PerObjectConfig)
class IMGUI_API UUnrealImGuiPanelBase : public UObject
{
	GENERATED_BODY()

public:
	UUnrealImGuiPanelBase();

	int32 ImGuiWindowFlags;
	FText Title;
	// 没有Dock，默认漂浮状态
	static constexpr int32 NoDock = -1;
	// Key为类型名,Value为DockId
	TMap<FName, int32> DefaultDockSpace;

	UPROPERTY(Transient)
	uint8 bIsOpen : 1;
	UPROPERTY(Config)
	TMap<FName, bool> PanelOpenState;

	FString GetLayoutPanelName(const FString& LayoutName) const { return FString::Printf(TEXT("%s##%s"), *Title.ToString(), *LayoutName); }

	virtual void Register(UObject* Owner) {}
	virtual void Draw(UObject* Owner, float DeltaSeconds) {}
	virtual void Unregister(UObject* Owner) {}

	virtual void DrawWindow(UUnrealImGuiLayoutBase* Layout, UObject* Owner, float DeltaSeconds);
};
