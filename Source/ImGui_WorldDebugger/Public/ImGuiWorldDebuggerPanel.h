// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "ImGuiWorldDebuggerPanel.generated.h"

class AImGuiWorldDebuggerBase;

UCLASS(Abstract, Config = ImGuiPanelUserConfig, PerObjectConfig)
class IMGUI_WORLDDEBUGGER_API UImGuiWorldDebuggerPanelBase : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
private:
	bool ShouldCreatePanel(UObject* Owner) const override;
};
