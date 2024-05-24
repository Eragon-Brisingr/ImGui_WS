// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanel.h"

#include "imgui.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanelBuilder.h"
#include "Engine/World.h"

UUnrealImGuiPanelBase::UUnrealImGuiPanelBase()
	: ImGuiWindowFlags{ ImGuiWindowFlags_None }
{
	Categories.Add(TEXT("Misc"));
}

void UUnrealImGuiPanelBase::SetOpenState(bool bOpen)
{
	UUnrealImGuiPanelBase* Default = GetDefaultObject();
	if (Default->bIsOpen != bOpen)
	{
		Default->bIsOpen = bOpen;
		if (LocalOpenCounter == 0)
		{
			UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
			if (bOpen)
			{
				WhenOpen(Builder->GetOuter(), Builder);
			}
			else
			{
				WhenClose(Builder->GetOuter(), Builder);
			}
		}
	}
}

void UUnrealImGuiPanelBase::LocalPanelOpened()
{
	if (LocalOpenCounter == 0 && GetDefaultObject()->bIsOpen == false)
	{
		UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
		WhenOpen(Builder->GetOuter(), Builder);
	}
	LocalOpenCounter += 1;
}

void UUnrealImGuiPanelBase::LocalPanelClosed()
{
	LocalOpenCounter -= 1;
	if (LocalOpenCounter == 0 && GetDefaultObject()->bIsOpen == false)
	{
		UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
		WhenClose(Builder->GetOuter(), Builder);
	}
}

UUnrealImGuiPanelBase::FScriptExecutionGuard::FScriptExecutionGuard(const UUnrealImGuiPanelBase* Panel)
{
	if (UWorld* World = Panel->GetWorld())
	{
		if (World->IsGameWorld())
		{
			return;
		}
	}
	EditorScriptExecutionGuard = FEditorScriptExecutionGuard{};
}

void UUnrealImGuiPanelBase::DrawWindow(UUnrealImGuiLayoutBase* Layout, UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	UUnrealImGuiPanelBase* Default = GetDefaultObject();
	if (Default->bIsOpen == false)
	{
		return;
	}
	
	bool IsOpen = Default->bIsOpen;
	const FString WindowName = GetLayoutPanelName(Layout->GetName());
	if (ImGui::Begin(TCHAR_TO_UTF8(*WindowName), &IsOpen, ImGuiWindowFlags))
	{
		Draw(Owner, Builder, DeltaSeconds);
	}
	ImGui::End();
	if (Default->bIsOpen != IsOpen)
	{
		GetDefaultObject()->PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
		SetOpenState(IsOpen);
		GetDefaultObject()->SaveConfig();
	}
}

bool UUnrealImGuiPanelBase::ReceiveShouldCreatePanel_Implementation(UObject* Owner) const
{
	return true;
}
