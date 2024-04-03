// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanel.h"

#include "imgui.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanelBuilder.h"

UUnrealImGuiPanelBase::UUnrealImGuiPanelBase()
	: ImGuiWindowFlags(ImGuiWindowFlags_None)
	, bIsOpen(false)
{
	Categories.Add(NSLOCTEXT("ImGui_WS", "MiscCategory", "Misc"));
}

void UUnrealImGuiPanelBase::SetOpenState(bool bOpen)
{
	if (bIsOpen != bOpen)
	{
		bIsOpen = bOpen;
		UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
		if (bIsOpen)
		{
			WhenOpen(Builder->GetOuter(), Builder);
		}
		else
		{
			WhenClose(Builder->GetOuter(), Builder);
		}
	}
}

void UUnrealImGuiPanelBase::DrawWindow(UUnrealImGuiLayoutBase* Layout, UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	if (bIsOpen == false)
	{
		return;
	}
	
	bool IsOpen = bIsOpen;
	const FString WindowName = GetLayoutPanelName(Layout->GetName());
	if (ImGui::Begin(TCHAR_TO_UTF8(*WindowName), &IsOpen, ImGuiWindowFlags))
	{
		Draw(Owner, Builder, DeltaSeconds);
	}
	ImGui::End();
	if (bIsOpen != IsOpen)
	{
		PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
		SetOpenState(IsOpen);
		SaveConfig();
	}
}

bool UUnrealImGuiPanelBase::ReceiveShouldCreatePanel_Implementation(UObject* Owner) const
{
	return true;
}
