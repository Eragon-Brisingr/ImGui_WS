// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_CallFunction.h"
#include "BPNode_ImGuiFunction.generated.h"

UCLASS()
class IMGUI_BLUEPRINT_API UBPNode_ImGuiFunction : public UK2Node_CallFunction
{
	GENERATED_BODY()

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};
