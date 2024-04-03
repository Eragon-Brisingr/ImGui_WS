// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerPanel.h"

#include "ImGuiWorldDebuggerBase.h"

bool UImGuiWorldDebuggerPanelBase::ShouldCreatePanel(UObject* Owner) const
{
	return Owner && Owner->IsA<AImGuiWorldDebuggerBase>() && ReceiveShouldCreatePanel(Owner);
}
