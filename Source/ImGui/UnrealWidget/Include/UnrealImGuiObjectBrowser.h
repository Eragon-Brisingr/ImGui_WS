// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiObjectBrowser.generated.h"

/**
 * 
 */
UCLASS()
class IMGUI_API UUnrealImGuiObjectBrowserPanel : public UUnrealImGuiDefaultPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiObjectBrowserPanel();
	
	UPROPERTY(Transient)
	TObjectPtr<UObject> SelectedObject = nullptr;
	uint32 DockSpaceId = INDEX_NONE;
	UPROPERTY(Config)
	uint8 bDisplayAllProperties : 1;
	UPROPERTY(Config)
	uint8 bEnableEditVisibleProperty : 1;
	
	void Draw(UObject* Owner, float DeltaSeconds) override;
};
