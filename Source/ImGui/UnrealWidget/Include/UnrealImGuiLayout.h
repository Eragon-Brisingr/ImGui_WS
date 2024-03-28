// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiLayout.generated.h"

struct ImGuiContext;
struct FUnrealImGuiPanelBuilder;

UCLASS(Abstract)
class IMGUI_API UUnrealImGuiLayoutBase : public UObject
{
	GENERATED_BODY()
public:
	UUnrealImGuiLayoutBase()
	{}

	virtual bool ShouldCreateLayout(UObject* Owner) const { unimplemented(); return false; }
	virtual void Register(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder);
	virtual void Unregister(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder);
	virtual void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) {}

	void ApplyPanelDockSettings(const FUnrealImGuiPanelBuilder& LayoutBuilder, const TMap<int32, uint32>& DockIdMap, const int32 DefaultDockId);
	void CreateDockSpace(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder);

	FText LayoutName;
protected:
	TMap<void*, uint32> DockSpaceIdMap;

	void WhenImGuiContextDestroyed(ImGuiContext* Context);
};
