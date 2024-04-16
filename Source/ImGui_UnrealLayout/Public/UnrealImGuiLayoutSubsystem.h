// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnrealImGuiLayoutSubsystem.generated.h"

class UUnrealImGuiPanelBuilder;

UCLASS()
class IMGUI_UNREALLAYOUT_API UUnrealImGuiLayoutSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static UUnrealImGuiLayoutSubsystem* Get(const UObject* WorldContextObject);

	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE; }

	UPROPERTY()
	TArray<TObjectPtr<UUnrealImGuiPanelBuilder>> PanelBuilders;
};
