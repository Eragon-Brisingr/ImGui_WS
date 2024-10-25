// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/EngineSubsystem.h"
#include "ImGui_WS_Manager.generated.h"

UCLASS()
class IMGUI_WS_API UImGui_WS_Manager : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	static UImGui_WS_Manager* GetChecked();

	~UImGui_WS_Manager() override;

	static bool IsSettingsEnable();
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	bool IsEnable() const { return Impl != nullptr; }
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	void Enable();
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	void Disable();
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	int32 GetPort() const;
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	int32 GetConnectionCount() const;
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	void OpenWebPage(bool bServerPort = false) const;

	bool IsRecording() const;
	void StartRecord();
	void StopRecord();
protected:
	int32 DrawContextIndex = 0;
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
private:
	friend class UImGui_WS_WorldSubsystem;

	class FImpl;
	FImpl* Impl = nullptr;
};
