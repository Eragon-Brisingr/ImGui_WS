// Fill out your copyright notice in the Description page of Project Settings.


#include "BPNode_ImGuiTrigger.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "ImGuiNodeUtils.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

FText UBPNode_ImGuiTrigger::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::Format(LOCTEXT("ImGuiFunctionPrefix", "ImGui {0}"), Super::GetNodeTitle(TitleType));
}

void UBPNode_ImGuiTrigger::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	UEdGraphPin* OnTriggerPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("OnTrigger"));
	OnTriggerPin->PinFriendlyName = LOCTEXT("OnTrigger", "OnTrigger");
	if (UEdGraphPin* ReturnPin = FindPin(UEdGraphSchema_K2::PN_ReturnValue, EGPD_Output))
	{
		OnTriggerPin->PinToolTip = ReturnPin->PinToolTip;
		RemovePin(ReturnPin);
	}
}

void UBPNode_ImGuiTrigger::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		return;
	}

	const static FName MD_ImGuiTrigger = TEXT("ImGuiTrigger");
	for (UClass* Class : ImGuiNodeUtils::GetImGuiLibraryClasses())
	{
		for (TFieldIterator<UFunction> It{ Class }; It; ++It)
		{
			const UFunction* Function = *It;
			if (Function->HasMetaData(MD_ImGuiTrigger) == false)
			{
				continue;
			}
			if (!ensure(CastField<FBoolProperty>(Function->GetReturnProperty())))
			{
				continue;
			}
			ensure(Function->GetBoolMetaData(FBlueprintMetadata::MD_BlueprintInternalUseOnly));

			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda([Function](UEdGraphNode* NewNode, bool bIsTemplateNode)
			{
				UBPNode_ImGuiTrigger* Node = CastChecked<UBPNode_ImGuiTrigger>(NewNode);
				Node->SetFromFunction(Function);
			});
			ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
		}
	}
}

void UBPNode_ImGuiTrigger::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
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

	UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();

	FunctionNode->FindPinChecked(UEdGraphSchema_K2::PN_Then)->MakeLinkTo(SequenceNode->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *SequenceNode->GetThenPinGivenIndex(1));

	UK2Node_IfThenElse* BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	BranchNode->AllocateDefaultPins();
	SequenceNode->GetThenPinGivenIndex(0)->MakeLinkTo(BranchNode->GetExecPin());
	FunctionNode->GetReturnValuePin()->MakeLinkTo(BranchNode->GetConditionPin());
	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(TEXT("OnTrigger"), EGPD_Output), *BranchNode->GetThenPin());
}

#undef LOCTEXT_NAMESPACE
