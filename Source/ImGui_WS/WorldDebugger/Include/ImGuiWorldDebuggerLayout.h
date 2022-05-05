// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiLayout.h"
#include "ImGuiWorldDebuggerLayout.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class IMGUI_WS_API UImGuiWorldDebuggerLayoutBase : public UUnrealImGuiLayoutBase
{
	GENERATED_BODY()
public:
};

UCLASS()
class IMGUI_WS_API UImGuiWorldDebuggerDefaultLayout : public UImGuiWorldDebuggerLayoutBase
{
	GENERATED_BODY()
public:
	enum EDockId
	{
		Viewport,
		Outliner,
		Details,
		Utils,
	};
	UImGuiWorldDebuggerDefaultLayout();
	void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) override;
};
