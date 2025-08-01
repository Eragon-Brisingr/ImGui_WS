﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "BPNode_ImGuiFunction.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "ImGuiNodeUtils.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

FText UBPNode_ImGuiFunction::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::Format(LOCTEXT("ImGuiFunctionPrefix", "ImGui {0}"), Super::GetNodeTitle(TitleType));
}

void UBPNode_ImGuiFunction::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		return;
	}

	const static FName MD_ImGuiFunction = TEXT("ImGuiFunction");
	for (UClass* Class : ImGuiNodeUtils::GetImGuiLibraryClasses())
	{
		for (TFieldIterator<UFunction> It{ Class }; It; ++It)
		{
			const UFunction* Function = *It;
			if (Function->HasMetaData(MD_ImGuiFunction) == false)
			{
				continue;
			}
			ensure(Function->GetBoolMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda([Function](UEdGraphNode* NewNode, bool bIsTemplateNode)
			{
				UBPNode_ImGuiFunction* Node = CastChecked<UBPNode_ImGuiFunction>(NewNode);
				Node->SetFromFunction(Function);
			});
			ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
		}
	}
}

#undef LOCTEXT_NAMESPACE
