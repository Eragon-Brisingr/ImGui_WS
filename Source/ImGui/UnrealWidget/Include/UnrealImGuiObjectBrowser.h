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
	void Draw(UObject* Owner);

	UPROPERTY()
	TObjectPtr<UObject> SelectedObject;
	uint32 DockSpaceId = INDEX_NONE;
};
