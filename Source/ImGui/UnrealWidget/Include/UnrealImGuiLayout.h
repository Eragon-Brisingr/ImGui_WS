// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiLayout.generated.h"

struct FUnrealImGuiPanelBuilder;

/**
 * 
 */
UCLASS(Abstract)
class IMGUI_API UUnrealImGuiLayoutBase : public UObject
{
	GENERATED_BODY()
public:
	UUnrealImGuiLayoutBase()
		: bIsCheckNewPanelLayout(false)
	{}

	virtual void Register(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}
	virtual void Unregister(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}
	virtual void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}

	void ApplyPanelDockSettings(const FUnrealImGuiPanelBuilder& LayoutBuilder, TMap<int32, uint32> DockIdMap, const int32 DefaultDockId);
	void CreateDockSpace(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder);

	FText LayoutName;
protected:
	uint32 DockSpaceId = INDEX_NONE;
	uint8 bIsCheckNewPanelLayout : 1;
};
