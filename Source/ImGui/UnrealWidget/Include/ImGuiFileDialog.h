// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

namespace UnrealImGui
{
	enum class FileDialogType : uint8
	{
		OpenFile,
		SelectFolder
	};

	enum class FileDialogSortOrder : uint8
	{
		Up,
		Down,
		None
	};

	struct IMGUI_API FFileDialogState
	{
		int32 FileSelectIndex = 0;
		int32 FolderSelectIndex = 0;
		FString CurrentPath;
		FString CurrentFile;
		FString FileDialogCurrentFolder;
		FileDialogSortOrder FileNameSortOrder = FileDialogSortOrder::None;
		FileDialogSortOrder SizeSortOrder = FileDialogSortOrder::None;
		FileDialogSortOrder DateSortOrder = FileDialogSortOrder::None;
		FileDialogSortOrder TypeSortOrder = FileDialogSortOrder::None;
		bool InitialPathSet = false;
	};

	struct FUTF8String;
	IMGUI_API void ShowFileDialog(const char* name, FFileDialogState& FileDialogState, FUTF8String& Path, const char* Ext, FileDialogType Type = FileDialogType::OpenFile);
}
