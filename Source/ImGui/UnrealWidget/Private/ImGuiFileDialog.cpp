// Fill out your copyright notice in the Description page of Project Settings.

#include "ImGuiFileDialog.h"
#include <filesystem>
#include <sstream>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_notify.h"
#include "UnrealImGuiUtils.h"

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
		// If it doesn't exist, just put them into the current path.
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
					FileDialogState.CurrentPath = path.remove_filename().string();
				}
				else
				{
					// An invalid path was entered
					FileDialogState.CurrentPath = std::filesystem::current_path().string();
				}
				std::replace(FileDialogState.CurrentPath.begin(), FileDialogState.CurrentPath.end(), '\\', '/');
			}
			FileDialogState.InitialPathSet = true;
		}

		{
			if (ImGui::Button("Root"))
			{
#if PLATFORM_WINDOWS
				FileDialogState.CurrentFile.clear();
				FileDialogState.CurrentPath.clear();
#endif
			}
			int32 LIdx = 0, RIdx = 0;
			while (RIdx != std::string::npos && LIdx < FileDialogState.CurrentPath.size() && FileDialogState.CurrentPath.size() > 0)
			{
				RIdx = FileDialogState.CurrentPath.find('/', LIdx);
				const int32 TRIdx = RIdx != std::string::npos ? RIdx + 1 : FileDialogState.CurrentPath.size();
				const std::string DirectoryName = FileDialogState.CurrentPath.substr(LIdx, TRIdx - LIdx);
				ImGui::SameLine();
				if (ImGui::Button(DirectoryName.c_str()))
				{
					FileDialogState.CurrentFile.clear();
					FileDialogState.CurrentPath = FileDialogState.CurrentPath.substr(0, TRIdx);
				}

				LIdx = RIdx + 1;
			}
		}

		std::vector<std::filesystem::directory_entry> Files;
		std::vector<std::filesystem::directory_entry> Folders;

		ImGui::BeginChild("Directories##1", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);

		if (FileDialogState.CurrentPath.size() > 0)
		{
			if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					const std::string ParentPath = std::filesystem::path{ FileDialogState.CurrentPath }.parent_path().string();
					if (FileDialogState.CurrentPath != ParentPath)
					{
						FileDialogState.CurrentFile.clear();
						FileDialogState.CurrentPath = ParentPath;
						std::replace(FileDialogState.CurrentPath.begin(), FileDialogState.CurrentPath.end(), '\\', '/');
					}
					else
					{
#if PLATFORM_WINDOWS
						FileDialogState.CurrentFile.clear();
						FileDialogState.CurrentPath.clear();
#endif
					}
				}
			}
		}
		if (FileDialogState.CurrentPath.empty())
		{
#if PLATFORM_WINDOWS
			static auto Drives = GetDrivesBitMask();
			for (int32 Idx = 0; Idx < Drives.Num(); ++Idx)
			{
				char DriveCh = Drives[Idx];
				char NewDrive[] = { DriveCh, ':', '/', '\0' };

				if (ImGui::Selectable(NewDrive, Idx == FileDialogState.FolderSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
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
			for (auto& p : std::filesystem::directory_iterator(FileDialogState.CurrentPath))
			{
				if (p.is_directory())
				{
					Folders.push_back(p);
				}
				else
				{
					if (Ext == nullptr || p.path().extension().compare(Ext) == 0)
					{
						Files.push_back(p);
					}
				}
			}

			for (int i = 0; i < Folders.size(); ++i)
			{
				if (ImGui::Selectable(Folders[i].path().stem().string().c_str(), i == FileDialogState.FolderSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
				{
					FileDialogState.CurrentFile = "";
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						FileDialogState.CurrentPath = Folders[i].path().string();
						std::replace(FileDialogState.CurrentPath.begin(), FileDialogState.CurrentPath.end(), '\\', '/');
						FileDialogState.FolderSelectIndex = 0;
						FileDialogState.FileSelectIndex = 0;
						ImGui::SetScrollHereY(0.0f);
						FileDialogState.FileDialogCurrentFolder = "";
					}
					else
					{
						FileDialogState.FolderSelectIndex = i;
						FileDialogState.FileDialogCurrentFolder = Folders[i].path().stem().string();
					}
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(Folders[i].path().stem().string().c_str());
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
			std::sort(Files.begin(), Files.end(),
			          [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
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
			std::sort(Files.begin(), Files.end(),
			          [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
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
			std::sort(Files.begin(), Files.end(),
			          [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
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
			std::sort(Files.begin(), Files.end(),
			          [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
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

		for (int i = 0; i < Files.size(); ++i)
		{
			if (ImGui::Selectable(Files[i].path().filename().string().c_str(), i == FileDialogState.FileSelectIndex, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
			{
				FileDialogState.FileSelectIndex = i;
				FileDialogState.CurrentFile = Files[i].path().filename().string();
				FileDialogState.FileDialogCurrentFolder = "";
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(Files[i].path().filename().string().c_str());
				ImGui::EndTooltip();
			}
			ImGui::NextColumn();
			constexpr float KB_Pre_B = 1024.f;
			constexpr float MB_Pre_B = KB_Pre_B * KB_Pre_B;
			constexpr float GB_Pre_B = MB_Pre_B * KB_Pre_B;
			if (Files[i].file_size() > GB_Pre_B)
			{
				ImGui::Text("%.2f GB", Files[i].file_size() / GB_Pre_B);
			}
			else if (Files[i].file_size() > MB_Pre_B)
			{
				ImGui::Text("%.2f MB", Files[i].file_size() / MB_Pre_B);
			}
			else if (Files[i].file_size() > KB_Pre_B)
			{
				ImGui::Text("%.2f KB", Files[i].file_size() / KB_Pre_B);
			}
			else
			{
				ImGui::Text("%d B", Files[i].file_size());
			}
			ImGui::NextColumn();
			ImGui::TextUnformatted(Files[i].path().extension().string().c_str());
			ImGui::NextColumn();
			auto ftime = Files[i].last_write_time();
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

		std::string SelectedFilePath = FileDialogState.CurrentPath + (FileDialogState.CurrentPath.empty() || FileDialogState.CurrentPath.back() == '/' ? "" : "/") + (FileDialogState.FileDialogCurrentFolder.size() > 0 ? FileDialogState.FileDialogCurrentFolder : FileDialogState.CurrentFile);
		char* buf = &SelectedFilePath[0];
		ImGui::PushItemWidth(724);
		ImGui::InputText("##SelectedFilePath", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

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

		ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f,
		              ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal))
		{
			ImGui::Text("Enter a name for the new folder");
			static char new_folder_name[500] = "";
			static char new_folder_error[500] = "";
			ImGui::InputText("##newfolder", new_folder_name, sizeof(new_folder_name));
			if (ImGui::Button("Create##1"))
			{
				if (strlen(new_folder_name) <= 0)
				{
					strcpy_s(new_folder_error, "Folder name can't be empty");
				}
				else
				{
					std::string new_file_path = FileDialogState.CurrentPath + (FileDialogState.CurrentPath.empty() || FileDialogState.CurrentPath.back() == '/' ? "" : "/") + new_folder_name;
					std::filesystem::create_directory(new_file_path);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel##1"))
			{
				strcpy_s(new_folder_name, "");
				strcpy_s(new_folder_error, "");
				ImGui::CloseCurrentPopup();
			}
			ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), new_folder_error);
			ImGui::EndPopup();
		}

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal))
		{
			ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "Are you sure you want to delete this folder?");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
			ImGui::TextUnformatted(FileDialogState.FileDialogCurrentFolder.c_str());
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
			if (ImGui::Button("Yes"))
			{
				std::filesystem::remove(FileDialogState.CurrentPath + (FileDialogState.CurrentPath.empty() || FileDialogState.CurrentPath.back() == '/' ? "" : "/") + FileDialogState.FileDialogCurrentFolder);
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

		static auto reset_everything = [&]()
		{
			FileDialogState.FileSelectIndex = 0;
			FileDialogState.FolderSelectIndex = 0;
			FileDialogState.CurrentFile = "";
			FileDialogState.InitialPathSet = false;
			ImGui::CloseCurrentPopup();
		};

		if (ImGui::Button("Cancel"))
		{
			reset_everything();
		}
		ImGui::SameLine();
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
					Path = UTF8_TO_TCHAR((FileDialogState.CurrentPath + (FileDialogState.CurrentPath.empty() || FileDialogState.CurrentPath.back() == '/' ? "" : "/") + FileDialogState.FileDialogCurrentFolder).c_str());
					reset_everything();
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
					Path = UTF8_TO_TCHAR((FileDialogState.CurrentPath + (FileDialogState.CurrentPath.empty() || FileDialogState.CurrentPath.back() == '/' ? "" : "/") + FileDialogState.CurrentFile).c_str());
					reset_everything();
				}
			}
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
	for(int32 i = 0; i < 26; ++i)
	{
		if(!(Mask & (1 << i)))
		{
			continue;
		}
		const char DriveChName = static_cast<char>('A' + i);
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
