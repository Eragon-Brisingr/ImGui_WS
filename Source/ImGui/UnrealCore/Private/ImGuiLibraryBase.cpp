// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiLibraryBase.h"

#include "imgui_internal.h"
#include "Blueprint/BlueprintExceptionInfo.h"

bool UImGuiLibraryBase::CheckImGuiContextThrowError()
{
	if (ensure(ImGui::GetCurrentContext()))
	{
		return true;
	}
	FFrame* Frame = FFrame::GetThreadLocalTopStackFrame();
	if (Frame && Frame->Object)
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			NSLOCTEXT("ImGui_WS", "CallWithoutContext", "Call without ImGui context")
		);
		FBlueprintCoreDelegates::ThrowScriptException(Frame->Object, *Frame, ExceptionInfo);
	}
	return false;
}

void UImGuiLibraryBase::PostCDOContruct()
{
	Super::PostCDOContruct();
	
#if WITH_EDITOR
	static const FName MD_CustomThunk(TEXT("CustomThunk"));
	static const FName MD_BlueprintInternalUseOnly(TEXT("BlueprintInternalUseOnly"));
	static const FName MD_ScriptCallable("ScriptCallable");

	// Support script call internal function, for angelscript
	for (TFieldIterator<UFunction> It{ GetClass() }; It; ++It)
	{
		if (!It->GetBoolMetaData(MD_BlueprintInternalUseOnly))
		{
			continue;
		}
		if (It->HasMetaData(MD_CustomThunk))
		{
			continue;
		}
		It->SetMetaData(MD_ScriptCallable, TEXT(""));
	}
#endif
}
