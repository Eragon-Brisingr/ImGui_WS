// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ImGuiWorldDebuggerBase.generated.h"

class UUnrealImGuiPanelBuilder;
class AImGuiWorldDebuggerBase;

namespace ImGuiWorldDebuggerBootstrap
{
	IMGUI_WORLDDEBUGGER_API extern TSubclassOf<AImGuiWorldDebuggerBase> DebuggerClass;
	IMGUI_WORLDDEBUGGER_API extern bool bLaunchImGuiWorldDebugger;
	void PostWorldInitialization(UWorld* World, const UWorld::InitializationValues);
	void RequireCreateDebugger();
	void RequireDestroyDebugger();
}

UCLASS(Transient, Config = ImGuiPanelConfig, PerObjectConfig)
class IMGUI_WORLDDEBUGGER_API AImGuiWorldDebuggerBase : public AActor
{
	GENERATED_BODY()
public:
	AImGuiWorldDebuggerBase();

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void DrawDebugPanel(float DeltaSeconds);
	
	UPROPERTY(Config)
	uint8 bEnableImGuiWorldDebugger : 1;
	
	UPROPERTY()
	TObjectPtr<UUnrealImGuiPanelBuilder> PanelBuilder;
};
