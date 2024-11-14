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
