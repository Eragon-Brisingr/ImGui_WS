// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerPanel.h"

#include "ImGuiWorldDebuggerBase.h"

void UImGuiWorldDebuggerPanelBase::Register(UObject* Owner)
{
	Register(CastChecked<AImGuiWorldDebuggerBase>(Owner));
}

void UImGuiWorldDebuggerPanelBase::Draw(UObject* Owner, float DeltaSeconds)
{
	Draw(CastChecked<AImGuiWorldDebuggerBase>(Owner), DeltaSeconds);
}

void UImGuiWorldDebuggerPanelBase::Unregister(UObject* Owner)
{
	Unregister(CastChecked<AImGuiWorldDebuggerBase>(Owner));
}
