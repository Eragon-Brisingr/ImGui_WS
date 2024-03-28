// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	struct IMGUI_API FUTF8String : private TArray<ANSICHAR>
	{
		using Super = TArray<ANSICHAR>;
		FUTF8String()
			: Super{ '\0' }
		{}
		FUTF8String(const TCHAR* Text)
		{
			FTCHARToUTF8 TCHARToUTF8{ Text };
			SetNumUninitialized(TCHARToUTF8.Length() + 1);
			FMemory::Memcpy(GetData(), TCHARToUTF8.Get(), TCHARToUTF8.Length() + 1);
		}
		FUTF8String(const FString& Text)
			: FUTF8String(*Text)
		{}

		const ANSICHAR* operator*() const { return GetData(); }
		FString ToString() const { return UTF8_TO_TCHAR(GetData()); }
		void Reset(SizeType NewSize = 0) { Super::Reset(NewSize + 1); Add('\0'); }
		void Empty(SizeType Slack = 0) { Super::Empty(Slack + 1); Add('\0'); }
		bool IsEmpty() const { return (*this)[0] == '\0'; }
		int32 Len() const { return Num() - 1; }

		ANSICHAR* GetData() { return Super::GetData(); }
		const ANSICHAR* GetData() const { return Super::GetData(); }
		SIZE_T GetAllocatedSize() const { return Super::GetAllocatedSize(); }
		void SetNum(SIZE_T NewNum) { Super::SetNum(NewNum + 1); }
		SIZE_T Max() const { return Super::Max(); }
	};
}
