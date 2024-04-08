// Fill out your copyright notice in the Description page of Project Settings.

#include "ImGuiFileDialog.h"

#include <filesystem>
#include <sstream>

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_notify.h"
#include "ImGuiEx.h"
#include "UnrealImGuiString.h"
#include "Framework/Application/SlateApplication.h"

namespace UnrealImGui
{
#if PLATFORM_WINDOWS
TArray<char, TInlineAllocator<26>> GetDrivesBitMask();
#endif

void ShowFileDialog(const char* name, FFileDialogState& FileDialogState, FUTF8String& Path, const char* Ext, FileDialogType Type)
{
	if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize(ImVec2(740.0f, 410.0f));
		// Check if there was already something in the buffer. If so, try to use that path (if it exists).
		// If it doesn't exist, just put them int32o the current path.
		if (!FileDialogState.InitialPathSet && Path.IsEmpty() == false)
		{
			auto path = std::filesystem::path(*Path);
			if (std::filesystem::is_directory(path))
			{
				FileDialogState.CurrentPath = *Path;
			}
			else
			{
				// Check if this is just a file in a real path. If so, use the real path.
				// If that still doesn't work, use current path.
				if (std::filesystem::exists(path))
				{
					// It's a file! Take the path and set it.
					FileDialogState.CurrentPath = UTF8_TO_TCHAR(path.remove_filename().string().c_str());
				}
				else
				{
					// An invalid path was entered
					FileDialogState.CurrentPath = UTF8_TO_TCHAR(std::filesystem::current_path().string().c_str());
				}
				FileDialogState.CurrentPath.ReplaceCharInline(TEXT('\\'), TEXT('/'));
			}
			FileDialogState.InitialPathSet = true;
		}

		{
			if (ImGui::Button("Root"))
			{
#if PLATFORM_WINDOWS
				FileDialogState.CurrentFile.Empty();
				FileDialogState.CurrentPath.Empty();
#endif
			}
			int32 LIdx = 0, RIdx = 0;
			while (RIdx != INDEX_NONE && LIdx < FileDialogState.CurrentPath.Len() && FileDialogState.CurrentPath.Len() > 0)
			{
				RIdx = FileDialogState.CurrentPath.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, LIdx);
				const int32 TRIdx = RIdx != INDEX_NONE ? RIdx + 1 : FileDialogState.CurrentPath.Len();
				const FString DirectoryName = FileDialogState.CurrentPath.Mid(LIdx, TRIdx - LIdx);
				ImGui::SameLine();
				if (ImGui::Button(TCHAR_TO_UTF8(*DirectoryName)))
				{
					FileDialogState.CurrentFile.Empty();
					FileDialogState.CurrentPath = FileDialogState.CurrentPath.Mid(0, TRIdx);
				}

				LIdx = RIdx + 1;
			}
		}

		TArray<std::filesystem::directory_entry, TInlineAllocator<16>> Files;
		TArray<std::filesystem::directory_entry, TInlineAllocator<16>> Folders;

		ImGui::BeginChild("Directories##1", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);

		if (FileDialogState.CurrentPath.Len() > 0)
		{
			if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					const FString ParentPath = UTF8_TO_TCHAR(std::filesystem::path{ TCHAR_TO_UTF8(*FileDialogState.CurrentPath) }.parent_path().string().c_str());
					if (FileDialogState.CurrentPath != ParentPath)
					{
						FileDialogState.CurrentFile.Empty();
						FileDialogState.CurrentPath = ParentPath;
						FileDialogState.CurrentPath.ReplaceCharInline(TEXT('\\'), TEXT('/'));
					}
					else
					{
#if PLATFORM_WINDOWS
						FileDialogState.CurrentFile.Empty();
						FileDialogState.CurrentPath.Empty();
#endif
					}
				}
			}
		}
		if (FileDialogState.CurrentPath.IsEmpty())
		{
#if PLATFORM_WINDOWS
			static auto Drives = GetDrivesBitMask();
			for (int32 Idx = 0; Idx < Drives.Num(); ++Idx)
			{
				char DriveCh = Drives[Idx];
				char NewDrive[] = { DriveCh, ':', '/', '\0' };

				if (ImGui::Selectable(NewDrive, Idx == FileDialogState.FolderSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 0)))
				{
					FileDialogState.CurrentFile = "";
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						FileDialogState.CurrentPath = NewDrive;
						FileDialogState.FolderSelectIndex = 0;
						FileDialogState.FileSelectIndex = 0;
						ImGui::SetScrollHereY(0.0f);
						FileDialogState.FileDialogCurrentFolder = "";
					}
					else
					{
						FileDialogState.FolderSelectIndex = Idx;
						FileDialogState.FileDialogCurrentFolder = NewDrive;
					}
				}
			}
#endif
		}
		else
		{
			for (auto& Entry : std::filesystem::directory_iterator(TCHAR_TO_UTF8(*FileDialogState.CurrentPath)))
			{
				if (Entry.is_directory())
				{
					Folders.Add(Entry);
				}
				else
				{
					if (Ext == nullptr || Entry.path().extension().compare(Ext) == 0)
					{
						Files.Add(Entry);
					}
				}
			}

			for (int32 Idx = 0; Idx < Folders.Num(); ++Idx)
			{
				if (ImGui::Selectable(Folders[Idx].path().stem().string().c_str(), Idx == FileDialogState.FolderSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 0)))
				{
					FileDialogState.CurrentFile = "";
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						FileDialogState.CurrentPath = UTF8_TO_TCHAR(Folders[Idx].path().string().c_str());
						FileDialogState.CurrentPath.ReplaceCharInline(TEXT('\\'), TEXT('/'));
						FileDialogState.FolderSelectIndex = 0;
						FileDialogState.FileSelectIndex = 0;
						ImGui::SetScrollHereY(0.0f);
						FileDialogState.FileDialogCurrentFolder = "";
					}
					else
					{
						FileDialogState.FolderSelectIndex = Idx;
						FileDialogState.FileDialogCurrentFolder = UTF8_TO_TCHAR(Folders[Idx].path().stem().string().c_str());
					}
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(Folders[Idx].path().stem().string().c_str());
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Files##1", ImVec2(516, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Columns(4);
		static float initial_spacing_column_0 = 230.0f;
		if (initial_spacing_column_0 > 0)
		{
			ImGui::SetColumnWidth(0, initial_spacing_column_0);
			initial_spacing_column_0 = 0.0f;
		}
		static float initial_spacing_column_1 = 80.0f;
		if (initial_spacing_column_1 > 0)
		{
			ImGui::SetColumnWidth(1, initial_spacing_column_1);
			initial_spacing_column_1 = 0.0f;
		}
		static float initial_spacing_column_2 = 80.0f;
		if (initial_spacing_column_2 > 0)
		{
			ImGui::SetColumnWidth(2, initial_spacing_column_2);
			initial_spacing_column_2 = 0.0f;
		}
		if (ImGui::Selectable("File"))
		{
			FileDialogState.SizeSortOrder = FileDialogSortOrder::None;
			FileDialogState.DateSortOrder = FileDialogSortOrder::None;
			FileDialogState.TypeSortOrder = FileDialogSortOrder::None;
			FileDialogState.FileNameSortOrder = (FileDialogState.FileNameSortOrder == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
		}
		ImGui::NextColumn();
		if (ImGui::Selectable("Size"))
		{
			FileDialogState.FileNameSortOrder = FileDialogSortOrder::None;
			FileDialogState.DateSortOrder = FileDialogSortOrder::None;
			FileDialogState.TypeSortOrder = FileDialogSortOrder::None;
			FileDialogState.SizeSortOrder = (FileDialogState.SizeSortOrder == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
		}
		ImGui::NextColumn();
		if (ImGui::Selectable("Type"))
		{
			FileDialogState.FileNameSortOrder = FileDialogSortOrder::None;
			FileDialogState.DateSortOrder = FileDialogSortOrder::None;
			FileDialogState.SizeSortOrder = FileDialogSortOrder::None;
			FileDialogState.TypeSortOrder = (FileDialogState.TypeSortOrder == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
		}
		ImGui::NextColumn();
		if (ImGui::Selectable("Date"))
		{
			FileDialogState.FileNameSortOrder = FileDialogSortOrder::None;
			FileDialogState.SizeSortOrder = FileDialogSortOrder::None;
			FileDialogState.TypeSortOrder = FileDialogSortOrder::None;
			FileDialogState.DateSortOrder = (FileDialogState.DateSortOrder == FileDialogSortOrder::Down ? FileDialogSortOrder::Up  : FileDialogSortOrder::Down);
		}
		ImGui::NextColumn();
		ImGui::Separator();

		// Sort files
		if (FileDialogState.FileNameSortOrder != FileDialogSortOrder::None)
		{
			Files.Sort([&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
					  {
						  if (FileDialogState.FileNameSortOrder == FileDialogSortOrder::Down)
						  {
							  return a.path().filename().string() > b.path().filename().string();
						  }
						  else
						  {
							  return a.path().filename().string() < b.path().filename().string();
						  }
					  });
		}
		else if (FileDialogState.SizeSortOrder != FileDialogSortOrder::None)
		{
			Files.Sort([&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
					  {
						  if (FileDialogState.SizeSortOrder == FileDialogSortOrder::Down)
						  {
							  return a.file_size() > b.file_size();
						  }
						  else
						  {
							  return a.file_size() < b.file_size();
						  }
					  });
		}
		else if (FileDialogState.TypeSortOrder != FileDialogSortOrder::None)
		{
			Files.Sort([&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
					  {
						  if (FileDialogState.TypeSortOrder == FileDialogSortOrder::Down)
						  {
							  return a.path().extension().string() > b.path().extension().string();
						  }
						  else
						  {
							  return a.path().extension().string() < b.path().extension().string();
						  }
					  });
		}
		else if (FileDialogState.DateSortOrder != FileDialogSortOrder::None)
		{
			Files.Sort([&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
					  {
						  if (FileDialogState.DateSortOrder == FileDialogSortOrder::Down)
						  {
							  return a.last_write_time() > b.last_write_time();
						  }
						  else
						  {
							  return a.last_write_time() < b.last_write_time();
						  }
					  });
		}

		for (int32 Idx = 0; Idx < Files.Num(); ++Idx)
		{
			if (ImGui::Selectable(Files[Idx].path().filename().string().c_str(), Idx == FileDialogState.FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 0)))
			{
				FileDialogState.FileSelectIndex = Idx;
				FileDialogState.CurrentFile = UTF8_TO_TCHAR(Files[Idx].path().filename().string().c_str());
				FileDialogState.FileDialogCurrentFolder = "";
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(Files[Idx].path().filename().string().c_str());
				ImGui::EndTooltip();
			}
			ImGui::NextColumn();
			constexpr float KB_Pre_B = 1024.f;
			constexpr float MB_Pre_B = KB_Pre_B * KB_Pre_B;
			constexpr float GB_Pre_B = MB_Pre_B * KB_Pre_B;
			if (Files[Idx].file_size() > GB_Pre_B)
			{
				ImGui::Text("%.2f GB", Files[Idx].file_size() / GB_Pre_B);
			}
			else if (Files[Idx].file_size() > MB_Pre_B)
			{
				ImGui::Text("%.2f MB", Files[Idx].file_size() / MB_Pre_B);
			}
			else if (Files[Idx].file_size() > KB_Pre_B)
			{
				ImGui::Text("%.2f KB", Files[Idx].file_size() / KB_Pre_B);
			}
			else
			{
				ImGui::Text("%lu B", Files[Idx].file_size());
			}
			ImGui::NextColumn();
			ImGui::TextUnformatted(Files[Idx].path().extension().string().c_str());
			ImGui::NextColumn();
			auto ftime = Files[Idx].last_write_time();
			auto st = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
			std::time_t tt = std::chrono::system_clock::to_time_t(st);
#if PLATFORM_WINDOWS
			auto localtime_r = [](const time_t* const _Time, tm* const _Tm)
			{
				localtime_s(_Tm, _Time);
				return _Tm;
			};
#endif
			std::tm tm;
			std::tm* mt = localtime_r(&tt, &tm);
			std::stringstream ss;
			ss << std::put_time(mt, "%F %R");
			ImGui::TextUnformatted(ss.str().c_str());
			ImGui::NextColumn();
		}
		ImGui::EndChild();

		auto ClosePopup = [&]()
		{
			FileDialogState.FileSelectIndex = 0;
			FileDialogState.FolderSelectIndex = 0;
			FileDialogState.CurrentFile = "";
			FileDialogState.InitialPathSet = false;
			ImGui::CloseCurrentPopup();
		};

		FUTF8String SelectedFilePath = *(FileDialogState.CurrentPath + (FileDialogState.CurrentPath.IsEmpty() || FileDialogState.CurrentPath[FileDialogState.CurrentPath.Len() - 1] == TEXT('/') ? TEXT("") : TEXT("/")) + (FileDialogState.FileDialogCurrentFolder.Len() > 0 ? FileDialogState.FileDialogCurrentFolder : FileDialogState.CurrentFile));
		ImGui::PushItemWidth(ImGui::GetWindowWidth() - 130);
		ImGui::InputText("##SelectedFilePath", SelectedFilePath, ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		if (ImGui::Button("System Browser"))
		{
			const FString SystemBrowserFilePath = [&]
			{
				if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
				{
					TArray<FString> OutFilePaths;
					const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

					if (DesktopPlatform->OpenFileDialog(
						ParentWindowWindowHandle,
						NSLOCTEXT("ImGui_WS", "ImportTooltip", "Select File Import").ToString(),
						FileDialogState.FileDialogCurrentFolder,
						*Path,
						Ext,
						EFileDialogFlags::None,
						OutFilePaths
					))
					{
						return OutFilePaths[0];
					}
				}
				return FString();
			}();
			if (SystemBrowserFilePath.Len() > 0)
			{
				Path = *SystemBrowserFilePath;
				ClosePopup();
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);

		if (ImGui::Button("New folder"))
		{
			ImGui::OpenPopup("NewFolderPopup");
		}
		ImGui::SameLine();

		static bool disable_delete_button = false;
		disable_delete_button = (FileDialogState.FileDialogCurrentFolder == "");
		if (disable_delete_button)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button("Delete folder"))
		{
			ImGui::OpenPopup("DeleteFolderPopup");
		}
		if (disable_delete_button)
		{
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}

		auto GetCurrentPath = [&]
		{
			return FileDialogState.CurrentPath + (FileDialogState.CurrentPath.IsEmpty() || FileDialogState.CurrentPath[FileDialogState.CurrentPath.Len() - 1] == TEXT('/') ? TEXT("") : TEXT("/"));	
		};

		ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal))
		{
			ImGui::Text("Enter a name for the new folder");
			static FUTF8String NewFolderName;
			ImGui::InputText("##newfolder", NewFolderName, sizeof(NewFolderName));
			if (ImGui::Button("Create##1"))
			{
				if (NewFolderName.Len() <= 0)
				{
					ImGui::InsertNotification(ImGuiToastType_Error, "Folder name can't be IsEmpty");
				}
				else
				{
					FString NewFilePath = GetCurrentPath() + *NewFolderName;
					std::filesystem::create_directory(TCHAR_TO_UTF8(*NewFilePath));
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel##1"))
			{
				NewFolderName.Empty();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal))
		{
			ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "Are you sure you want to delete this folder?");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FileDialogState.FileDialogCurrentFolder));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
			if (ImGui::Button("Yes"))
			{
				std::filesystem::remove(TCHAR_TO_UTF8(*(GetCurrentPath() + FileDialogState.FileDialogCurrentFolder)));
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120);

		if (ImGui::Button("Choose"))
		{
			if (Type == FileDialogType::SelectFolder)
			{
				if (FileDialogState.FileDialogCurrentFolder == "")
				{
					ImGui::InsertNotification(ImGuiToastType_Error, "You must select a folder!");
				}
				else
				{
					Path = *(GetCurrentPath() + FileDialogState.FileDialogCurrentFolder);
					ClosePopup();
				}
			}
			else if (Type == FileDialogType::OpenFile)
			{
				if (FileDialogState.CurrentFile == "")
				{
					ImGui::InsertNotification(ImGuiToastType_Error, "You must select a file!");
				}
				else
				{
					Path = *(GetCurrentPath() + FileDialogState.CurrentFile);
					ClosePopup();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ClosePopup();
		}

		ImGui::EndPopup();
	}
}
}

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

TArray<char, TInlineAllocator<26>> UnrealImGui::GetDrivesBitMask()
{
	const DWORD Mask = GetLogicalDrives();
	TArray<char, TInlineAllocator<26>> Drives;
	for(int32 Idx = 0; Idx < 26; ++Idx)
	{
		if(!(Mask & (1 << Idx)))
		{
			continue;
		}
		const char DriveChName = static_cast<char>('A' + Idx);
		const char RootName[4] = { DriveChName, ':', '\\', '\0' };
		const UINT Type = GetDriveTypeA(RootName);
		if(Type == DRIVE_REMOVABLE || Type == DRIVE_FIXED ||  Type == DRIVE_REMOTE)
		{
			Drives.Add(DriveChName);
		}
	}
	return Drives;
}
#endif
