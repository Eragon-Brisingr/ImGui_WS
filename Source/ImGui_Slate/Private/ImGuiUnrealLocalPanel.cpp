// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiUnrealLocalPanel.h"

#include "imgui.h"
#include "ImGuiUnrealContextManager.h"
#include "SImGuiPanel.h"
#include "UnrealImGuiString.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformFileManager.h"
#include "Widgets/SWindow.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

namespace ImGui_WS::LocalPanel
{
TWeakPtr<SWindow> WindowPtr;
struct FViewportPanel
{
	TWeakPtr<SImGuiPanel> Panel;
	int32 ContextIndex;
};
TMap<TWeakObjectPtr<UWorld>, FViewportPanel> ViewportPanelMap;
enum class ELocalPanelMode : int32
{
	GameViewport = 0,
	SingleWindow = 1,
	DockWindow = 2,
};
TAutoConsoleVariable<int32> CVarLocalPanelMode
{
	TEXT("ImGui.WS.LocalPanelMode"),
	(int32)ELocalPanelMode::DockWindow,
	TEXT("0: open in game viewport\n")
	TEXT("1: open as single window (desktop platform only)\n")
	TEXT("2: open as dockable window (editor only)")
};
TSharedPtr<SImGuiPanel> CreatePanel(int32& ContextIndex)
{
	UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
	if (Manager == nullptr)
	{
		return nullptr;
	}

	const TSharedRef<SImGuiPanel> Panel = SNew(SImGuiPanel)
		.OnImGuiTick_Lambda([ManagerPtr = TWeakObjectPtr<UImGuiUnrealContextManager>(Manager), &ContextIndex](float DeltaSeconds)
		{
			if (UImGuiUnrealContextManager* Manager = ManagerPtr.Get())
			{
				Manager->DrawViewport(ContextIndex, DeltaSeconds);
			}
		});

	ImGuiContext* OldContent = ImGui::GetCurrentContext();
	ON_SCOPE_EXIT
	{
		ImGui::SetCurrentContext(OldContent);
	};
	ImGui::SetCurrentContext(Panel->GetContext());

	ImGuiIO& IO = ImGui::GetIO();

	// Enable Docking
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
	// Make sure that directory is created.
	PlatformFile.CreateDirectory(*IniDirectory);
	static const UnrealImGui::FUTF8String IniFilePath = IniDirectory / TEXT("Imgui_WS_Window.ini");
	IO.IniFilename = IniFilePath.GetData();
	return Panel;
}
FName GetWindowTabId()
{
	static FName TabId = []
	{
		const FName Id = TEXT("ImGui_WS_Window");
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Id, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
		{
			static int32 ContextIndex = INDEX_NONE;
			TSharedPtr<SWidget> Panel = CreatePanel(ContextIndex);
			if (Panel == nullptr)
			{
				Panel = SNullWidget::NullWidget;
			}
			TSharedRef<SDockTab> NewTab = SNew(SDockTab)
			.TabRole(NomadTab)
			.Label(LOCTEXT("ImGui_WS_WindowTitle", "ImGui WS"))
			[
				Panel.ToSharedRef()
			];
			NewTab->SetTabIcon(FAppStyle::Get().GetBrush("LevelEditor.Tab"));
			return NewTab;
		}));
		return Id;
	}();
	return TabId;
}
FAutoConsoleCommand OpenImGuiWindowCommand
{
	TEXT("ImGui.WS.OpenPanel"),
	TEXT("Open ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
		if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
		{
			FGlobalTabmanager::Get()->TryInvokeTab(GetWindowTabId());
		}
		else if (PanelMode == ELocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
		{
			if (WindowPtr.IsValid())
			{
				return;
			}
			static int32 ContextIndex = INDEX_NONE;
			const TSharedPtr<SImGuiPanel> Panel = CreatePanel(ContextIndex);
			if (Panel == nullptr)
			{
				return;
			}
			const TSharedRef<SWindow> Window =
				SNew(SWindow)
				.Title(LOCTEXT("ImGui_WS_WindowTitle", "ImGui WS"))
				.ClientSize(FVector2f(1000.f, 800.f))
				[
					Panel.ToSharedRef()
				];
			WindowPtr = Window;
			FSlateApplication::Get().AddWindow(Window);
		}
		else
		{
			for (auto It = ViewportPanelMap.CreateIterator(); It; ++It)
			{
				if (It.Key().IsValid())
				{
					continue;
				}
				It.RemoveCurrent();
			}
			FViewportPanel& ViewportPanel = ViewportPanelMap.FindOrAdd(World);
			if (ViewportPanel.Panel.IsValid())
			{
				return;
			}
			UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
			if (GameViewport == nullptr)
			{
				return;
			}
			const UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
			if (Manager == nullptr)
			{
				return;
			}
			ViewportPanel.ContextIndex = Manager->GetWorldSubsystems().IndexOfByPredicate([World](const UImGuiUnrealContextWorldSubsystem* E)
			{
				return E && E->GetWorld() == World;
			});
			const TSharedPtr<SImGuiPanel> Panel = CreatePanel(ViewportPanel.ContextIndex);
			if (Panel == nullptr)
			{
				return;
			}
			ViewportPanel.Panel = Panel;
			GameViewport->AddViewportWidgetContent(Panel.ToSharedRef(), INT32_MAX - 1);
		}
	})
};
FAutoConsoleCommand CloseImGuiWindowCommand
{
	TEXT("ImGui.WS.ClosePanel"),
	TEXT("Close ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
		if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
		{
			if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(GetWindowTabId()))
			{
				Tab->RequestCloseTab();
			}
		}
		else if (PanelMode == ELocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
		{
			if (WindowPtr.IsValid())
			{
				WindowPtr.Pin()->RequestDestroyWindow();
			}
		}
		else
		{
			const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(World);
			if (ViewportPanel && ViewportPanel->Panel.IsValid())
			{
				if (UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr)
				{
					GameViewport->RemoveViewportWidgetContent(ViewportPanel->Panel.Pin().ToSharedRef());
				}
				ViewportPanelMap.Remove(World);
			}
		}
	})
};
}

#undef LOCTEXT_NAMESPACE
