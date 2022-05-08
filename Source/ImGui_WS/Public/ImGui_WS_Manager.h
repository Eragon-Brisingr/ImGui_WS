// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "ImGui_WS_Manager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnImGui_WS_Draw, float);

USTRUCT()
struct IMGUI_WS_API FImGui_WS_Context
{
	GENERATED_BODY()
public:
	FOnImGui_WS_Draw OnDraw;
};

UCLASS()
class UImGui_WS_WorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE; }
	
	UPROPERTY()
	FImGui_WS_Context Context;
};

UCLASS(Config=ImGui_WS, DisplayName = "ImGui WS Settings")
class UImGui_WS_Settings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UImGui_WS_Settings()
	{
		CategoryName = TEXT("Plugins");
		SectionName = TEXT("ImGui_WS_Settings");
	}

	// Editor
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Editor Enable ImGui WS"))
	bool bEditorEnableImGui_WS = true;
	// Packaged DedicatedServer
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Server Enable ImGui WS"))
	bool bServerEnableImGui_WS = true;
	// Packaged Game
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (DisplayName = "Client Enable ImGui WS"))
	bool bClientEnableImGui_WS = false;

	// ImGui-WS Web Port, Only Valid When Pre Game Start. Set In
	// 1. ImGui_WS.ini
	// [/Script/ImGui_WS.ImGui_WS_Settings]
	// Port=8890
	// 2. UE4Editor.exe GAMENAME -ExecCmds="ImGui.WS.Port 8890"
	//
	// When in Editor, Editor Port = Port + 2, DS Port = Port + 1
	UPROPERTY(EditAnywhere, Config, Category = "ImGui WS", meta = (ConfigRestartRequired = true))
	int32 Port = 8890;
};

UCLASS()
class IMGUI_WS_API UImGui_WS_Manager : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	static UImGui_WS_Manager* GetChecked();
	static FImGui_WS_Context* GetImGuiContext(const UWorld* World);
	static FImGui_WS_Context* GetImGuiEditorContext();

	static bool IsSettingsEnable();
	bool IsEnable() const { return Drawer != nullptr; }
	
	int32 GetPort() const;
	int32 GetConnectionCount() const;
protected:
	static constexpr int32 EditorIndex = INDEX_NONE;
	int32 DrawContextIndex = 0;
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
private:
	friend class UImGui_WS_WorldSubsystem;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FImGui_WS_Context EditorContext;
#endif
	
	UPROPERTY(Transient)
	TArray<UImGui_WS_WorldSubsystem*> WorldSubsystems;

	class FDrawer;
	FDrawer* Drawer;
};
