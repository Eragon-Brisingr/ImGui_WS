﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiPanel.generated.h"

class UUnrealImGuiLayoutBase;

USTRUCT()
struct FImGuiDefaultPanelState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	bool bOpen{ false };
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bEnableDock{ true };
};

USTRUCT()
struct FImGuiDefaultDockLayout : public FImGuiDefaultPanelState
{
	GENERATED_BODY()

	FImGuiDefaultDockLayout() = default;
	FImGuiDefaultDockLayout(int32 DockId)
		: FImGuiDefaultDockLayout(DockId, true)
	{}
	FImGuiDefaultDockLayout(int32 DockId, bool bOpen, bool bEnableDock = true)
		: FImGuiDefaultPanelState{ bOpen, bEnableDock }
		, DockId(DockId)
	{}
	FImGuiDefaultDockLayout(int32 DockId, const FImGuiDefaultPanelState& DefaultPanelState)
		: FImGuiDefaultPanelState{ DefaultPanelState }
		, DockId(DockId)
	{}

	UPROPERTY(EditAnywhere, Category = Settings)
	int32 DockId = 0;
};

UCLASS(Abstract, Config = ImGuiPanelUserConfig, PerObjectConfig, Blueprintable)
class IMGUI_UNREALLAYOUT_API UUnrealImGuiPanelBase : public UObject
{
	GENERATED_BODY()

public:
	UUnrealImGuiPanelBase();

	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"), Category = Settings)
	int32 ImGuiWindowFlags;
	UPROPERTY(EditDefaultsOnly, Category = Settings)
	FName Title;
	UPROPERTY(EditDefaultsOnly, Category = Settings)
	TArray<FName> Categories;
	UPROPERTY(EditDefaultsOnly, Category = Settings)
	FImGuiDefaultPanelState DefaultState{ false, true };
	// Key为类型名
	UPROPERTY(EditDefaultsOnly, Category = Settings)
	TMap<FName, FImGuiDefaultDockLayout> DefaultDockSpace;

	bool IsOpen() const { return GetConfigObject()->bIsOpen; }
	void SetOpenState(bool bOpen);
	FString GetLayoutPanelName(const FString& LayoutName) const { return FString::Printf(TEXT("%s##%s_%s"), *Title.ToString(), *GetClass()->GetName(), *LayoutName); }

	void LocalPanelOpened();
	void LocalPanelClosed();
	
	UUnrealImGuiPanelBase* GetConfigObject() const;

private:
	friend class UUnrealImGuiPanelBuilder;
	friend class UUnrealImGuiLayoutBase;

	uint8 bIsOpen : 1 { false };
	uint8 LocalOpenCounter : 7 { 0 };
	UPROPERTY(Config)
	TMap<FName, bool> PanelOpenState;
public:
	struct FScriptExecutionGuard
	{
		FScriptExecutionGuard(const UUnrealImGuiPanelBase* Panel);
		TOptional<FEditorScriptExecutionGuard> EditorScriptExecutionGuard;
	};
	
	virtual bool ShouldCreatePanel(UObject* Owner) const { FScriptExecutionGuard ScriptExecutionGuard{ this }; return ReceiveShouldCreatePanel(Owner); }

	virtual void Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) { FScriptExecutionGuard ScriptExecutionGuard{ this }; ReveiveRegister(Owner, Builder); }
	virtual void Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) { FScriptExecutionGuard ScriptExecutionGuard{ this }; ReveiveUnregister(Owner, Builder); }

	virtual void WhenOpen(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) { FScriptExecutionGuard ScriptExecutionGuard{ this }; ReveiveWhenOpen(Owner, Builder); }
	virtual void WhenClose(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) { FScriptExecutionGuard ScriptExecutionGuard{ this }; ReveiveWhenClose(Owner, Builder); }
	
	virtual void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) { FScriptExecutionGuard ScriptExecutionGuard{ this }; ReveiveDraw(Owner, Builder, DeltaSeconds); }
	virtual void DrawWindow(UUnrealImGuiLayoutBase* Layout, UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds);
protected:
	UFUNCTION(BlueprintNativeEvent)
	bool ReceiveShouldCreatePanel(UObject* Owner) const;
	UFUNCTION(BlueprintImplementableEvent)
	void ReveiveRegister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder);
	UFUNCTION(BlueprintImplementableEvent)
	void ReveiveUnregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder);
	UFUNCTION(BlueprintImplementableEvent)
	void ReveiveWhenOpen(UObject* Owner, UUnrealImGuiPanelBuilder* Builder);
	UFUNCTION(BlueprintImplementableEvent)
	void ReveiveWhenClose(UObject* Owner, UUnrealImGuiPanelBuilder* Builder);
	UFUNCTION(BlueprintImplementableEvent)
	void ReveiveDraw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds);
};
