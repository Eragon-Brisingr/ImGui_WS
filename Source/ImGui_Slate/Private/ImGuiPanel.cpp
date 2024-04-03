// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiPanel.h"

#include "imgui_internal.h"
#include "SImGuiPanel.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

UImGuiPanel::UImGuiPanel()
{
#if WITH_EDITORONLY_DATA
	AccessibleBehavior = ESlateAccessibleBehavior::Summary;
	bCanChildrenBeAccessible = false;
#endif
}

TSharedRef<SWidget> UImGuiPanel::RebuildWidget()
{
#if WITH_EDITOR
	if (IsDesignTime())
	{
		return SNew(STextBlock)
			.Text(LOCTEXT("ImGuiPreviewWorldText", "ImGui Panel"))
			.Justification(ETextJustify::Center);
	}
#endif

	const TSharedRef<SImGuiPanel> ImGuiPanel = SNew(SImGuiPanel)
		.OnImGuiTick_Lambda([this](float DeltaSeconds)
		{
#if WITH_EDITOR
			FEditorScriptExecutionGuard EditorScriptExecutionGuard;
#endif
			OnImGuiTick.Broadcast(DeltaSeconds);
		})
		.DesiredSize_Lambda([this]
		{
			return DesiredSize;
		});

	ImGuiIO& IO = ImGuiPanel->GetContext()->IO;
	if (IniFileName.Len() == 0)
	{
		IO.IniFilename = nullptr;
	}
	else
	{
		if (IniFilePath.Len() == 0)
		{
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
			PlatformFile.CreateDirectory(*IniDirectory);
			IniFilePath = IniDirectory / IniFileName;
		}
		IO.IniFilename = IniFilePath.GetData();
	}
	IO.ConfigFlags = ConfigFlags;
	return ImGuiPanel;
}

#if WITH_EDITOR
const FText UImGuiPanel::GetPaletteCategory()
{
	return LOCTEXT("Common", "Common");
}
#endif

#undef LOCTEXT_NAMESPACE
