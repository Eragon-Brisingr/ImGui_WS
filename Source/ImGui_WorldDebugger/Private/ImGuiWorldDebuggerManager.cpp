// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerManager.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiStat.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

namespace ImGuiWorldDebuggerBootstrap
{
	void EnableDebugger(UWorld* World)
	{
		check(World);

		auto Manager = UImGuiWorldDebuggerManager::Get(World);
		if (Manager == nullptr)
		{
			return;
		}
		Manager->EnableDebugger();
	}

	void DisableDebugger(UWorld* World)
	{
		check(World);

		auto Manager = UImGuiWorldDebuggerManager::Get(World);
		if (Manager == nullptr)
		{
			return;
		}
		Manager->DisableDebugger();
	}

	bool bLaunchImGuiWorldDebugger = false;
	FAutoConsoleVariable EnableImGuiWorldDebugger
	{
		TEXT("ImGui.DebugGameWorld"),
		bLaunchImGuiWorldDebugger,
		TEXT("Enable Draw Game World Debug Map")
		TEXT("0: Disable")
		TEXT("1: Enable"),
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* ConsoleVariable)
		{
			const bool Enable = ConsoleVariable->GetBool();
			if (bLaunchImGuiWorldDebugger == Enable)
			{
				return;
			}
			bLaunchImGuiWorldDebugger = Enable;
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				if (UWorld* World = WorldContext.World())
				{
					if (Enable)
					{
						EnableDebugger(World);
					}
					else
					{
						DisableDebugger(World);
					}
				}
			}
		}),
		ECVF_Default
	};

	int32 RequireDebuggerCounter = 0;
	void RequireCreateDebugger()
	{
		RequireDebuggerCounter += 1;
		if (bLaunchImGuiWorldDebugger == false)
		{
			bLaunchImGuiWorldDebugger = true;

			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				if (World == nullptr)
				{
					continue;
				}
				EnableDebugger(World);
			}
		}
	}
	void RequireDestroyDebugger()
	{
		RequireDebuggerCounter -= 1;
		ensure(RequireDebuggerCounter >= 0);
		if (RequireDebuggerCounter == 0 && bLaunchImGuiWorldDebugger)
		{
			bLaunchImGuiWorldDebugger = false;
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				if (World == nullptr)
				{
					continue;
				}
				DisableDebugger(World);
			}
		}
	}
}

bool UImGuiWorldDebuggerManager::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UImGuiWorldDebuggerManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (ImGuiWorldDebuggerBootstrap::bLaunchImGuiWorldDebugger == false)
	{
		return;
	}

	EnableDebugger();
}

void UImGuiWorldDebuggerManager::Deinitialize()
{
	DisableDebugger();
	
	Super::Deinitialize();
}

void UImGuiWorldDebuggerManager::DrawDebugPanel(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebugger_DrawDebugPanel"), STAT_ImGuiWorldDebugger_DrawDebugPanel, STATGROUP_ImGui);

	auto Config = GetMutableDefault<UImGuiWorldDebuggerManager>();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::Separator();

			bool bEnable = Config->bEnableImGuiWorldDebugger;
			if (ImGui::Checkbox("ImGui World Debugger", &bEnable))
			{
				Config->bEnableImGuiWorldDebugger = bEnable;
				Config->SaveConfig();
			}
			if (Config->bEnableImGuiWorldDebugger)
			{
				PanelBuilder->DrawPanelStateMenu(this);
			}
			
			ImGui::EndMenu();
		}
		if (Config->bEnableImGuiWorldDebugger)
		{
			if (ImGui::BeginMenu("Layout"))
			{
				PanelBuilder->DrawLayoutStateMenu(this);

				ImGui::Separator();
				if (ImGui::MenuItem("Reset Layout"))
				{
					PanelBuilder->LoadDefaultLayout(this);
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
	if (Config->bEnableImGuiWorldDebugger == false)
	{
		return;
	}
	
	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(Viewport->WorkPos);
	ImGui::SetNextWindowSize(Viewport->WorkSize);
	ImGui::SetNextWindowViewport(Viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDocking;
	WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (ImGui::Begin("Background", nullptr, WindowFlags))
	{
		PanelBuilder->DrawPanels(this, DeltaSeconds);
	}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void UImGuiWorldDebuggerManager::EnableDebugger()
{
	if (bIsEnable)
	{
		return;
	}
	bIsEnable = true;
	
	FImGuiUnrealContext* Context = UImGuiUnrealContextManager::GetImGuiContext(GetWorld());
	if (ensure(Context))
	{
		Context->OnDraw.AddUObject(this, &UImGuiWorldDebuggerManager::DrawDebugPanel);
	}
	PanelBuilder = NewObject<UUnrealImGuiPanelBuilder>(this, GET_MEMBER_NAME_CHECKED(ThisClass, PanelBuilder));
	PanelBuilder->DockSpaceName = TEXT("ImGuiWorldDebuggerDockSpace");
	PanelBuilder->Register(this);
}

void UImGuiWorldDebuggerManager::DisableDebugger()
{
	if (bIsEnable == false)
	{
		return;
	}
	bIsEnable = false;

	PanelBuilder->Unregister(this);
	PanelBuilder->MarkAsGarbage();
	PanelBuilder = nullptr;
	if (FImGuiUnrealContext* Context = UImGuiUnrealContextManager::GetImGuiContext(GetWorld()))
	{
		Context->OnDraw.RemoveAll(this);
	}
}

#undef LOCTEXT_NAMESPACE
