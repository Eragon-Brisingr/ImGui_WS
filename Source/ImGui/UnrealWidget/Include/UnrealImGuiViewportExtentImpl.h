// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiViewportExtent.h"
#include "UObject/Object.h"
#include "UnrealImGuiViewportExtentImpl.generated.h"

UCLASS()
class IMGUI_API UUnrealImGuiViewportWorldPartitionExtent : public UUnrealImGuiViewportExtentBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiViewportWorldPartitionExtent();

	void DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty) override;
	void DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext) override;

	UPROPERTY(Config)
	int32 WorldPartitionGridIndex = 0;
	UPROPERTY(Config)
	int32 WorldPartitionGridLevelRange[2] = { 3, 4 };
};

UCLASS()
class IMGUI_API UUnrealImGuiViewportNavMeshExtent : public UUnrealImGuiViewportExtentBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiViewportNavMeshExtent();

	void DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty) override;
	void DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext) override;

	UPROPERTY(Config)
	int32 NavMeshAgentIndex = 0;
};
