// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	struct IMGUI_API FUTF8String
	{
		using SizeType = TArray<ANSICHAR>::SizeType;
		
		FUTF8String()
			: Data{ '\0' }
		{}
		FUTF8String(const TCHAR* Text)
		{
			FTCHARToUTF8 TCHARToUTF8{ Text };
			Data.SetNumUninitialized(TCHARToUTF8.Length() + 1);
			FMemory::Memcpy(GetData(), TCHARToUTF8.Get(), TCHARToUTF8.Length() + 1);
		}
		FUTF8String(const FString& Text)
			: FUTF8String(*Text)
		{}

		const ANSICHAR* operator*() const { return GetData(); }
		FString ToString() const { return UTF8_TO_TCHAR(GetData()); }
		void Reset(SizeType NewSize = 0) { Data.Reset(NewSize + 1); Data.Add('\0'); }
		void Empty(SizeType Slack = 0) { Data.Empty(Slack + 1); Data.Add('\0'); }
		bool IsEmpty() const { return Data[0] == '\0'; }
		int32 Len() const { return Data.Num() - 1; }

		ANSICHAR* GetData() { return Data.GetData(); }
		const ANSICHAR* GetData() const { return Data.GetData(); }
		SIZE_T GetAllocatedSize() const { return Data.GetAllocatedSize(); }
		void SetNum(SIZE_T NewNum) { Data.SetNum(NewNum + 1); }
		SIZE_T Max() const { return Data.Max(); }
	private:
		TArray<ANSICHAR> Data;
	};
}
