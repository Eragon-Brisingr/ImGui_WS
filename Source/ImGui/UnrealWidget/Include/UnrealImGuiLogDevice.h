// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiUtils.h"
#include "UnrealImGuiLogDevice.generated.h"

struct FUnrealImGuiLogDevice;

/**
 * 
 */
namespace UnrealImGui
{
	IMGUI_API const FUTF8String& ToString(ELogVerbosity::Type Verbosity);
	IMGUI_API uint32 ToColor(ELogVerbosity::Type Verbosity);
	
	class IMGUI_API FUnrealImGuiOutputDevice : public FOutputDevice
	{
	public:
		FUnrealImGuiOutputDevice();

		void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category) override;

		struct FLog
		{
			FUTF8String LogString;
			ELogVerbosity::Type Verbosity;
			FName Category;
			FDateTime Time;
			uint64 Frame;
		};
		static constexpr int32 PreChunkLogCount = 1024 * 1024 / sizeof(FLog);
		struct FLogCollections : TChunkedArray<FLog, sizeof(FLog) * PreChunkLogCount>
		{
			void RemoveFirstChunk()
			{
				NumElements -= PreChunkLogCount;
				Chunks.RemoveAt(0);
			}
		};
		FLogCollections Logs;
		TSet<FName> CategoryNames;

		void Register(FUnrealImGuiLogDevice* LogDevice);
		void Unregister(FUnrealImGuiLogDevice* LogDevice);
		TArray<FUnrealImGuiLogDevice*> LogDevices;
		SIZE_T AllLogSize = 0;
		SIZE_T MaxLogSize;
	};

	extern IMGUI_API FUnrealImGuiOutputDevice GUnrealImGuiOutputDevice;
}

USTRUCT()
struct IMGUI_API FUnrealImGuiLogDevice
{
	GENERATED_BODY()
public:
	FUnrealImGuiLogDevice();

	void Register();
	void Unregister();
	void Draw(UObject* Owner);
private:
	UPROPERTY()
	TArray<bool> VerbosityVisibility;
	UPROPERTY()
	TSet<FName> HideCategoryNames;
	UPROPERTY()
	FString FilterString;

	UPROPERTY()
	uint8 bDisplayTime : 1;
	UPROPERTY()
	uint8 bDisplayFrame : 1;
	uint8 bIsFirstDraw : 2;

	int32 DisplayLineIndexOffset = 0;
	TArray<int32> DisplayLines;

	int32 HoveredLogIndex = INDEX_NONE;

	void RefreshDisplayLines();
	void ClearCurrentLines();

	friend class UnrealImGui::FUnrealImGuiOutputDevice;
	int32 StartDisplayLine = 0;
	bool CanLogDisplay(const UnrealImGui::FUnrealImGuiOutputDevice::FLog& Log);
	void PostLogAdded(const UnrealImGui::FUnrealImGuiOutputDevice::FLog& Log, int32 LogLine);
};
