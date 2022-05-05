// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanelBuilder.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ImGuiWorldDebuggerBase.generated.h"

class AImGuiWorldDebuggerBase;

namespace ImGuiWorldDebuggerBootstrap
{
	IMGUI_WS_API extern TSubclassOf<AImGuiWorldDebuggerBase> DebuggerClass;
	IMGUI_WS_API bool GetIsEnable();
	void PostWorldInitialization(UWorld* World, const UWorld::InitializationValues);
}

UCLASS(Transient, DefaultToInstanced, config = ImGuiWorldDebugger, PerObjectConfig)
class IMGUI_WS_API AImGuiWorldDebuggerBase : public AActor
{
	GENERATED_BODY()
public:
	AImGuiWorldDebuggerBase();

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void DrawDebugPanel(float DeltaSeconds);
	
	UPROPERTY(Config)
	uint8 bEnableImGuiWorldDebugger : 1;
	
	UPROPERTY(Config)
	FUnrealImGuiPanelBuilder PanelBuilder;
};
