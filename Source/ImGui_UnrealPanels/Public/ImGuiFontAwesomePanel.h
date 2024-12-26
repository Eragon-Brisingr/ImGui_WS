// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "ImGuiFontAwesomePanel.generated.h"

UCLASS()
class IMGUI_UNREALPANELS_API UImGuiFontAwesomePanel : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UImGuiFontAwesomePanel();
	
	void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) override;
};
