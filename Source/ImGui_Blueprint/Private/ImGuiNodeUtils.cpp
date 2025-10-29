// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiNodeUtils.h"

#include "ImGuiLibraryBase.h"
#include "K2Node_CallFunction.h"
#include "UObject/UObjectHash.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

TArray<UClass*> ImGuiNodeUtils::GetImGuiLibraryClasses()
{
	TArray<UClass*> Classes;
	GetDerivedClasses(UImGuiLibraryBase::StaticClass(), Classes);
	return Classes;
}

FText ImGuiNodeUtils::GetNodeTitle(const UK2Node_CallFunction* FunctionNode, ENodeTitleType::Type TitleType)
{
	const auto DefaultNodeTitle = FunctionNode->UK2Node_CallFunction::GetNodeTitle(TitleType);
	if (UFunction* Function = FunctionNode->GetTargetFunction())
	{
		if (auto OwnerClass = Function->GetOwnerClass())
		{
			static const FName MD_FunctionPrefixOverride = TEXT("FunctionPrefixOverride");
			if (auto FunctionPrefixOverride = OwnerClass->GetMetaData(MD_FunctionPrefixOverride); FunctionPrefixOverride.Len() > 0)
			{
				return FText::Format(LOCTEXT("ImGuiFunctionPrefixOverride", "{0} {1}"), FText::FromString(FunctionPrefixOverride), DefaultNodeTitle);
			}
		}
	}
	return FText::Format(LOCTEXT("ImGuiFunctionPrefix", "ImGui {0}"), DefaultNodeTitle);
}

#undef LOCTEXT_NAMESPACE
