// Fill out your copyright notice in the Description page of Project Settings.


#include "BPNode_ImGuiScope.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "ImGuiNodeUtils.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

FText UBPNode_ImGuiScope::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::Format(LOCTEXT("ImGuiFunctionPrefix", "ImGui {0}"), Super::GetNodeTitle(TitleType));
}

void UBPNode_ImGuiScope::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	UEdGraphPin* OnScopePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("OnScope"));
	OnScopePin->PinFriendlyName = LOCTEXT("OnScope", "OnScope");
	if (UEdGraphPin* ReturnPin = FindPin(UEdGraphSchema_K2::PN_ReturnValue, EGPD_Output))
	{
		OnScopePin->PinToolTip = ReturnPin->PinToolTip;
		RemovePin(ReturnPin);
	}
	if (UEdGraphPin* ThenPin = GetThenPin())
	{
		Pins.Swap(Pins.IndexOfByKey(OnScopePin), Pins.IndexOfByKey(ThenPin));
	}
}

void UBPNode_ImGuiScope::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		return;
	}

	const static FName MD_ImGuiScopeExit = TEXT("ImGuiScopeExit");
	for (UClass* Class : ImGuiNodeUtils::GetImGuiLibraryClasses())
	{
		for (TFieldIterator<UFunction> It{ Class }; It; ++It)
		{
			const UFunction* Function = *It;
			const FString ScopeExitFuncName = Function->GetMetaData(MD_ImGuiScopeExit);
			if (ScopeExitFuncName.Len() == 0)
			{
				continue;
			}
			ensure(Function->GetBoolMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

			const UFunction* ExitFunction = Class->FindFunctionByName(*ScopeExitFuncName);
			if (!ensure(ExitFunction))
			{
				continue;
			}
			ensure(ExitFunction->GetBoolMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda(
				[Function, ExitFunction](UEdGraphNode* NewNode, bool bIsTemplateNode)
			{
				UBPNode_ImGuiScope* Node = CastChecked<UBPNode_ImGuiScope>(NewNode);
				Node->SetFromFunction(Function);
				Node->ExitFunctionReference.SetFromField<UFunction>(ExitFunction, Node->GetBlueprintClassFromNode());
			});
			ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
		}
	}
}

void UBPNode_ImGuiScope::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::Super::ExpandNode(CompilerContext, SourceGraph);

	const UFunction* Function = GetTargetFunction();
	if (Function == nullptr)
	{
		BreakAllNodeLinks();
		return;
	}

	UK2Node_CallFunction* FunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	FunctionNode->FunctionReference = FunctionReference;
	FunctionNode->AllocateDefaultPins();

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != EGPD_Input)
		{
			continue;
		}
		UEdGraphPin* IntermediatePin = FunctionNode->FindPin(Pin->GetFName(), EGPD_Input);
		if (!ensure(IntermediatePin))
		{
			continue;
		}
		CompilerContext.MovePinLinksToIntermediate(*Pin, *IntermediatePin);
	}
	UK2Node_CallFunction* ExitFunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ExitFunctionNode->FunctionReference = ExitFunctionReference;
	ExitFunctionNode->AllocateDefaultPins();

	UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();
	FunctionNode->FindPinChecked(UEdGraphSchema_K2::PN_Then)->MakeLinkTo(SequenceNode->GetExecPin());

	const bool bBoolReturn = CastField<FBoolProperty>(Function->GetReturnProperty()) != nullptr;
	const static FName MD_ImGuiAlwaysExit = TEXT("ImGuiAlwaysExit");
	const bool bAlwaysExit = bBoolReturn == false || Function->HasMetaData(MD_ImGuiAlwaysExit);
	if (bAlwaysExit)
	{
		SequenceNode->GetThenPinGivenIndex(1)->MakeLinkTo(ExitFunctionNode->GetExecPin());
		CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *ExitFunctionNode->GetThenPin());
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *SequenceNode->GetThenPinGivenIndex(1));
	}

	if (bBoolReturn)
	{
		UK2Node_IfThenElse* BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
		BranchNode->AllocateDefaultPins();
		SequenceNode->GetThenPinGivenIndex(0)->MakeLinkTo(BranchNode->GetExecPin());
		FunctionNode->GetReturnValuePin()->MakeLinkTo(BranchNode->GetConditionPin());
		if (bAlwaysExit)
		{
			CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(TEXT("OnScope"), EGPD_Output), *BranchNode->GetThenPin());
		}
		else
		{
			UK2Node_ExecutionSequence* SequenceNodeExit = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
			SequenceNodeExit->AllocateDefaultPins();

			BranchNode->GetThenPin()->MakeLinkTo(SequenceNodeExit->GetExecPin());
			CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(TEXT("OnScope"), EGPD_Output), *SequenceNodeExit->GetThenPinGivenIndex(0));
			SequenceNodeExit->GetThenPinGivenIndex(1)->MakeLinkTo(ExitFunctionNode->GetExecPin());
		}
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(TEXT("OnScope"), EGPD_Output), *SequenceNode->GetThenPinGivenIndex(0));
	}
}

#undef LOCTEXT_NAMESPACE
