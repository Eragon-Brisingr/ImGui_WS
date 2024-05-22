﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ImGuiSettings.generated.h"

UENUM()
enum class EImGuiFontGlyphRanges : uint8
{
	// Basic Latin, Extended Latin
	Default,
	// Default + Greek and Coptic
	Greek,
	// Default + Korean characters
	Korean,
	// Default + Hiragana, Katakana, Half-Width, Selection of 2999 Ideographs
	Japanese,
	// Default + Half-Width + Japanese Hiragana/Katakana + full set of about 21000 CJK Unified Ideographs
	ChineseFull,
	// Default + Half-Width + Japanese Hiragana/Katakana + set of 2500 CJK Unified Ideographs for common simplified Chinese
	ChineseSimplifiedCommon,
	// Default + about 400 Cyrillic characters
	Cyrillic,
	// Default + Thai characters
	Thai,
	// Default + Vietnamese characters
	Vietnamese,
};

UCLASS(Config = Game, DefaultConfig, DisplayName = "ImGui WS")
class IMGUI_API UImGuiSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UImGuiSettings()
	{
		CategoryName = TEXT("Plugins");
		SectionName = TEXT("ImGui_WS_Settings");
	}

	// Editor
	// Launch command line add -ExecCmds="ImGui.WS.Enable 1" can enable ImGuiWS
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Editor Enable ImGui WS"))
	bool bEditorEnableImGui_WS = true;
	// ImGui-WS Web Port, Only Valid When Pre Game Start. Set In
	// 1. ImGui_WS.ini
	// [/Script/ImGui_WS.ImGui_WS_Settings]
	// GamePort=8890
	// 2. UE4Editor.exe GAMENAME -ExecCmds="ImGui.WS.Port 8890"
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	int32 EditorPort = 8892;

	// Packaged Server
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Server Enable ImGui WS"))
	bool bServerEnableImGui_WS = false;
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	int32 ServerPort = 8891;

	// Packaged Game
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Game Enable ImGui WS"))
	bool bGameEnableImGui_WS = false;
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	int32 GamePort = 8890;

	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	FString FontName = TEXT("zpix, 12px");
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	FString FontFileName = TEXT("zpix.ttf");
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	float FontSize = 12.f;
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	EImGuiFontGlyphRanges FontGlyphRanges = EImGuiFontGlyphRanges::ChineseFull;

	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	float ServerTickInterval = 1 / 120.f;

	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (AllowedClasses = "/Script/ImGui_UnrealLayout.UnrealImGuiPanelBase"))
	TArray<TSoftClassPtr<UObject>> BlueprintPanels;

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostEditChangeProperty, UImGuiSettings*, FPropertyChangedEvent&);
	FOnPostEditChangeProperty OnPostEditChangeProperty;
	
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		OnPostEditChangeProperty.Broadcast(this, PropertyChangedEvent);
	}
#endif
};
