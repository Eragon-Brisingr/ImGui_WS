// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiObjectBrowser.generated.h"

/**
 * 
 */
USTRUCT()
struct IMGUI_API FUnrealImGuiObjectBrowser
{
	GENERATED_BODY()
public:
	FUnrealImGuiObjectBrowser()
		: bDisplayAllProperties(true)
		, bEnableEditVisibleProperty(false)
	{}
	
	void Draw(UObject* Owner);

	UPROPERTY(Transient)
	TObjectPtr<UObject> SelectedObject = nullptr;
	uint32 DockSpaceId = INDEX_NONE;
	UPROPERTY(Config)
	uint8 bDisplayAllProperties : 1;
	UPROPERTY(Config)
	uint8 bEnableEditVisibleProperty : 1;
};
