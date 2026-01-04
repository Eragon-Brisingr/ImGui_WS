// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiLibraryBase.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "UnrealImGuiLayoutLibrary.generated.h"

namespace UnrealImGui
{
	IMGUI_UNREALLAYOUT_API bool IsUnrealPanelOpened(const UObject* WorldContextObject, FName PanelName);
	IMGUI_UNREALLAYOUT_API void OpenUnrealPanel(const UObject* WorldContextObject, FName PanelName);
	IMGUI_UNREALLAYOUT_API void CloseUnrealPanel( const UObject* WorldContextObject, FName PanelName);
	
	IMGUI_UNREALLAYOUT_API bool IsUnrealPanelOpened(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass);
	IMGUI_UNREALLAYOUT_API void OpenUnrealPanel(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass);
	IMGUI_UNREALLAYOUT_API void CloseUnrealPanel( const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass);
}

UCLASS()
class IMGUI_UNREALLAYOUT_API UUnrealImGuiLayoutLibrary : public UImGuiLibraryBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject), BlueprintInternalUseOnly)
	static bool IsUnrealPanelOpened(const UObject* WorldContextObject, FName PanelName)
	{
		if (!CheckImGuiContextThrowError())
		{
			return false;
		}
		return UnrealImGui::IsUnrealPanelOpened(WorldContextObject, PanelName);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject), BlueprintInternalUseOnly)
	static void OpenUnrealPanel(const UObject* WorldContextObject, FName PanelName)
	{
		if (!CheckImGuiContextThrowError())
		{
			return;
		}
		UnrealImGui::OpenUnrealPanel(WorldContextObject, PanelName);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject), BlueprintInternalUseOnly)
	static void CloseUnrealPanel(const UObject* WorldContextObject, FName PanelName)
	{
		if (!CheckImGuiContextThrowError())
		{
			return;
		}
		UnrealImGui::CloseUnrealPanel(WorldContextObject, PanelName);
	}
	
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject, ScriptName = IsUnrealPanelOpened), BlueprintInternalUseOnly)
	static bool IsUnrealPanelOpenedByClass(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
	{
		if (!CheckImGuiContextThrowError())
		{
			return false;
		}
		return UnrealImGui::IsUnrealPanelOpened(WorldContextObject, PanelClass);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject, ScriptName = OpenUnrealPanel), BlueprintInternalUseOnly)
	static void OpenUnrealPanelByClass(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
	{
		if (!CheckImGuiContextThrowError())
		{
			return;
		}
		UnrealImGui::OpenUnrealPanel(WorldContextObject, PanelClass);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Panel", meta = (ImGuiFunction, WorldContext = WorldContextObject, ScriptName = CloseUnrealPanel), BlueprintInternalUseOnly)
	static void CloseUnrealPanelByClass(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelClass)
	{
		if (!CheckImGuiContextThrowError())
		{
			return;
		}
		UnrealImGui::CloseUnrealPanel(WorldContextObject, PanelClass);
	}
};
