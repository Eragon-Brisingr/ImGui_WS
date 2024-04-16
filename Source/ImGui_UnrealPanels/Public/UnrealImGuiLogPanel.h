// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiCmdDevice.h"
#include "UnrealImGuiLogDevice.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiLogPanel.generated.h"

UCLASS()
class IMGUI_UNREALPANELS_API UUnrealImGuiLogPanel : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiLogPanel();
	
	UPROPERTY(Config)
	FUnrealImGuiLogDevice LogDevice;
	UPROPERTY(Config)
	FUnrealImGuiCmdDevice CmdDevice;

	void Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;
	void Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;
	void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) override;
};

