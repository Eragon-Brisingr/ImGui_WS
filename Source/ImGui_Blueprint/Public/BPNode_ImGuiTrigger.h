// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_CallFunction.h"
#include "BPNode_ImGuiTrigger.generated.h"

UCLASS()
class IMGUI_BLUEPRINT_API UBPNode_ImGuiTrigger : public UK2Node_CallFunction
{
	GENERATED_BODY()

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void AllocateDefaultPins() override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
