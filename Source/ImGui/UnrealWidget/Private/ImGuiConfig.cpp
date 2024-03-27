// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiConfig.h"

#include "imgui.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/LowLevelMemTracker.h"
#include "HAL/PlatformFileManager.h"

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
static void* ImGui_MemAlloc(size_t Size, void* UserData)
{
	LLM_SCOPE_BYNAME(TEXT("ImGui"));
	return FMemory::Malloc(Size);
}

static void ImGui_MemFree(void* Ptr, void* UserData)
{
	FMemory::Free(Ptr);
}
auto AutoRegisterAllocator = []
{
	ImGui::SetAllocatorFunctions(ImGui_MemAlloc, ImGui_MemFree);
	return true;
}();
#endif

#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
ImFileHandle ImFileOpen(const char* FileName, const char* Mode)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	bool bRead = false;
	bool bWrite = false;
	bool bAppend = false;
	bool bExtended = false;

	for (; *Mode; ++Mode)
	{
		if (*Mode == 'r')
		{
			bRead = true;
		}
		else if (*Mode == 'w')
		{
			bWrite = true;
		}
		else if (*Mode == 'a')
		{
			bAppend = true;
		}
		else if (*Mode == '+')
		{
			bExtended = true;
		}
	}

	if (bWrite || bAppend || bExtended)
	{
		return PlatformFile.OpenWrite(UTF8_TO_TCHAR(FileName), bAppend, bExtended);
	}

	if (bRead)
	{
		return PlatformFile.OpenRead(UTF8_TO_TCHAR(FileName), true);
	}

	return nullptr;
}

bool ImFileClose(ImFileHandle File)
{
	if (!File)
	{
		return false;
	}

	delete File;
	return true;
}

uint64 ImFileGetSize(ImFileHandle File)
{
	if (!File)
	{
		return MAX_uint64;
	}

	const uint64 FileSize = File->Size();
	return FileSize;
}

uint64 ImFileRead(void* Data, uint64 Size, uint64 Count, ImFileHandle File)
{
	if (!File)
	{
		return 0;
	}

	const int64 StartPos = File->Tell();
	File->Read(static_cast<uint8*>(Data), Size * Count);

	const uint64 ReadSize = File->Tell() - StartPos;
	return ReadSize;
}

uint64 ImFileWrite(const void* Data, uint64 Size, uint64 Count, ImFileHandle File)
{
	if (!File)
	{
		return 0;
	}

	const int64 StartPos = File->Tell();
	File->Write(static_cast<const uint8*>(Data), Size * Count);

	const uint64 WriteSize = File->Tell() - StartPos;
	return WriteSize;
}
#endif
