// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "Containers/Utf8String.h"

namespace ImGui
{
	IMGUI_API bool InputText(const char* label, FUtf8String& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextMultiline(const char* label, FUtf8String& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextWithHint(const char* label, const char* hint, FUtf8String& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	FORCEINLINE bool InputText(const char* label, FString& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		FUtf8String Utf8String{ str };
		const bool Ret = ImGui::InputText(label, Utf8String, flags, callback, user_data);
		if (ImGui::IsItemEdited())
		{
			str = FString{ Utf8String };
		}
		return Ret;
	}
	FORCEINLINE bool InputTextMultiline(const char* label, FString& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		FUtf8String Utf8String{ str };
		const bool Ret = ImGui::InputTextMultiline(label, Utf8String, size, flags, callback, user_data);
		if (ImGui::IsItemEdited())
		{
			str = FString{ Utf8String };
		}
		return Ret;
	}
	FORCEINLINE bool InputTextWithHint(const char* label, const char* hint, FString& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		FUtf8String Utf8String{ str };
		const bool Ret = ImGui::InputTextWithHint(label, hint, Utf8String, flags, callback, user_data);
		if (ImGui::IsItemEdited())
		{
			str = FString{ Utf8String };
		}
		return Ret;
	}
}

namespace ImGui
{
	struct FWindow : FNoncopyable
	{
		[[nodiscard]]
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
		[[nodiscard]]
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
		[[nodiscard]]
		FORCEINLINE FStyleColor(ImGuiCol idx, ImU32 col) { ImGui::PushStyleColor(idx, col); }
		[[nodiscard]]
		FORCEINLINE FStyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
		FORCEINLINE ~FStyleColor() { ImGui::PopStyleColor(); }
	};

	struct FStyleVar : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FStyleVar(ImGuiStyleVar idx, float val) { ImGui::PushStyleVar(idx, val); }
		[[nodiscard]]
		FORCEINLINE FStyleVar(ImGuiStyleVar idx, const ImVec2& val) { ImGui::PushStyleVar(idx, val); }
		FORCEINLINE ~FStyleVar() { ImGui::PopStyleVar(); }
	};

	struct FItemFlagScope : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FItemFlagScope(ImGuiItemFlags option, bool enabled) { ImGui::PushItemFlag(option, enabled); }
		FORCEINLINE ~FItemFlagScope() { ImGui::PopItemFlag(); }
	};

	struct FItemWidth : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FItemWidth(float item_width) { ImGui::PushItemWidth(item_width); }
		FORCEINLINE ~FItemWidth() { ImGui::PopItemWidth(); }
	};

	struct FTextWrapPos : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTextWrapPos(float wrap_local_pos_x = 0.0f) { ImGui::PushTextWrapPos(wrap_local_pos_x); }
		FORCEINLINE ~FTextWrapPos() { ImGui::PopTextWrapPos(); }
	};

	struct FIndent : FNoncopyable
	{
		[[nodiscard]]
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
		[[nodiscard]]
		FORCEINLINE FGroup() { ImGui::BeginGroup(); }
		FORCEINLINE ~FGroup() { ImGui::EndGroup(); }
	};

	struct FIdScope : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FIdScope(const char* str_id) { ImGui::PushID(str_id); }
		[[nodiscard]]
		FORCEINLINE FIdScope(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
		[[nodiscard]]
		FORCEINLINE FIdScope(const void* ptr_id) { ImGui::PushID(ptr_id); }
		[[nodiscard]]
		FORCEINLINE FIdScope(int32 int_id) { ImGui::PushID(int_id); }
		[[nodiscard]]
		FORCEINLINE FIdScope(uint32 int_id) { ImGui::PushID(int_id); }
		FORCEINLINE ~FIdScope() { ImGui::PopID(); }
	};

	struct FCombo : FNoncopyable
	{
		[[nodiscard]]
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
		[[nodiscard]]
		FORCEINLINE FTreeNode(const char* label) : State{ ImGui::TreeNode(label) } {}
		FORCEINLINE ~FTreeNode() { if (State) { ImGui::TreePop(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTreeNodeEx : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0) : State{ ImGui::TreeNodeEx(label, flags) } {}
		FORCEINLINE ~FTreeNodeEx() { if (State) { ImGui::TreePop(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTree : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTree(const char* str_id) { ImGui::TreePush(str_id); }
		[[nodiscard]]
		FORCEINLINE FTree(const void* ptr_id) { ImGui::TreePush(ptr_id); }
		FORCEINLINE ~FTree() { ImGui::TreePop(); }
	};

	struct FListBox : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FListBox(const char* label, const ImVec2& size = ImVec2(0, 0)) : State{ ImGui::BeginListBox(label, size) } {}
		FORCEINLINE ~FListBox() { if (State) { ImGui::EndListBox(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMenuBar : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FMenuBar() : State{ ImGui::BeginMenuBar() } {}
		FORCEINLINE ~FMenuBar() { if (State) { ImGui::EndMenuBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMainMenuBar : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FMainMenuBar() : State{ ImGui::BeginMainMenuBar() } {}
		FORCEINLINE ~FMainMenuBar() { if (State) { ImGui::EndMainMenuBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FMenu : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FMenu(const char* label, bool enabled = true) : State{ ImGui::BeginMenu(label, enabled) } {}
		FORCEINLINE ~FMenu() { if (State) { ImGui::EndMenu(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTooltip : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTooltip() : State{ ImGui::BeginTooltip() } {}
		FORCEINLINE ~FTooltip() { if (State) { ImGui::EndTooltip(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FItemTooltip : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FItemTooltip() : State{ ImGui::BeginItemTooltip() } {}
		FORCEINLINE ~FItemTooltip() { if (State) { ImGui::EndTooltip(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopup : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FPopup(const char* str_id, ImGuiWindowFlags flags = 0) : State{ ImGui::BeginPopup(str_id, flags) } {}
		FORCEINLINE ~FPopup() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupModal : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) : State{ ImGui::BeginPopupModal(name, p_open, flags) } {}
		FORCEINLINE ~FPopupModal() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextItem : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextItem(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextItem() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextWindow : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextWindow(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextWindow() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FPopupContextVoid : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1) : State{ ImGui::BeginPopupContextVoid(str_id, popup_flags) } {}
		FORCEINLINE ~FPopupContextVoid() { if (State) { ImGui::EndPopup(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTable : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f) : State{ ImGui::BeginTable(str_id, column, flags, outer_size, inner_width) } {}
		FORCEINLINE ~FTable() { if (State) { ImGui::EndTable(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTabBar : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTabBar(const char* str_id, ImGuiTabBarFlags flags = 0) : State{ ImGui::BeginTabBar(str_id, flags) } {}
		FORCEINLINE ~FTabBar() { if (State) { ImGui::EndTabBar(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FTabItem : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0) : State{ ImGui::BeginTabItem(label, p_open, flags) } {}
		FORCEINLINE ~FTabItem() { if (State) { ImGui::EndTabItem(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};

	struct FDisabled : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FDisabled(bool disabled = true) { ImGui::BeginDisabled(disabled); }
		FORCEINLINE ~FDisabled() { ImGui::EndDisabled(); }
	};

	struct FClipRect : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect = false) { ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect); }
		FORCEINLINE ~FClipRect() { ImGui::PopClipRect(); }
	};
}
