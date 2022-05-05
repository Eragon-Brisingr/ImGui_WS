// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "ImGuiWorldDebuggerPanel.generated.h"

/**
 * 
 */
class AImGuiWorldDebuggerBase;

UCLASS(Abstract, config = ImGuiWorldDebugger, PerObjectConfig)
class IMGUI_WS_API UImGuiWorldDebuggerPanelBase : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
private:
	void Register(UObject* Owner) override final;
	void Draw(UObject* Owner, float DeltaSeconds) override final;
	void Unregister(UObject* Owner) override final;

protected:
	virtual void Register(AImGuiWorldDebuggerBase* WorldDebugger) {}
	virtual void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) {}
	virtual void Unregister(AImGuiWorldDebuggerBase* WorldDebugger) {}
};
