// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ImGuiWorldDebuggerDrawer.generated.h"

struct FImGuiWorldViewportContext;

/**
 * 
 */
UCLASS(Const, Abstract)
class IMGUI_WS_API UImGuiWorldDebuggerDrawerBase : public UObject
{
	GENERATED_BODY()

public:
	UImGuiWorldDebuggerDrawerBase()
		: bAlwaysDebuggerDraw(false)
	{}
	
	UPROPERTY(Transient)
	TSoftClassPtr<AActor> DrawActor;

	UPROPERTY()
	FLinearColor Color = FColor::Cyan;

	UPROPERTY()
	float Radius = 100.f;
	
	UPROPERTY()
	uint8 bAlwaysDebuggerDraw : 1;
	
	virtual void DrawImGuiDebuggerExtendInfo(const AActor* Actor, const FImGuiWorldViewportContext& DebuggerContext) const {}
	virtual void DrawImGuiDebuggerToolTips(const AActor* Actor) const {}
};

UCLASS(Const)
class IMGUI_WS_API UImGuiWorldDebuggerDrawer_Default : public UImGuiWorldDebuggerDrawerBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerDrawer_Default();
};
