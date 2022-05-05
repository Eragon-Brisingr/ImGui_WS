// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace UnrealImGui
{
	struct IMGUI_API FUTF8String
	{
		FUTF8String() = default;
		FUTF8String(const TCHAR* Message)
		{
			FTCHARToUTF8 TCHARToUTF8{ Message };
			UTF8RawArray.SetNumUninitialized(TCHARToUTF8.Length() + 1);
			FMemory::Memcpy(UTF8RawArray.GetData(), TCHARToUTF8.Get(), TCHARToUTF8.Length() + 1);
		}

		const ANSICHAR* operator*() const
		{
			return UTF8RawArray.GetData();
		}

		SIZE_T GetAllocatedSize() const
		{
			return UTF8RawArray.GetAllocatedSize();
		}
	private:
		TArray<ANSICHAR> UTF8RawArray;  
	};
	
	struct IMGUI_API FWidgetDisableScope
	{
		FWidgetDisableScope(bool IsDisable);
		~FWidgetDisableScope();
	private:
		bool Disable;
	};
}