// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiFileUtils.h"

#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace UnrealImGui
{
	IMGUI_API FDownloadFileHook DownloadFileHook;

	void DownloadFile(const TCHAR* FileName, const TConstArrayView64<uint8> Data)
	{
		if (DownloadFileHook.IsBound())
		{
			if (DownloadFileHook.Execute(FileName, Data))
			{
				return;
			}
		}

		const auto DownloadPath = FPaths::ProjectSavedDir() / TEXT("Download") / FileName;
		if (FFileHelper::SaveArrayToFile(Data, *DownloadPath))
		{
			FPlatformProcess::ExploreFolder(*DownloadPath);
		}
	}
}
