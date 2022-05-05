// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnrealImGuiStatDevice.generated.h"

/**
 * 
*/
USTRUCT()
struct IMGUI_API FUnrealImGuiStatDevice
{
	GENERATED_BODY()
public:
	void Draw(UObject* Owner);
};
