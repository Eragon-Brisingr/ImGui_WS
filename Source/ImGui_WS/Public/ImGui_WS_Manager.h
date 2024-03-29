// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "ImGui_WS_Manager.generated.h"

UCLASS()
class IMGUI_WS_API UImGui_WS_Manager : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	static UImGui_WS_Manager* GetChecked();

	static bool IsSettingsEnable();
	bool IsEnable() const { return Impl.IsValid(); }
	void Enable();
	void Disable();
	
	int32 GetPort() const;
	int32 GetConnectionCount() const;

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
	TUniquePtr<FImpl> Impl;
};
