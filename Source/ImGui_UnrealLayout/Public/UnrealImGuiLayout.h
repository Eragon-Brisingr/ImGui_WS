// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiLayout.generated.h"

struct ImGuiContext;
class UUnrealImGuiPanelBuilder;

UCLASS(Abstract)
class IMGUI_UNREALLAYOUT_API UUnrealImGuiLayoutBase : public UObject
{
	GENERATED_BODY()
public:
	UUnrealImGuiLayoutBase()
	{}

	virtual bool ShouldCreateLayout(UObject* Owner) const { unimplemented(); return false; }
	virtual void Register(UObject* Owner, const UUnrealImGuiPanelBuilder& LayoutBuilder);
	virtual void Unregister(UObject* Owner, const UUnrealImGuiPanelBuilder& LayoutBuilder);
	virtual void LoadDefaultLayout(UObject* Owner, const UUnrealImGuiPanelBuilder& LayoutBuilder) {}

	void ApplyPanelDockSettings(const UUnrealImGuiPanelBuilder& LayoutBuilder, const TMap<int32, uint32>& DockIdMap, const int32 DefaultDockId);
	void CreateDockSpace(UObject* Owner, const UUnrealImGuiPanelBuilder& LayoutBuilder);

	FText LayoutName;
protected:
	TMap<void*, uint32> DockSpaceIdMap;

	void WhenImGuiContextDestroyed(ImGuiContext* Context);
};
