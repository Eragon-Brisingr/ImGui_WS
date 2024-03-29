// Fill out your copyright notice in the Description page of Project Settings.


#include "BPNode_ImGuiScope.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "UnrealImGuiLibrary.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

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
	Pins.Swap(Pins.IndexOfByKey(OnScopePin), Pins.IndexOfByKey(GetThenPin()));
}

void UBPNode_ImGuiScope::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		return;
	}

	const static FName MD_ImGuiScopeExit = TEXT("ImGuiScopeExit");
	const static FName MD_ImGuiAlwaysExit = TEXT("ImGuiAlwaysExit");
	for (TFieldIterator<UFunction> It{ UUnrealImGuiLibrary::StaticClass() }; It; ++It)
	{
		const UFunction* Function = *It;
		const FString ScopeExitFuncName = Function->GetMetaData(MD_ImGuiScopeExit);
		if (ScopeExitFuncName.Len() == 0)
		{
			continue;
		}
		if (!ensure(CastField<FBoolProperty>(Function->GetReturnProperty())))
		{
			continue;
		}
		ensure(Function->HasMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

		const UFunction* ExitFunction = UUnrealImGuiLibrary::StaticClass()->FindFunctionByName(*ScopeExitFuncName);
		if (!ensure(ExitFunction))
		{
			continue;
		}
		ensure(ExitFunction->HasMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda(
			[Function, ExitFunction, AlwaysExit = Function->HasMetaData(MD_ImGuiAlwaysExit)](UEdGraphNode* NewNode, bool bIsTemplateNode)
		{
			UBPNode_ImGuiScope* ImGuiScope = CastChecked<UBPNode_ImGuiScope>(NewNode);
			ImGuiScope->SetFromFunction(Function);
			ImGuiScope->bAlwaysExit = AlwaysExit;
			ImGuiScope->ExitFunctionReference.SetFromField<UFunction>(ExitFunction, ImGuiScope->GetBlueprintClassFromNode());
		});
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

void UBPNode_ImGuiScope::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::Super::ExpandNode(CompilerContext, SourceGraph);

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
	if (bAlwaysExit)
	{
		SequenceNode->GetThenPinGivenIndex(1)->MakeLinkTo(ExitFunctionNode->GetExecPin());
		CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *ExitFunctionNode->GetThenPin());
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *SequenceNode->GetThenPinGivenIndex(1));
	}

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

#undef LOCTEXT_NAMESPACE
