// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLayoutLibrary.h"

#include "ImGuiUnrealLocalPanel.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiPanelBuilder.h"

namespace UnrealImGui
{
extern UUnrealImGuiPanelBuilder* GCurrentDrawBuilder;
namespace LocalPanel
{
	extern TOptional<EImGuiLocalPanelMode> GCurrentLocalPanelMode;
}
	
bool IsUnrealPanelOpened(const UObject* WorldContextObject, FName PanelName)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return false;
	}
	if (GCurrentDrawBuilder)
	{
		return GCurrentDrawBuilder->FindPanel(PanelName) != nullptr;
	}
	else if (LocalPanel::GCurrentLocalPanelMode)
	{
		return LocalPanel::IsLocalPanelOpened(World, *LocalPanel::GCurrentLocalPanelMode, PanelName);
	}
	else if (FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World))
	{
		return LayoutManager->FindPanel(PanelName) != nullptr;
	}
	return false;
}

void OpenUnrealPanel(const UObject* WorldContextObject, FName PanelName)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return;
	}
	if (GCurrentDrawBuilder)
	{
		if (auto Panel = GCurrentDrawBuilder->FindPanel(PanelName, true))
		{
			Panel->SetOpenState(true);
		}
	}
	else if (LocalPanel::GCurrentLocalPanelMode)
	{
		LocalPanel::OpenLocalPanel(World, *LocalPanel::GCurrentLocalPanelMode, PanelName);
	}
	else if (FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World))
	{
		if (auto Panel = LayoutManager->FindPanel(PanelName, true))
		{
			Panel->SetOpenState(true);
		}
	}
}

void CloseUnrealPanel(const UObject* WorldContextObject, FName PanelName)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return;
	}
	if (GCurrentDrawBuilder)
	{
		if (auto Panel = GCurrentDrawBuilder->FindPanel(PanelName))
		{
			Panel->SetOpenState(false);
		}
	}
	else if (LocalPanel::GCurrentLocalPanelMode)
	{
		LocalPanel::CloseLocalPanel(World, *LocalPanel::GCurrentLocalPanelMode, PanelName);
	}
	else if (FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World))
	{
		if (auto Panel = LayoutManager->FindPanel(PanelName))
		{
			Panel->SetOpenState(false);
		}
	}
}

bool IsUnrealPanelOpened(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
{
	if (PanelClass == nullptr)
	{
		return false;
	}
	return IsUnrealPanelOpened(WorldContextObject, PanelClass->GetFName());
}

void OpenUnrealPanel(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
{
	if (PanelClass == nullptr)
	{
		return;
	}
	OpenUnrealPanel(WorldContextObject, PanelClass->GetFName());
}

void CloseUnrealPanel(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
{
	if (PanelClass == nullptr)
	{
		return;
	}
	CloseUnrealPanel(WorldContextObject, PanelClass->GetFName());
}

}
