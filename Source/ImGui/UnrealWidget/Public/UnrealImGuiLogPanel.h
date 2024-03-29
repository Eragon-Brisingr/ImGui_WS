// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiCmdDevice.h"
#include "UnrealImGuiLogDevice.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiLogPanel.generated.h"

UCLASS()
class IMGUI_API UUnrealImGuiLogPanel : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiLogPanel();
	
	UPROPERTY(Config)
	FUnrealImGuiLogDevice LogDevice;
	UPROPERTY(Config)
	FUnrealImGuiCmdDevice CmdDevice;

	void Register(UObject* Owner) override;
	void Unregister(UObject* Owner) override;
	void Draw(UObject* Owner, float DeltaSeconds) override;
};

