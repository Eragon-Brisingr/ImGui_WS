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
	UUnrealImGuiPanelBase* ConfigObject = ConfigObjectPrivate;
	if (ConfigObject->bIsOpen != bOpen)
	{
		ConfigObject->bIsOpen = bOpen;
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
	if (LocalOpenCounter == 0 && ConfigObjectPrivate->bIsOpen == false)
	{
		UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
		WhenOpen(Builder->GetOuter(), Builder);
	}
	LocalOpenCounter += 1;
}

void UUnrealImGuiPanelBase::LocalPanelClosed()
{
	LocalOpenCounter -= 1;
	if (LocalOpenCounter == 0 && ConfigObjectPrivate->bIsOpen == false)
	{
		UUnrealImGuiPanelBuilder* Builder = CastChecked<UUnrealImGuiPanelBuilder>(GetOuter());
		WhenClose(Builder->GetOuter(), Builder);
	}
}

void UUnrealImGuiPanelBase::InitialConfigObject()
{
	if (UWorld* World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			ConfigObjectPrivate = GetClass()->GetDefaultObject<UUnrealImGuiPanelBase>();
			return;
		}
	}
	ConfigObjectPrivate = this;
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
	UUnrealImGuiPanelBase* ConfigObject = ConfigObjectPrivate;
	if (ConfigObject->bIsOpen == false)
	{
		return;
	}
	
	bool IsOpen = ConfigObject->bIsOpen;
	const FString WindowName = GetLayoutPanelName(Layout->GetName());
	if (ImGui::Begin(TCHAR_TO_UTF8(*WindowName), &IsOpen, ImGuiWindowFlags))
	{
		Draw(Owner, Builder, DeltaSeconds);
	}
	ImGui::End();
	if (ConfigObject->bIsOpen != IsOpen)
	{
		ConfigObject->PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
		SetOpenState(IsOpen);
		ConfigObject->SaveConfig();
	}
}

bool UUnrealImGuiPanelBase::ReceiveShouldCreatePanel_Implementation(UObject* Owner) const
{
	return true;
}
