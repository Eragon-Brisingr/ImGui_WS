// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnrealImGuiLayoutSubsystem.generated.h"

class UUnrealImGuiPanelBuilder;

USTRUCT()
struct IMGUI_UNREALLAYOUT_API FUnrealImGuiLayoutManager
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<UUnrealImGuiPanelBuilder>> PanelBuilders;
	
	static FUnrealImGuiLayoutManager* Get(const UObject* WorldContextObject);
};

UCLASS()
class IMGUI_UNREALLAYOUT_API UUnrealImGuiLayoutSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE; }

	UPROPERTY()
	FUnrealImGuiLayoutManager Context;
};
