// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	IMGUI_API void DownloadFile(const TCHAR* FileName, const TConstArrayView64<uint8> Data);
	
	DECLARE_DELEGATE_RetVal_TwoParams(bool, FDownloadFileHook, const TCHAR* FileName, const TConstArrayView64<uint8> Data);
}
