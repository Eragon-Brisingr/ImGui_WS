// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "ImGuiWorldDebuggerManager.generated.h"

class UUnrealImGuiPanelBuilder;
class UImGuiWorldDebuggerManager;

namespace ImGuiWorldDebuggerBootstrap
{
	void RequireCreateDebugger();
	void RequireDestroyDebugger();
}

UCLASS(Transient, Config = GameUserSettings)
class IMGUI_WORLDDEBUGGER_API UImGuiWorldDebuggerManager : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static UImGuiWorldDebuggerManager* Get(const UWorld* World) { return World->GetSubsystem<UImGuiWorldDebuggerManager>(); }

	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	void OnWorldBeginPlay(UWorld& InWorld) override;
	void Deinitialize() override;

	void DrawDebugPanel(float DeltaSeconds);

	void EnableDebugger();
	void DisableDebugger();

	uint8 bIsEnable : 1 { false };
	UPROPERTY(Config)
	uint8 bEnableImGuiWorldDebugger : 1 { true };
	
	UPROPERTY()
	TObjectPtr<UUnrealImGuiPanelBuilder> PanelBuilder;
};
