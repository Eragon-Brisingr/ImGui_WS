
#pragma once

struct ImFontAtlas;
typedef int ImGuiToastType;

enum ImGuiToastType_
{
	ImGuiToastType_None,
	ImGuiToastType_Success,
	ImGuiToastType_Warning,
	ImGuiToastType_Error,
	ImGuiToastType_Info,
	ImGuiToastType_COUNT
};

namespace ImGui
{
	IMGUI_API void RenderNotifications();

	IMGUI_API void InsertNotification(ImGuiToastType type, const char* format, ...);
	IMGUI_API void InsertNotification(ImGuiToastType type, float dismiss_seconds, const char* format, ...);
}