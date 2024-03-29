// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_CallFunction.h"
#include "BPNode_ImGuiScope.generated.h"

UCLASS()
class IMGUI_BLUEPRINT_API UBPNode_ImGuiScope : public UK2Node_CallFunction
{
	GENERATED_BODY()

	void AllocateDefaultPins() override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	UPROPERTY()
	bool bAlwaysExit = false;
	UPROPERTY()
	FMemberReference ExitFunctionReference;
};
