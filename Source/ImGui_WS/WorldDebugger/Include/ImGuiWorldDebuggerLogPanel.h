// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "UnrealImGuiCmdDevice.h"
#include "UnrealImGuiLogDevice.h"
#include "ImGuiWorldDebuggerLogPanel.generated.h"

/**
 * 
 */
UCLASS()
class IMGUI_WS_API UImGuiWorldDebuggerLogPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerLogPanel();
	
	UPROPERTY(Config)
	FUnrealImGuiLogDevice LogDevice;
	UPROPERTY(Config)
	FUnrealImGuiCmdDevice CmdDevice;

	void Register(AImGuiWorldDebuggerBase* WorldDebugger) override;
	void Unregister(AImGuiWorldDebuggerBase* WorldDebugger) override;
	void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) override;
};

