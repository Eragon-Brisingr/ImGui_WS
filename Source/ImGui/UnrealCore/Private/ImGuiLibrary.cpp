// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiLibrary.h"

#include "imgui_internal.h"
#include "Blueprint/BlueprintExceptionInfo.h"

bool UImGuiLibrary::CheckImGuiContextThrowError()
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
			NSLOCTEXT("ImGui_WS", "CallWithoutContext", "Call without imgui context")
		);
		FBlueprintCoreDelegates::ThrowScriptException(Frame->Object, *Frame, ExceptionInfo);
	}
	return false;
}
