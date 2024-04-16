// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"

namespace UnrealImGui
{
	struct FUTF8String;
}

namespace ImGui
{
	IMGUI_API bool InputText(const char* label, UnrealImGui::FUTF8String& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextMultiline(const char* label, UnrealImGui::FUTF8String& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextWithHint(const char* label, const char* hint, UnrealImGui::FUTF8String& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}

namespace ImGui
{
	struct FWindow : FNoncopyable
	{
		FORCEINLINE FWindow(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
			: State{ ImGui::Begin(name, p_open, flags) }
		{}
		FORCEINLINE ~FWindow() { ImGui::End(); }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FChildWindow : FNoncopyable
	{
		FORCEINLINE FChildWindow(const char* str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0)
			: State{ ImGui::BeginChild(str_id, size, child_flags, window_flags) }
		{}
		FORCEINLINE ~FChildWindow() { ImGui::EndChild(); }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FStyleColor : FNoncopyable
	{
		FORCEINLINE FStyleColor(ImGuiCol idx, ImU32 col) { ImGui::PushStyleColor(idx, col); }
		FORCEINLINE FStyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
		FORCEINLINE ~FStyleColor() { ImGui::PopStyleColor(); }
	};

	struct FStyleVar : FNoncopyable
	{
		FORCEINLINE FStyleVar(ImGuiStyleVar idx, float val) { ImGui::PushStyleVar(idx, val); }
		FORCEINLINE FStyleVar(ImGuiStyleVar idx, const ImVec2& val) { ImGui::PushStyleVar(idx, val); }
		FORCEINLINE ~FStyleVar() { ImGui::PopStyleVar(); }
	};

	struct FTabStop : FNoncopyable
	{
		FORCEINLINE FTabStop(bool tab_stop) { ImGui::PushTabStop(tab_stop); }
		FORCEINLINE ~FTabStop() { ImGui::PopTabStop(); }
	};

	struct FButtonRepeat : FNoncopyable
	{
		FORCEINLINE FButtonRepeat(bool repeat) { ImGui::PushButtonRepeat(repeat); }
		FORCEINLINE ~FButtonRepeat() { ImGui::PopButtonRepeat(); }
	};

	struct FItemWidth : FNoncopyable
	{
		FORCEINLINE FItemWidth(float item_width) { ImGui::PushItemWidth(item_width); }
		FORCEINLINE ~FItemWidth() { ImGui::PopItemWidth(); }
	};

	struct FTextWrapPos : FNoncopyable
	{
		FORCEINLINE FTextWrapPos(float wrap_local_pos_x = 0.0f) { ImGui::PushTextWrapPos(wrap_local_pos_x); }
		FORCEINLINE ~FTextWrapPos() { ImGui::PopTextWrapPos(); }
	};

	struct FIndent : FNoncopyable
	{
		FORCEINLINE FIndent(float indent_w = 0.0f)
			: indent_w{ indent_w }
		{
			ImGui::Indent(indent_w);
		}
		FORCEINLINE ~FIndent() { ImGui::Unindent(indent_w); }
	private:
		float indent_w;
	};

	struct FGroup : FNoncopyable
	{
		FORCEINLINE FGroup() { ImGui::BeginGroup(); }
		FORCEINLINE ~FGroup() { ImGui::EndGroup(); }
	};

	struct FIdScope : FNoncopyable
	{
		FORCEINLINE FIdScope(const char* str_id) { ImGui::PushID(str_id); }
		FORCEINLINE FIdScope(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
		FORCEINLINE FIdScope(const void* ptr_id) { ImGui::PushID(ptr_id); }
		FORCEINLINE FIdScope(int int_id) { ImGui::PushID(int_id); }
		FORCEINLINE ~FIdScope() { ImGui::PopID(); }
	};

	struct FCombo : FNoncopyable
	{
		FORCEINLINE FCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0)
			: State{ ImGui::BeginCombo(label, preview_value, flags) }
		{}
		FORCEINLINE ~FCombo() { if (State) { ImGui::EndCombo(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTreeNode : FNoncopyable
	{
		FORCEINLINE FTreeNode(const char* label) : State{ ImGui::TreeNode(label) } {}
		FORCEINLINE ~FTreeNode() { if (State) { ImGui::TreePop(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTreeNodeEx : FNoncopyable
	{
		FORCEINLINE FTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0) : State{ ImGui::TreeNodeEx(label, flags) } {}
		FORCEINLINE ~FTreeNodeEx() { if (State) { ImGui::TreePop(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTree : FNoncopyable
	{
		FORCEINLINE FTree(const char* str_id) { ImGui::TreePush(str_id); }
		FORCEINLINE FTree(const void* ptr_id) { ImGui::TreePush(ptr_id); }
		FORCEINLINE ~FTree() { ImGui::TreePop(); }
	};

	struct FListBox : FNoncopyable
	{
		FORCEINLINE FListBox(const char* label, const ImVec2& size = ImVec2(0, 0)) : State{ ImGui::BeginListBox(label, size) } {}
		FORCEINLINE ~FListBox() { if (State) { ImGui::EndListBox(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMenuBar : FNoncopyable
	{
		FORCEINLINE FMenuBar() : State{ ImGui::BeginMenuBar() } {}
		FORCEINLINE ~FMenuBar() { if (State) { ImGui::EndMenuBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMainMenuBar : FNoncopyable
	{
		FORCEINLINE FMainMenuBar() : State{ ImGui::BeginMainMenuBar() } {}
		FORCEINLINE ~FMainMenuBar() { if (State) { ImGui::EndMainMenuBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMenu : FNoncopyable
	{
		FORCEINLINE FMenu(const char* label, bool enabled = true) : State{ ImGui::BeginMenu(label, enabled) } {}
		FORCEINLINE ~FMenu() { if (State) { ImGui::EndMenu(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTooltip : FNoncopyable
	{
		FORCEINLINE FTooltip() : State{ ImGui::BeginTooltip() } {}
		FORCEINLINE ~FTooltip() { if (State) { ImGui::EndTooltip(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FItemTooltip : FNoncopyable
	{
		FORCEINLINE FItemTooltip() : State{ ImGui::BeginItemTooltip() } {}
		FORCEINLINE ~FItemTooltip() { if (State) { ImGui::EndTooltip(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopup : FNoncopyable
	{
		FORCEINLINE FPopup(const char* str_id, ImGuiWindowFlags flags = 0) : State{ ImGui::BeginPopup(str_id, flags) } {}
		FORCEINLINE ~FPopup() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupModal : FNoncopyable
	{
		FORCEINLINE FPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) : State{ ImGui::BeginPopupModal(name, p_open, flags) } {}
		FORCEINLINE ~FPopupModal() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextItem : FNoncopyable
	{
		FORCEINLINE FPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextItem(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextItem() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextWindow : FNoncopyable
	{
		FORCEINLINE FPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextWindow(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextWindow() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextVoid : FNoncopyable
	{
		FORCEINLINE FPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextVoid(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextVoid() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTable : FNoncopyable
	{
		FORCEINLINE FTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f) : State{ ImGui::BeginTable(str_id, column, flags, outer_size, inner_width) } {}
		FORCEINLINE ~FTable() { if (State) { ImGui::EndTable(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTabBar : FNoncopyable
	{
		FORCEINLINE FTabBar(const char* str_id, ImGuiTabBarFlags flags = 0) : State{ ImGui::BeginTabBar(str_id, flags) } {}
		FORCEINLINE ~FTabBar() { if (State) { ImGui::EndTabBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTabItem : FNoncopyable
	{
		FORCEINLINE FTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0) : State{ ImGui::BeginTabItem(label, p_open, flags) } {}
		FORCEINLINE ~FTabItem() { if (State) { ImGui::EndTabItem(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FDisabled : FNoncopyable
	{
		FORCEINLINE FDisabled(bool disabled = true) { ImGui::BeginDisabled(disabled); }
		FORCEINLINE ~FDisabled() { ImGui::EndDisabled(); }
	};

	struct FClipRect : FNoncopyable
	{
		FORCEINLINE FClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect = false) { ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect); }
		FORCEINLINE ~FClipRect() { ImGui::PopClipRect(); }
	};
}
