// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ImGuiSettings.generated.h"

class UUserWidget;

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

UCLASS(Config = EditorPerProjectUserSettings)
class IMGUI_API UImGuiPerUserSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = PerUserSettings, meta = (ConfigRestartRequired = true))
	bool bOverrideDPIScale = false;
	UPROPERTY(EditAnywhere, Config, Category = PerUserSettings, meta = (ConfigRestartRequired = true, EditCondition = bOverrideDPIScale))
	float OverrideDPIScale = 1.5f;

	UPROPERTY(Config)
	TMap<TSoftClassPtr<UObject>, float> CustomPanelDPIScaleMap;

	UPROPERTY(Config)
	int32 StyleIndex = 0;

	UPROPERTY(EditAnywhere, Config, Category = PerUserSettings)
	int32 MaxRecordRecentlyNum = 8;

	UPROPERTY(Config)
	TArray<TSoftClassPtr<UObject>> RecentlyOpenPanels;

	void RecordRecentlyOpenPanel(const TSoftClassPtr<UObject>& PanelClass);

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

UCLASS(Config = Game, DefaultConfig, DisplayName = "ImGui WS")
class IMGUI_API UImGuiSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UImGuiSettings();

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

	UPROPERTY(VisibleDefaultsOnly, Category = "ImGui WS", meta = (EditInline))
	TObjectPtr<UImGuiPerUserSettings> PreUserSettings = nullptr;

	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (AllowedClasses = "/Script/ImGui_UnrealLayout.ImGuiLocalPanelManagerWidget"))
	TSoftClassPtr<UObject> LocalPanelManagerWidget;

	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (AllowedClasses = "/Script/ImGui_UnrealLayout.UnrealImGuiPanelBase"))
	TArray<TSoftClassPtr<UObject>> FavoritePanels;

	// Support Non-ASCII paths, will enable pre-thread locale and set ImGui_WS thread locale as UTF-8
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	bool bEnableThreadLocaleAsUft8 = false;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	bool bDisplayToolbarButton = false;
#endif

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostEditChangeProperty, UImGuiSettings*, FPropertyChangedEvent&);
	static FOnPostEditChangeProperty OnPostEditChangeProperty;
	
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
