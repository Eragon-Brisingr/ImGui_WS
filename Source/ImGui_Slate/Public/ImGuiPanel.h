// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "Components/Widget.h"
#include "Containers/Utf8String.h"
#include "ImGuiPanel.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiConfigFlags : int32
{
    None                 = ImGuiConfigFlags_None                   ,
    NavEnableKeyboard    = ImGuiConfigFlags_NavEnableKeyboard      ,   // Master keyboard navigation enable flag. Enable full Tabbing + directional arrows + space/enter to activate.
    NavEnableGamepad     = ImGuiConfigFlags_NavEnableGamepad       ,   // Master gamepad navigation enable flag. Backend also needs to set ImGuiBackendFlags_HasGamepad.
    NoMouse              = ImGuiConfigFlags_NoMouse                ,   // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
    NoMouseCursorChange  = ImGuiConfigFlags_NoMouseCursorChange    ,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

    // [BETA] Docking
    DockingEnable        = ImGuiConfigFlags_DockingEnable          ,   // Docking enable flags.
};
ENUM_CLASS_FLAGS(EImGuiConfigFlags);

UCLASS(meta = (DisplayName = "ImGui Panel"))
class IMGUI_SLATE_API UImGuiPanel : public UWidget
{
	GENERATED_BODY()
public:
	UImGuiPanel();

	TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(EditAnywhere, Category = "ImGui")
	FVector2D DesiredSize{ 800, 600 };

	UPROPERTY(EditAnywhere, Category = "ImGui", meta = (Bitmask, BitmaskEnum = "/Script/ImGui_Slate.EImGuiConfigFlags"))
	int32 ConfigFlags = 0;

	UPROPERTY(EditAnywhere, Category = "ImGui")
	FString IniFileName;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImGuiTick, float, DeltaSeconds);
	UPROPERTY(BlueprintAssignable, Category="ImGui")
	FOnImGuiTick OnImGuiTick;

#if WITH_EDITOR
	const FText GetPaletteCategory() override;
#endif

private:
	FUtf8String IniFilePath;
};
