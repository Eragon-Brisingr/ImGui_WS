// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace ENodeTitleType
{
	enum Type : int;
}

class UK2Node_CallFunction;

namespace ImGuiNodeUtils
{
	TArray<UClass*> GetImGuiLibraryClasses();
	FText GetNodeTitle(const UK2Node_CallFunction* FunctionNode, ENodeTitleType::Type TitleType);
}
