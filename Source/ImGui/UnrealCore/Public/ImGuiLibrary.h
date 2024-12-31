// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "ImGuiLibraryBase.h"
#include "InputCoreTypes.h"
#include "UnrealImGuiKeyUtils.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiTexture.h"
#include "UnrealImGuiTypes.h"
#include "ImGuiLibrary.generated.h"

static_assert(std::is_same_v<FVector2D::FReal, double>);

UCLASS()
class IMGUI_API UImGuiLibrary : public UImGuiLibraryBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", meta = (ImGuiScopeExit = End, ImGuiAlwaysExit, DisplayName = "Window", Name = "Untitle", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool Begin(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", meta = (ImGuiScopeExit = End, ImGuiAlwaysExit, Name = "Untitle", DisplayName = "Window (Open State)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginWithOpenState(FName Name, UPARAM(Ref)bool& OpenState, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), &OpenState, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", meta = (ImGuiScopeExit = End, ImGuiAlwaysExit, Name = "Untitle", DisplayName = "Window (Full Viewport)", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginFullViewport(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		static constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
		ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags | SinglePanelFlags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", BlueprintInternalUseOnly)
	static void End()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::End();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Child Windows", meta = (ImGuiScopeExit = EndChild, ImGuiAlwaysExit, DisplayName = "Child Window", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginChild(FName Name, FVector2D Size = FVector2D::ZeroVector, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiChildFlags"))int32 ChildFlags = 0, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 WindowFlags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginChild(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Size }, ChildFlags, WindowFlags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Child Windows", BlueprintInternalUseOnly)
	static void EndChild()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndChild();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsWindowAppearing()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsWindowAppearing();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsWindowCollapsed()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsWindowCollapsed();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsWindowFocused(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiFocusedFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsWindowFocused(Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsWindowHovered(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiHoveredFlags")) int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsWindowHovered(Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowDpiScale()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::GetWindowDpiScale();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowPos()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetWindowPos() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowSize()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetWindowSize() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowWidth()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetWindowWidth();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowHeight()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetWindowHeight();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetNextWindowPos(const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0, const FVector2D& Pivot = FVector2D::ZeroVector)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowPos(ImVec2{ Pos }, Cond, ImVec2{ Pivot });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowSize(const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowSize(ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowContentSize(const FVector2D& Size)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowSize(ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowCollapsed(bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowCollapsed(Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowFocus()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowScroll(const FVector2D& Scroll)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowScroll(ImVec2{ Scroll });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowBgAlpha(float Alpha)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowBgAlpha(Alpha);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowViewport(int32 ViewportId)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextWindowViewport(ViewportId);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowPos(const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowPos(ImVec2{ Pos }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowSize(const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowSize(ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowCollapsed(bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowCollapsed(Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFocus()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFontScale(float Scale)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowFontScale(Scale);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void SetWindowPosByName(FName Name, const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowPos(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Pos }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowSizeByName(FName Name, const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowSize(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowCollapsedByName(FName Name, bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowCollapsed(TCHAR_TO_UTF8(*Name.ToString()), Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFocusByName(FName Name)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetWindowFocus(TCHAR_TO_UTF8(*Name.ToString()));
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetContentRegionAvail()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetContentRegionAvail() };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollX()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetScrollX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollY()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetScrollY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollX(float ScrollX)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollX(ScrollX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollY(float ScrollY)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollX(ScrollY);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollMaxX()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetScrollMaxX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollMaxY()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetScrollMaxY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollHereX(float CenterRatioX = 0.5f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollHereX(CenterRatioX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollHereY(float CenterRatioY = 0.5f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollHereY(CenterRatioY);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollFromPosX(float LocalX, float CenterRatioX = 0.5f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollFromPosX(LocalX, CenterRatioX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollFromPosY(float LocalY, float CenterRatioY = 0.5f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetScrollFromPosY(LocalY, CenterRatioY);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleColor, DisplayName = "Style Color"), BlueprintInternalUseOnly)
	static void PushStyleColor(EImGuiCol ColorType, const FColor& Color)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushStyleColor((ImGuiCol)ColorType, IM_COL32(Color.R, Color.G, Color.B, Color.A));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopStyleColor(int32 Count = 1)
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopStyleColor(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleVar, DisplayName = "Style Var (float)"), BlueprintInternalUseOnly)
	static void PushStyleVarFloat(EImGuiStyleVar StyleVar, float Value)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushStyleVar((ImGuiStyleVar)StyleVar, Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleVar, DisplayName = "Style Var (Vector2)"), BlueprintInternalUseOnly)
	static void PushStyleVarVector2(EImGuiStyleVar StyleVar, const FVector2D& Value)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushStyleVar((ImGuiStyleVar)StyleVar, ImVec2{ Value });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopStyleVar(int32 Count = 1)
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopStyleVar(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopTabStop, DisplayName = "Tab Stop"), BlueprintInternalUseOnly)
	static void PushItemFlag(EImGuiItemFlags Option, bool TabStop)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushItemFlag((ImGuiItemFlags)Option, TabStop);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopItemFlag()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopItemFlag();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopItemWidth, DisplayName = "Item Width"), BlueprintInternalUseOnly)
	static void PushItemWidth(float ItemWidth)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushItemWidth(ItemWidth);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopItemWidth()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopItemWidth();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemWidth(float ItemWidth)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextItemWidth(ItemWidth);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Parameters stacks", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float CalcItemWidth()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::CalcItemWidth();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopTextWrapPos, DisplayName = "Text Wrap Pos"), BlueprintInternalUseOnly)
	static void PushTextWrapPos(float WrapLocalPosX = 0.0f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushTextWrapPos(WrapLocalPosX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopTextWrapPos()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopTextWrapPos();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFontSize()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetFontSize();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetFontTexUvWhitePixel()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetFontTexUvWhitePixel() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FColor GetStyleColor(EImGuiCol ColorType, float AlphaMul = 1.0f)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		const ImU32 Color{ ImGui::GetColorU32((ImGuiCol)ColorType, AlphaMul) };
		return FColor{ uint8(Color >> IM_COL32_R_SHIFT), uint8(Color >> IM_COL32_G_SHIFT), uint8(Color >> IM_COL32_B_SHIFT), uint8(Color >> IM_COL32_A_SHIFT) };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorScreenPos()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetCursorScreenPos() };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorScreenPos(const FVector2D& Pos)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetCursorScreenPos(ImVec2{ Pos });
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorPos()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetCursorPos() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetCursorPosX()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetCursorPosX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetCursorPosY()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetCursorPosY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPos(const FVector2D& LocalPos)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetCursorPos(ImVec2{ LocalPos });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPosX(float LocalX)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetCursorPosX(LocalX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPosY(float LocalY)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetCursorPosY(LocalY);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorStartPos()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetCursorStartPos() };
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Separator()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Separator();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void SameLine(float OffsetFromStartX = 0.f, float Spacing = -1.f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SameLine(OffsetFromStartX, Spacing);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void NewLine()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::NewLine();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Spacing()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Spacing();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Dummy(const FVector2D& Size)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Dummy(ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiScopeExit = Unindent, AdvancedDisplay = IndentW), BlueprintInternalUseOnly)
	static void Indent(float IndentW = 0.0f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Indent(IndentW);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", BlueprintInternalUseOnly)
	static void Unindent(float IndentW = 0.0f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Unindent(IndentW);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiScopeExit = PopTextWrapPos, DisplayName = "Group"), BlueprintInternalUseOnly)
	static void BeginGroup()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::BeginGroup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", BlueprintInternalUseOnly)
	static void EndGroup()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndGroup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void AlignTextToFramePadding()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::AlignTextToFramePadding();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTextLineHeight()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetTextLineHeight();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTextLineHeightWithSpacing()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetTextLineHeightWithSpacing();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFrameHeight()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetFrameHeight();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFrameHeightWithSpacing()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetFrameHeightWithSpacing();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiScopeExit = PopID, DisplayName = "Set Id"), BlueprintInternalUseOnly)
	static void PushID(FName Name)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImGui::PushID(TCHAR_TO_UTF8(*Name.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiScopeExit = PopID, DisplayName = "Set Id (Integer)"), BlueprintInternalUseOnly)
	static void PushIDByInteger(int32 Integer)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImGui::PushID(Integer);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", BlueprintInternalUseOnly)
	static void PopID()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopID();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiId GetID(FName Name)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetID(TCHAR_TO_UTF8(*Name.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Text(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextColored(FLinearColor Color, const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TextColored(ImVec4{ Color }, "%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextDisabled(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TextDisabled("%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextWrapped(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TextWrapped("%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void LabelText(FName Label, const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::LabelText(TCHAR_TO_UTF8(*Label.ToString()), "%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void BulletText(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::BulletText("%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SeparatorText(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SeparatorText(TCHAR_TO_UTF8(*Text));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool Button(FName Label, FVector2D Size = FVector2D::ZeroVector)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::Button(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool SmallButton(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SmallButton(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool ArrowButton(FName Label, EImGuiDir Dir)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::ArrowButton(TCHAR_TO_UTF8(*Label.ToString()), (ImGuiDir)Dir);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool InvisibleButton(FName Label, FVector2D Size)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InvisibleButton(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool CheckBox(FName Label, UPARAM(Ref)bool& Value)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::Checkbox(TCHAR_TO_UTF8(*Label.ToString()), &Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool CheckboxFlags(FName Label, UPARAM(Ref)int32& Flags, int32 FlagsValue)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::CheckboxFlags(TCHAR_TO_UTF8(*Label.ToString()), &Flags, FlagsValue);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool RadioButton(FName Label, bool bActive)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::RadioButton(TCHAR_TO_UTF8(*Label.ToString()), bActive);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiFunction, SizeArg = "(X=-1, Y=0)", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void ProgressBar(float Fraction, FVector2D SizeArg, const FString& Overlay)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::ProgressBar(Fraction, ImVec2{ SizeArg }, TCHAR_TO_UTF8(*Overlay));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Bullet()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Bullet();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiTextureHandle CreateTextureHandle() { return FImGuiTextureHandle::MakeUnique(); }
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiTextureHandle FindOrCreateTextureHandle(const UTexture* Texture, bool& bCreated) { return FImGuiTextureHandle::FindOrCreateHandle(Texture, bCreated); }
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsTextureHandleValid(const FImGuiTextureHandle& Handle) { return Handle.IsValid(); }
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, TextureFormat = Gray8), BlueprintInternalUseOnly)
	static void UpdateTexture2D(const FImGuiTextureHandle& Handle, EImGuiTextureFormat TextureFormat, UTexture2D* Texture2D)
	{
		UnrealImGui::UpdateTextureData(Handle, (UnrealImGui::ETextureFormat)TextureFormat, Texture2D);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, TextureFormat = Gray8), BlueprintInternalUseOnly)
	static void UpdateTextureRenderTarget2D(const FImGuiTextureHandle& Handle, EImGuiTextureFormat TextureFormat, UTextureRenderTarget2D* RenderTarget2D)
	{
		UnrealImGui::UpdateTextureData(Handle, (UnrealImGui::ETextureFormat)TextureFormat, RenderTarget2D);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, DefaultToSelf = Outer, AdvancedDisplay = 4, TextureFormat = Gray8, Width = 128, Height = 128), BlueprintInternalUseOnly)
	static UTextureRenderTarget2D* CreatePersistentTexture(UPARAM(Ref)FImGuiTextureHandle& Handle, EImGuiTextureFormat TextureFormat, int32 Width, int32 Height, UObject* Outer, FName Name)
	{
		return UnrealImGui::CreateTexture(Handle, (UnrealImGui::ETextureFormat)TextureFormat, Width, Height, Outer, Name);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, TextureFormat = Gray8, Width = 128, Height = 128), BlueprintInternalUseOnly)
	static void UpdateTextureDataRaw(const FImGuiTextureHandle& Handle, EImGuiTextureFormat TextureFormat, int32 Width, int32 Height, const TArray<uint8>& Data, UTextureRenderTarget2D* PersistentTexture)
	{
		UnrealImGui::UpdateTextureData(Handle, (UnrealImGui::ETextureFormat)TextureFormat, Width, Height, Data.GetData(), PersistentTexture);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void Texture(const FImGuiTextureHandle& Handle, FVector2D ImageSize, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Image(Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Texture", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool TextureButton(FName Label, const FImGuiTextureHandle& Handle, FVector2D ImageSize, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::ImageButton(TCHAR_TO_UTF8(*Label.ToString()), Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Images", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void Image(UTexture* Texture, FVector2D ImageSize, EImGuiTextureFormat Format = EImGuiTextureFormat::RGB8, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		const FImGuiTextureHandle Handle = UnrealImGui::FindOrAddTexture((UnrealImGui::ETextureFormat)Format, Texture);
		ImGui::Image(Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Images", meta = (ImGuiTrigger, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	static bool ImageButton(FName Label, UTexture* Texture, FVector2D ImageSize, EImGuiTextureFormat Format = EImGuiTextureFormat::RGB8, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		const FImGuiTextureHandle Handle = UnrealImGui::FindOrAddTexture((UnrealImGui::ETextureFormat)Format, Texture);
		return ImGui::ImageButton(TCHAR_TO_UTF8(*Label.ToString()), Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", meta = (ImGuiScopeExit = EndCombo, DisplayName = "Combo (Scope)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginCombo(FName Label, FName PreviewValue, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiComboFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginCombo(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*PreviewValue.ToString()), (ImGuiComboFlags)Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", BlueprintInternalUseOnly)
	static void EndCombo()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndCombo();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", meta = (ImGuiTrigger, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	static bool Combo(FName Label, UPARAM(Ref)int32& CurrentItem, const TArray<FName>& Items, int32 PopupMaxHeightInItems = -1)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		struct FUserData
		{
			const TArray<FName>& Items;
		};
		FUserData UserData{ Items };
		return ImGui::Combo(TCHAR_TO_UTF8(*Label.ToString()), &CurrentItem, [](void* UserData, int32 Idx)
		{
			const TArray<FName>& Items = static_cast<FUserData*>(UserData)->Items;
			static UnrealImGui::FUTF8String String;
			String = Items[Idx].ToString();
			return *String;
		}, &UserData, Items.Num(), PopupMaxHeightInItems);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat(FName Label, UPARAM(Ref)float& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat2(FName Label, UPARAM(Ref)FVector2f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat3(FName Label, UPARAM(Ref)FVector3f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat4(FName Label, UPARAM(Ref)FVector4f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloatRange2(FName Label, UPARAM(Ref)float& CurrentMin, UPARAM(Ref)float& CurrentMax, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, const FString& FormatMin = TEXT("%.3f"), const FString& FormatMax = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragFloatRange2(TCHAR_TO_UTF8(*Label.ToString()), &CurrentMin, &CurrentMax, Speed, Min, Max, TCHAR_TO_UTF8(*FormatMin), TCHAR_TO_UTF8(*FormatMax), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt(FName Label, UPARAM(Ref)int32& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt2(FName Label, UPARAM(Ref)FIntPoint& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt3(FName Label, UPARAM(Ref)FIntVector& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt4(FName Label, UPARAM(Ref)FIntVector4& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragIntRange2(FName Label, UPARAM(Ref)int32& CurrentMin, UPARAM(Ref)int32& CurrentMax, float Speed = 1.0f, int Min = 0, int Max = 0, const FString& FormatMin = TEXT("%d"), const FString& FormatMax = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragIntRange2(TCHAR_TO_UTF8(*Label.ToString()), &CurrentMin, &CurrentMax, Speed, Min, Max, TCHAR_TO_UTF8(*FormatMin), TCHAR_TO_UTF8(*FormatMax), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragDouble(FName Label, UPARAM(Ref)double& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector2(FName Label, UPARAM(Ref)FVector2D& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 2, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector3(FName Label, UPARAM(Ref)FVector& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 3, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector4(FName Label, UPARAM(Ref)FVector4& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 4, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragByte(FName Label, UPARAM(Ref)uint8& Value, float Speed = 1.0f, uint8 Min = 0, uint8 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt64(FName Label, UPARAM(Ref)int64& Value, float Speed = 1.0f, int64 Min = 0, int64 Max = 0, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat(FName Label, UPARAM(Ref)float& Value, float Min, float Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat2(FName Label, UPARAM(Ref)FVector2f& Value, float Min, float Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat3(FName Label, UPARAM(Ref)FVector3f& Value, float Min, float Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat4(FName Label, UPARAM(Ref)FVector4f& Value, float Min, float Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderAngle(FName Label, UPARAM(Ref)float& Rad, float DegreesMin = -360.0f, float DegreesMax = +360.0f, const FString& Format = TEXT("%.0f deg"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderAngle(TCHAR_TO_UTF8(*Label.ToString()), &Rad, DegreesMin, DegreesMax, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt(FName Label, UPARAM(Ref)int32& Value, int32 Min, int32 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt2(FName Label, UPARAM(Ref)FIntPoint& Value, int32 Min, int32 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt3(FName Label, UPARAM(Ref)FIntVector& Value, int32 Min, int32 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt4(FName Label, UPARAM(Ref)FIntVector4& Value, int32 Min, int32 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderDouble(FName Label, UPARAM(Ref)double& Value, double Min, double Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector2(FName Label, UPARAM(Ref)FVector2D& Value, double Min, double Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 2, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector3(FName Label, UPARAM(Ref)FVector& Value, double Min, double Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 3, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector4(FName Label, UPARAM(Ref)FVector4& Value, double Min, double Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 4, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderByte(FName Label, UPARAM(Ref)uint8& Value, uint8 Min, uint8 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt64(FName Label, UPARAM(Ref)int64& Value, int64 Min, int64 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool VSliderFloat(FName Label, FVector2D Size, UPARAM(Ref)float& Value, float Min, float Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::VSliderFloat(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderInt(FName Label, FVector2D Size, UPARAM(Ref)int32& Value, int32 Min, int32 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::VSliderInt(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderDouble(FName Label, FVector2D Size, UPARAM(Ref)double& Value, double Min, double Max, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_Double, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderByte(FName Label, FVector2D Size, UPARAM(Ref)uint8& Value, uint8 Min, uint8 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_U8, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderInt64(FName Label, FVector2D Size, UPARAM(Ref)int64& Value, int64 Min, int64 Max, const FString& Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_S64, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputText(FName Label, UPARAM(Ref)FString& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = ImGui::InputText(TCHAR_TO_UTF8(*Label.ToString()), UTF8String, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputTextMultiline(FName Label, UPARAM(Ref)FString& Value, FVector2f Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = ImGui::InputTextMultiline(TCHAR_TO_UTF8(*Label.ToString()), UTF8String, ImVec2{ Size }, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputTextWithHint(FName Label, const FString& Hint, UPARAM(Ref)FString& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = ImGui::InputTextWithHint(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Hint), UTF8String, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputFloat(FName Label, UPARAM(Ref)float& Value, float Step = 0.f, float StepFast = 0.f, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector2f(FName Label, UPARAM(Ref)FVector2f& Value, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector3f(FName Label, UPARAM(Ref)FVector3f& Value, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputFloat4(FName Label, UPARAM(Ref)FVector4f& Value, const FString& Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt(FName Label, UPARAM(Ref)int32& Value, int32 Step = 1, int32 StepFast = 100, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt2(FName Label, UPARAM(Ref)FIntPoint& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt3(FName Label, UPARAM(Ref)FIntVector& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt4(FName Label, UPARAM(Ref)FIntVector4& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputDouble(FName Label, UPARAM(Ref)double& Value, double Step = 0.f, double StepFast = 0.f, const FString& Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputDouble(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector2(FName Label, UPARAM(Ref)FVector2D& Value, double Step = 0.f, double StepFast = 0.f, const FString& Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 2, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector3(FName Label, UPARAM(Ref)FVector& Value, double Step = 0.f, double StepFast = 0.f, const FString& Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 3, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector4(FName Label, UPARAM(Ref)FVector4& Value, double Step = 0.f, double StepFast = 0.f, const FString& Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 4, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputByte(FName Label, UPARAM(Ref)uint8& Value, uint8 Step = 1, uint8 StepFast = 10, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt64(FName Label, UPARAM(Ref)int64& Value, int64 Step = 1, int64 StepFast = 100, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, nullptr, Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool LinearColorEdit(FName Label, UPARAM(Ref)FLinearColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &Value.R, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorEdit(FName Label, UPARAM(Ref)FColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		FLinearColor V = Value;
		const bool Ret = ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &V.R, Flags);
		if (Ret)
		{
			Value = V.ToFColor(false);
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool LinearColorPicker(FName Label, UPARAM(Ref)FLinearColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::ColorPicker4(TCHAR_TO_UTF8(*Label.ToString()), &Value.R, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorPicker(FName Label, UPARAM(Ref)FColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		FLinearColor V = Value;
		const bool Ret = ImGui::ColorPicker4(TCHAR_TO_UTF8(*Label.ToString()), &V.R, Flags);
		if (Ret)
		{
			Value = V.ToFColor(false);
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorButton(FName Label, FLinearColor Color, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0, FVector2D Size = FVector2D::ZeroVector)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::ColorButton(TCHAR_TO_UTF8(*Label.ToString()), ImVec4{ Color }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColorEditOptions(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetColorEditOptions(Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiScopeExit = TreePop, DisplayName = "Tree Node"), BlueprintInternalUseOnly)
	static bool TreeNode(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::TreeNode(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiScopeExit = TreePop, DisplayName = "Tree Node Ex"), BlueprintInternalUseOnly)
	static bool TreeNodeEx(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiScopeExit = TreePop, DisplayName = "Tree Id Scope"), BlueprintInternalUseOnly)
	static void TreePush(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TreePush(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", BlueprintInternalUseOnly)
	static void TreePop()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::TreePop();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTreeNodeToLabelSpacing()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetTreeNodeToLabelSpacing();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool CollapsingHeaderRegular(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::CollapsingHeader(TCHAR_TO_UTF8(*Label.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool CollapsingHeader(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::CollapsingHeader(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool CollapsingHeaderWithState(FName Label, UPARAM(Ref)bool& Visible, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::CollapsingHeader(TCHAR_TO_UTF8(*Label.ToString()), &Visible, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemOpen(bool IsOpen, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextItemOpen(IsOpen, (int32)Cond);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Selectables", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool Selectable(FName Label, bool bActive, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSelectableFlags"))int32 Flags = 0, FVector2D Size = FVector2D::ZeroVector)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::Selectable(TCHAR_TO_UTF8(*Label.ToString()), bActive, Flags, ImVec2{ Size });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", meta = (ImGuiScopeExit = EndListBox, DisplayName = "List Box Scope"), BlueprintInternalUseOnly)
	static bool BeginListBox(FName Label, const FVector2D& Size = FVector2D::ZeroVector)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginListBox(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", BlueprintInternalUseOnly)
	static void EndListBox()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndListBox();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", meta = (ImGuiFunction, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	static bool ListBox(FName Label, UPARAM(Ref)int32& CurrentItem, const TArray<FName>& Items, int HeightInItems = -1)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		struct FUserData
		{
			const TArray<FName>& Items;
		};
		FUserData UserData{ Items };
		return ImGui::ListBox(TCHAR_TO_UTF8(*Label.ToString()), &CurrentItem, [](void* UserData, int32 Idx)->const char*
		{
			const TArray<FName>& Items = static_cast<FUserData*>(UserData)->Items;
			static UnrealImGui::FUTF8String String;
			String = Items[Idx].ToString();
			return *String;
		}, &UserData, Items.Num(), HeightInItems);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueBool(const FString& Prefix, bool Value)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Value(TCHAR_TO_UTF8(*Prefix), Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueInt(const FString& Prefix, int32 Value)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Value(TCHAR_TO_UTF8(*Prefix), Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueFloat(const FString& Prefix, float Value, const FString& Format = TEXT(""))
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Value(TCHAR_TO_UTF8(*Prefix), Value, Format.Len() > 0 ? TCHAR_TO_UTF8(*Format) : nullptr);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta=(ImGuiScopeExit = EndMenuBar, DisplayName = "Menu Bar"), BlueprintInternalUseOnly)
	static bool BeginMenuBar()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", BlueprintInternalUseOnly)
	static void EndMenuBar()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta = (ImGuiScopeExit = EndMainMenuBar, DisplayName = "Main Menu Bar"), BlueprintInternalUseOnly)
	static bool BeginMainMenuBar()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginMainMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", BlueprintInternalUseOnly)
	static void EndMainMenuBar()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndMainMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta=(ImGuiScopeExit = EndMenu, DisplayName = "Menu", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginMenu(FName Label, bool Enabled = true)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginMenu(TCHAR_TO_UTF8(*Label.ToString()), Enabled);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", BlueprintInternalUseOnly)
	static void EndMenu()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		return ImGui::EndMenu();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool MenuItem(FName Label, const FString& Shortcut, bool Selected, bool Enabled = true)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::MenuItem(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Shortcut), Selected, Enabled);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", meta=(ImGuiScopeExit = EndTooltip, DisplayName = "Tooltip"), BlueprintInternalUseOnly)
	static bool BeginTooltip()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginTooltip();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", BlueprintInternalUseOnly)
	static void EndTooltip()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		return ImGui::EndTooltip();
	}
	UFUNCTION(Blueprintable, Category="ImGui|Tooltip", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetTooltip(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetTooltip("%s", TCHAR_TO_UTF8(*Text));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", meta=(ImGuiScopeExit = EndTooltip, DisplayName = "Tooltip"), BlueprintInternalUseOnly)
	static bool BeginItemTooltip()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginItemTooltip();
	}
	UFUNCTION(Blueprintable, Category="ImGui|Tooltip", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetItemTooltip(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetItemTooltip("%s", TCHAR_TO_UTF8(*Text));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginPopup(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopup(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup Modal", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginPopupModal(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopupModal(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup Modal (Open State)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginPopupModalOpenState(FName Name, bool& Open, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopupModal(TCHAR_TO_UTF8(*Name.ToString()), &Open, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", BlueprintInternalUseOnly)
	static void EndPopup()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndPopup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void OpenPopup(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::OpenPopup(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void OpenPopupOnItemClick(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::OpenPopupOnItemClick(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void CloseCurrentPopup()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::CloseCurrentPopup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextItem"), BlueprintInternalUseOnly)
	static bool BeginPopupContextItem(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopupContextItem(Name == NAME_None ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextWindow"), BlueprintInternalUseOnly)
	static bool BeginPopupContextWindow(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopupContextWindow(Name == NAME_None ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextVoid"), BlueprintInternalUseOnly)
	static bool BeginPopupContextVoid(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginPopupContextVoid(Name == NAME_None ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsPopupOpen(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsPopupOpen(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiScopeExit = EndTable, AdvancedDisplay = 2, DisplayName = "Table"), BlueprintInternalUseOnly)
	static bool BeginTable(FName Name, int32 Column, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableFlags"))int32 Flags = 0, FVector2D OuterSize = FVector2D::ZeroVector, float InnerWidth = 0.0f)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginTable(TCHAR_TO_UTF8(*Name.ToString()), Column, Flags, ImVec2{ OuterSize }, InnerWidth);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", BlueprintInternalUseOnly)
	static void EndTable()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		return ImGui::EndTable();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void TableNextRow(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableRowFlags"))int32 Flags = 0, float MinRowHeight = 0.0f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableNextRow(Flags, MinRowHeight);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TableNextColumn()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::TableNextColumn();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TableSetColumnIndex(int32 ColumnN)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::TableSetColumnIndex(ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 1, Flag = 0, InitWidthOrWeight = 0, UserId = "(Id=0)"), BlueprintInternalUseOnly)
	static void TableSetupColumn(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableColumnFlags"))int32 Flags, float InitWidthOrWeight, FImGuiId UserId)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableSetupColumn(TCHAR_TO_UTF8(*Label.ToString()), Flags, InitWidthOrWeight, UserId);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableSetupScrollFreeze(int32 Columns, int32 Rows)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableSetupScrollFreeze(Columns, Rows);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableHeader(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableHeader(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableHeadersRow()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableHeadersRow();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableAngledHeadersRow()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableAngledHeadersRow();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetColumnCount()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::TableGetColumnCount();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetColumnIndex()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::TableGetColumnIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetRowIndex()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::TableGetRowIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static FName TableGetColumnName(int32 ColumnN = -1)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return UTF8_TO_TCHAR(ImGui::TableGetColumnName(ColumnN));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static int32 TableGetColumnFlags(int32 ColumnN = -1)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::TableGetColumnFlags(ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableSetColumnEnabled(int32 ColumnN, bool Value)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableSetColumnEnabled(ColumnN, Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void TableSetBgColor(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableBgTarget"))int32 Target, FColor Color, int32 ColumnN = -1)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::TableSetBgColor(Target, IM_COL32(Color.R, Color.G, Color.B, Color.A), ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void Columns(int32 Count = 1, FName Id = NAME_None, bool Border = true)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::Columns(Count, Id == NAME_None ? nullptr : TCHAR_TO_UTF8(*Id.ToString()), Border);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void NextColumn()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::NextColumn();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetColumnIndex()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetColumnIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static float GetColumnWidth(int32 ColumnIndex = -1)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetColumnWidth(ColumnIndex);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColumnWidth(int32 ColumnIndex, float Width)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetColumnWidth(ColumnIndex, Width);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static float GetColumnOffset(int32 ColumnIndex = -1)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetColumnOffset(ColumnIndex);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColumnOffset(int32 ColumnIndex, float OffsetX)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetColumnOffset(ColumnIndex, OffsetX);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetColumnsCount()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetColumnsCount();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabBar, AdvancedDisplay = 1, DisplayName = "Tab Bar"), BlueprintInternalUseOnly)
	static bool BeginTabBar(FName Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabBarFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginTabBar(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", BlueprintInternalUseOnly)
	static void EndTabBar()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndTabBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabItem, AdvancedDisplay = 1, DisplayName = "Tab Item"), BlueprintInternalUseOnly)
	static bool BeginTabItem(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginTabItem(TCHAR_TO_UTF8(*Label.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabItem, AdvancedDisplay = 1, DisplayName = "Tab Item (Open State)"), BlueprintInternalUseOnly)
	static bool BeginTabItemOpenState(FName Label, bool& IsOpen, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::BeginTabItem(TCHAR_TO_UTF8(*Label.ToString()), &IsOpen, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", BlueprintInternalUseOnly)
	static void EndTabItem()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndTabItem();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool TabItemButton(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::TabItemButton(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetTabItemClosed(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetTabItemClosed(TCHAR_TO_UTF8(*Label.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Disabling", meta = (ImGuiScopeExit = EndDisabled, AdvancedDisplay = 0, DisplayName = "Disabled Scope"), BlueprintInternalUseOnly)
	static void BeginDisabled(bool Disabled = true)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::BeginDisabled(Disabled);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Disabling", BlueprintInternalUseOnly)
	static void EndDisabled()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::EndDisabled();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Clipping", meta = (ImGuiScopeExit = PopClipRect, AdvancedDisplay = 0, DisplayName = "Clip Rect Scope"), BlueprintInternalUseOnly)
	static void PushClipRect(const FVector2D& ClipRectMin, const FVector2D& ClipRectMax, bool IntersectWithCurrentClipRect)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::PushClipRect(ImVec2{ ClipRectMin }, ImVec2{ ClipRectMax }, IntersectWithCurrentClipRect);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Clipping", BlueprintInternalUseOnly)
	static void PopClipRect()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImGui::PopClipRect();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Focus", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetItemDefaultFocus()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetItemDefaultFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Focus", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void SetKeyboardFocusHere(int32 Offset = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetKeyboardFocusHere(Offset);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Overlapping", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemAllowOverlap()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextItemAllowOverlap();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemHovered()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemActive()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemFocused()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemClicked()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemClicked();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemVisible()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemVisible();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemEdited()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemEdited();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemActivated()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemActivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemDeactivated()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemDeactivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemDeactivatedAfterEdit()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemDeactivatedAfterEdit();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemToggledOpen()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsItemToggledOpen();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemHovered()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsAnyItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemActive()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsAnyItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemFocused()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsAnyItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiId GetItemID()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetItemID();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectMin()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetItemRectMin() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectMax()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetItemRectMax() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectSize()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetItemRectSize() };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyDown(FKey Key)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsKeyDown(UnrealImGui::ConvertKey(Key));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsKeyPressed(FKey Key, bool Repeat = true)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsKeyPressed(UnrealImGui::ConvertKey(Key), Repeat);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyReleased(FKey Key)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsKeyReleased(UnrealImGui::ConvertKey(Key));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyChordPressed(FKey Key, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiModKey"))int32 ModKey)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsKeyChordPressed(UnrealImGui::ConvertKey(Key) | ModKey);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetKeyPressedAmount(FKey Key, float RepeatDelay, float Rate)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetKeyPressedAmount(UnrealImGui::ConvertKey(Key), RepeatDelay, Rate);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FName GetKeyName(FKey Key)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return UTF8_TO_TCHAR(ImGui::GetKeyName(UnrealImGui::ConvertKey(Key)));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextFrameWantCaptureKeyboard(bool WantCaptureKeyboard)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextFrameWantCaptureKeyboard(WantCaptureKeyboard);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsMouseDown(EImGuiMouseButton Button)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseDown((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseClicked(EImGuiMouseButton Button, bool Repeat = false)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseClicked((ImGuiMouseButton)Button, Repeat);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMouseReleased(EImGuiMouseButton Button)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseReleased((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMouseDoubleClicked(EImGuiMouseButton Button)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseDoubleClicked((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static int32 GetMouseClickedCount(EImGuiMouseButton Button)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return ImGui::GetMouseClickedCount((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseHoveringRect(FVector2D Min, FVector2D Max, bool Clip = true)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseHoveringRect(ImVec2{ Min }, ImVec2{ Max }, Clip);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMousePosValid(FVector2D MousePos)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		const ImVec2 Pos{ MousePos };
		return ImGui::IsMousePosValid(&Pos);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsAnyMouseDown()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsAnyMouseDown();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static FVector2D GetMousePos()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetMousePos() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static FVector2D GetMousePosOnOpeningCurrentPopup()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetMousePosOnOpeningCurrentPopup() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseDragging(EImGuiMouseButton Button, float LockThreshold = -1.0f)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImGui::IsMouseDragging((ImGuiMouseButton)Button, LockThreshold);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
    static FVector2D GetMouseDragDelta(EImGuiMouseButton Button = EImGuiMouseButton::Left, float LockThreshold = -1.0f)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FVector2D{ ImGui::GetMouseDragDelta((ImGuiMouseButton)Button, LockThreshold) };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
    static void ResetMouseDragDelta(EImGuiMouseButton Button = EImGuiMouseButton::Left)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::ResetMouseDragDelta((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static EImGuiMouseCursor GetMouseCursor()
	{
		if (!CheckImGuiContextThrowError()) { return EImGuiMouseCursor::Arrow; }
		return EImGuiMouseCursor{ (uint8)ImGui::GetMouseCursor() };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static void SetMouseCursor(EImGuiMouseCursor CursorType)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetMouseCursor((ImGuiMouseCursor)CursorType);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static void SetNextFrameWantCaptureMouse(bool WantCaptureKeyboard)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetNextFrameWantCaptureMouse(WantCaptureKeyboard);
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Clipboard", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FString GetClipboardText()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return UTF8_TO_TCHAR(ImGui::GetClipboardText());
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Clipboard", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetClipboardText(const FString& Text)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImGui::SetClipboardText(TCHAR_TO_UTF8(*Text));
	}
};
