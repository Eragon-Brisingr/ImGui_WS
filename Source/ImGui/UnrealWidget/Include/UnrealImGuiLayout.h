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
	{}

	virtual bool ShouldCreateLayout(UObject* Owner) const { unimplemented(); return false; }
	virtual void Register(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}
	virtual void Unregister(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}
	virtual void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}

	// DefaultDockId = INDEX_NONE mean no dock panel
	void ApplyPanelDockSettings(const FUnrealImGuiPanelBuilder& LayoutBuilder, const TMap<int32, uint32>& DockIdMap, const int32 DefaultDockId);
	void CreateDockSpace(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder);

	FText LayoutName;
protected:
	const uint32 DockSpaceId = INDEX_NONE;
};
