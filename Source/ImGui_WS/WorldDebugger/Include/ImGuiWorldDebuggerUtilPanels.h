// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "UnrealImGuiStatDevice.h"
#include "ImGuiWorldDebuggerUtilPanels.generated.h"

/**
 * 
 */
UCLASS()
class IMGUI_WS_API UImGuiWorldDebuggerStatPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerStatPanel();
	
	UPROPERTY()
	FUnrealImGuiStatDevice StatDevice;

	void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) override;
};