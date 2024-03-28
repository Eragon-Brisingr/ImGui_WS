// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiPanel.generated.h"

class UUnrealImGuiLayoutBase;

UCLASS(Abstract, config = ImGuiPanelConfig, PerObjectConfig)
class IMGUI_API UUnrealImGuiPanelBase : public UObject
{
	GENERATED_BODY()

public:
	UUnrealImGuiPanelBase();

	int32 ImGuiWindowFlags;
	FText Title;
	struct FDefaultPanelState
	{
		bool bOpen;
		bool bEnableDock;
	};
	struct FDefaultDockLayout : FDefaultPanelState
	{
		FDefaultDockLayout(int32 DockId)
			: FDefaultDockLayout(DockId, true)
		{}
		FDefaultDockLayout(int32 DockId, bool bOpen, bool bEnableDock = true)
			: FDefaultPanelState{ bOpen, bEnableDock }
			, DockId(DockId)
		{}
		FDefaultDockLayout(int32 DockId, const FDefaultPanelState& DefaultPanelState)
			: FDefaultPanelState{ DefaultPanelState }
			, DockId(DockId)
		{}
		int32 DockId;
	};
	FDefaultPanelState DefaultState{ false, true };
	// Key为类型名
	TMap<FName, FDefaultDockLayout> DefaultDockSpace;

	bool IsOpen() const { return bIsOpen; }
	void SetOpenState(bool bOpen);
	FString GetLayoutPanelName(const FString& LayoutName) const { return FString::Printf(TEXT("%s##%s_%s"), *Title.ToString(), *GetClass()->GetName(), *LayoutName); }
protected:
	friend struct FUnrealImGuiPanelBuilder;
	friend class UUnrealImGuiLayoutBase;
	
	UPROPERTY(Transient)
	uint8 bIsOpen : 1;
	UPROPERTY(Config)
	TMap<FName, bool> PanelOpenState;

	virtual bool ShouldCreatePanel(UObject* Owner) const { return true; }

	virtual void Register(UObject* Owner) {}
	virtual void Unregister(UObject* Owner) {}

	virtual void WhenOpen() {}
	virtual void WhenClose() {}
	
	virtual void Draw(UObject* Owner, float DeltaSeconds) {}

	virtual void DrawWindow(UUnrealImGuiLayoutBase* Layout, UObject* Owner, float DeltaSeconds);
};
