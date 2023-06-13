// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "ImGuiWorldDebuggerDetailsPanel.generated.h"

UCLASS()
class IMGUI_WS_API UImGuiWorldDebuggerDetailsPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerDetailsPanel();
	
	void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) override;
	
	UPROPERTY(Config)
	uint8 bDisplayAllProperties : 1;
	UPROPERTY(Config)
	uint8 bEnableEditVisibleProperty : 1;
};
