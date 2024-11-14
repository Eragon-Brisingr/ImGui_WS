// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ImGuiLibraryBase.generated.h"

UCLASS(Abstract)
class IMGUI_API UImGuiLibraryBase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:
	static bool CheckImGuiContextThrowError();

	void PostCDOContruct() override;
};
