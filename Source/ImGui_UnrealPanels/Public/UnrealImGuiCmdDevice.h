// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Utf8String.h"
#include "UnrealImGuiCmdDevice.generated.h"

/**
 * 
 */
USTRUCT()
struct IMGUI_UNREALPANELS_API FUnrealImGuiCmdDevice
{
	GENERATED_BODY()
public:
	FUnrealImGuiCmdDevice() = default;
	
	void Draw(UObject* Owner);
private:
	FString CmdString;
	int32 SelectedCmdBarIndex = INDEX_NONE;
	int32 SearchedCmdCount = 0;

	struct FCmd
	{
		FUtf8String CmdString;
		FUtf8String CmdHelp;
	};
	
	TArray<FCmd> MatchedCmd;
	void SetCmdString(const FString InCmdString);
	void ClearCmdString();
};
