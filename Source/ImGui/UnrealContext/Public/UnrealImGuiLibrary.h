// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiWrapper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealImGuiLibrary.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiWindowFlags : int32
{
	None                      = ImGuiWindowFlags_None                     ,
    NoTitleBar                = ImGuiWindowFlags_NoTitleBar               ,   // Disable title-bar
    NoResize                  = ImGuiWindowFlags_NoResize                 ,   // Disable user resizing with the lower-right grip
    NoMove                    = ImGuiWindowFlags_NoMove                   ,   // Disable user moving the window
    NoScrollbar               = ImGuiWindowFlags_NoScrollbar              ,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    NoScrollWithMouse         = ImGuiWindowFlags_NoScrollWithMouse        ,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    NoCollapse                = ImGuiWindowFlags_NoCollapse               ,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    AlwaysAutoResize          = ImGuiWindowFlags_AlwaysAutoResize         ,   // Resize every window to its content every frame
    NoBackground              = ImGuiWindowFlags_NoBackground             ,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    NoSavedSettings           = ImGuiWindowFlags_NoSavedSettings          ,   // Never load/save settings in .ini file
    NoMouseInputs             = ImGuiWindowFlags_NoMouseInputs            ,   // Disable catching mouse, hovering test with pass through.
    MenuBar                   = ImGuiWindowFlags_MenuBar                  ,  // Has a menu-bar
    HorizontalScrollbar       = ImGuiWindowFlags_HorizontalScrollbar      ,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    NoFocusOnAppearing        = ImGuiWindowFlags_NoFocusOnAppearing       ,  // Disable taking focus when transitioning from hidden to visible state
    NoBringToFrontOnFocus     = ImGuiWindowFlags_NoBringToFrontOnFocus    ,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    AlwaysVerticalScrollbar   = ImGuiWindowFlags_AlwaysVerticalScrollbar  ,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    NoNavInputs               = ImGuiWindowFlags_NoNavInputs              ,  // No gamepad/keyboard navigation within the window
    NoNavFocus                = ImGuiWindowFlags_NoNavFocus               ,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    UnsavedDocument           = ImGuiWindowFlags_UnsavedDocument          ,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    NoDocking                 = ImGuiWindowFlags_NoDocking                ,  // Disable docking of this window
    NoNav                     = ImGuiWindowFlags_NoNav                    ,
    NoDecoration              = ImGuiWindowFlags_NoDecoration             ,
    NoInputs                  = ImGuiWindowFlags_NoInputs                 ,
};
ENUM_CLASS_FLAGS(EImGuiWindowFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiSelectableFlags : int32
{
	None             = ImGuiSelectableFlags_None               ,
	DontClosePopups  = ImGuiSelectableFlags_DontClosePopups    ,   // Clicking this doesn't close parent popup window
	SpanAllColumns   = ImGuiSelectableFlags_SpanAllColumns     ,   // Frame will span all columns of its container table (text will still fit in current column)
	AllowDoubleClick = ImGuiSelectableFlags_AllowDoubleClick   ,   // Generate press events on double clicks too
	Disabled         = ImGuiSelectableFlags_Disabled           ,   // Cannot be selected, display grayed out text
	AllowOverlap     = ImGuiSelectableFlags_AllowOverlap       ,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
};
ENUM_CLASS_FLAGS(EImGuiSelectableFlags);

UCLASS()
class IMGUI_API UUnrealImGuiLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemHovered()
	{
		return ImGui::IsItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemActive()
	{
		return ImGui::IsItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemFocused()
	{
		return ImGui::IsItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemClicked()
	{
		return ImGui::IsItemClicked();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemVisible()
	{
		return ImGui::IsItemVisible();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemEdited()
	{
		return ImGui::IsItemEdited();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemActivated()
	{
		return ImGui::IsItemActivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemDeactivated()
	{
		return ImGui::IsItemDeactivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemDeactivatedAfterEdit()
	{
		return ImGui::IsItemDeactivatedAfterEdit();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsItemToggledOpen()
	{
		return ImGui::IsItemToggledOpen();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsAnyItemHovered()
	{
		return ImGui::IsAnyItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsAnyItemActive()
	{
		return ImGui::IsAnyItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static bool ImGui_IsAnyItemFocused()
	{
		return ImGui::IsAnyItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static int32 ImGui_GetItemID()
	{
		return ImGui::GetItemID();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static FVector2D ImGui_GetItemRectMin()
	{
		return FVector2D{ ImGui::GetItemRectMin() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static FVector2D ImGui_GetItemRectMax()
	{
		return FVector2D{ ImGui::GetItemRectMax() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Query")
	static FVector2D ImGui_GetItemRectSize()
	{
		return FVector2D{ ImGui::GetItemRectSize() };
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiScopeExit = ImGui_End, ImGuiAlwaysExit, Name = "NSLOCTEXT(\"ImGui_WS\", \"BP_DefaultTitle\", \"Untitle\")", DisplayName = "ImGui Panel", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool ImGui_Begin(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = EImGuiWindowFlags))int32 Flags)
	{
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiScopeExit = ImGui_End, ImGuiAlwaysExit, Name = "NSLOCTEXT(\"ImGui_WS\", \"BP_DefaultTitle\", \"Untitle\")", DisplayName = "ImGui Panel (Open State)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ImGui_BeginWithOpenState(FText Name, UPARAM(Ref)bool& OpenState, UPARAM(meta = (Bitmask, BitmaskEnum = EImGuiWindowFlags))int32 Flags)
	{
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), &OpenState, Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", BlueprintInternalUseOnly)
	static void ImGui_End()
	{
		ImGui::End();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (AdvancedDisplay = 1))
	static void ImGui_Text(FText Text, FLinearColor Color = FLinearColor(1.0f,1.0f,1.0f))
	{
		const FColor SRGB = Color.ToFColorSRGB();
		ImGui::TextColored(ImVec4(SRGB.R/255.f, SRGB.G/255.f, SRGB.B/255.f, SRGB.A/255.f), "%s", TCHAR_TO_UTF8(*Text.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (AdvancedDisplay = "OffsetFromStartX,Spacing"))
	static void ImGui_SameLine(float OffsetFromStartX, float Spacing = -1)
	{
		ImGui::SameLine(OffsetFromStartX, Spacing);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (AdvancedDisplay = 1, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_Button(FText Label, FVector2D Size)
	{
		return ImGui::Button(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_SmallButton(FText Label)
	{
		return ImGui::SmallButton(TCHAR_TO_UTF8(*Label.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InvisibleButton(FText Label, FVector2D Size)
	{
		return ImGui::InvisibleButton(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_CheckBox(FText Label, UPARAM(Ref)bool& Value)
	{
		return ImGui::Checkbox(TCHAR_TO_UTF8(*Label.ToString()), &Value);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_CheckboxFlags(FText Label, UPARAM(Ref)int32& Flags, int32 FlagsValue)
	{
		return ImGui::CheckboxFlags(TCHAR_TO_UTF8(*Label.ToString()), &Flags, FlagsValue);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_RadioButton(FText Label, bool bActive)
	{
		return ImGui::RadioButton(TCHAR_TO_UTF8(*Label.ToString()), bActive);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Basic", meta = (ImGuiBoolTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ImGui_Selectable(FText Label, bool bActive, UPARAM(meta = (Bitmask, BitmaskEnum = EImGuiSelectableFlags))int32 Flags, FVector2D Size)
	{
		return ImGui::Selectable(TCHAR_TO_UTF8(*Label.ToString()), bActive, Flags, ImVec2{ Size });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputByte(FText Label, UPARAM(Ref)uint8& Value, uint8 Step = 1, uint8 StepFast = 10)
	{
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputInt(FText Label, UPARAM(Ref)int32& Value, int32 Step = 1, int32 StepFast = 100)
	{
		return ImGui::InputInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputInt64(FText Label, UPARAM(Ref)int64& Value, int64 Step = 1, int64 StepFast = 100)
	{
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputFloat(FText Label, UPARAM(Ref)float& Value, float Step = 0.f, float StepFast = 0.f, FString Format = TEXT("%3.f"))
	{
		return ImGui::InputFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputVector2f(FText Label, UPARAM(Ref)FVector2f& Value, FString Format = TEXT("%3.f"))
	{
		return ImGui::InputFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputVector3f(FText Label, UPARAM(Ref)FVector3f& Value, FString Format = TEXT("%3.f"))
	{
		return ImGui::InputFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputDouble(FText Label, UPARAM(Ref)double& Value, double Step = 0.f, double StepFast = 0.f, FString Format = TEXT("%3.f"))
	{
		return ImGui::InputDouble(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputVector2D(FText Label, UPARAM(Ref)FVector2D& Value, FString Format = TEXT("%3.f"))
	{
		static_assert(std::is_same_v<FVector2D::FReal, double>);
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 2, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (AdvancedDisplay = 2, ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputVector(FText Label, UPARAM(Ref)FVector& Value, FString Format = TEXT("%3.f"))
	{
		static_assert(std::is_same_v<FVector2D::FReal, double>);
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 3, TCHAR_TO_UTF8(*Format));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_LinearColorEdit(FText Label, UPARAM(Ref)FLinearColor& Value)
	{
		return ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &Value.R);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_ColorEdit(FText Label, UPARAM(Ref)FColor& Value)
	{
		FLinearColor V = Value;
		const bool Ret = ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &V.R);
		if (Ret)
		{
			Value = V.ToFColor(false);
		}
		return Ret;
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputText(FText Label, UPARAM(Ref)FString& Value)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputText(TCHAR_TO_UTF8(*Label.ToString()), UTF8String);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputTextMultiline(FText Label, UPARAM(Ref)FString& Value, FVector2f Size)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputTextMultiline(TCHAR_TO_UTF8(*Label.ToString()), UTF8String, ImVec2{ Size });
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Input", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_InputTextWithHint(FText Label, FText Hint, UPARAM(Ref)FString& Value)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputTextWithHint(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Hint.ToString()), UTF8String);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", meta = (ImGuiScopeExit = ImGui_EndMainMenuBar, DisplayName = "ImGui Main Menu Bar"), BlueprintInternalUseOnly)
	static bool ImGui_BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", BlueprintInternalUseOnly)
	static void ImGui_EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", meta=(ImGuiScopeExit = ImGui_EndMenuBar, DisplayName = "ImGui Menu Bar"), BlueprintInternalUseOnly)
	static bool ImGui_BeginMenuBar()
	{
		return ImGui::BeginMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", BlueprintInternalUseOnly)
	static void ImGui_EndMenuBar()
	{
		ImGui::EndMenuBar();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", meta=(ImGuiScopeExit = ImGui_EndMenu, DisplayName = "ImGui Menu", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool ImGui_BeginMenu(FText Label, bool Enabled = true)
	{
		return ImGui::BeginMenu(TCHAR_TO_UTF8(*Label.ToString()), Enabled);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", BlueprintInternalUseOnly)
	static void ImGui_EndMenu()
	{
		return ImGui::EndMenu();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", meta = (ImGuiBoolTrigger), BlueprintInternalUseOnly)
	static bool ImGui_MenuItem(FText Label, FString Shortcut, bool Selected, bool Enabled = true)
	{
		return ImGui::MenuItem(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Shortcut), Selected, Enabled);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", meta=(ImGuiScopeExit = ImGui_EndTooltip, DisplayName = "ImGui Tooltip"), BlueprintInternalUseOnly)
	static bool ImGui_BeginTooltip()
	{
		return ImGui::BeginTooltip();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", BlueprintInternalUseOnly)
	static void ImGui_EndTooltip()
	{
		return ImGui::EndTooltip();
	}

	UFUNCTION(Blueprintable, Category="ImGui|Tooltip", meta = (DisplayName = "ImGui Set Tooltip"))
	static void ImGui_SetTooltip(FText Text)
	{
		ImGui::SetTooltip(TCHAR_TO_UTF8(*Text.ToString()));
	}
};
