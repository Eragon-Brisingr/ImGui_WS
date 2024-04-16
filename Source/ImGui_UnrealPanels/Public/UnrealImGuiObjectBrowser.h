// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiObjectBrowser.generated.h"

struct ImGuiContext;

UCLASS()
class IMGUI_UNREALPANELS_API UUnrealImGuiObjectBrowserPanel : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiObjectBrowserPanel();
	
	UPROPERTY(Transient)
	TObjectPtr<UObject> SelectedObject = nullptr;
	UPROPERTY(Config)
	uint8 bDisplayAllProperties : 1;
	UPROPERTY(Config)
	uint8 bEnableEditVisibleProperty : 1;

	void Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;
	void Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;

	void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) override;
protected:
	TMap<void*, uint32> DockSpaceIdMap;

	void WhenImGuiContextDestroyed(ImGuiContext* Context);
};
