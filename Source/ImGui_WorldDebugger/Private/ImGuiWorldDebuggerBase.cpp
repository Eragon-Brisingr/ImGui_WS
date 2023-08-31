// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerBase.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "ImGui_WS_Manager.h"
#include "UnrealImGuiStat.h"

#define LOCTEXT_NAMESPACE "ImGuiWorldDebugger"

namespace ImGuiWorldDebuggerBootstrap
{
	TSubclassOf<AImGuiWorldDebuggerBase> DebuggerClass;
	FName ImGuiWorldDebuggerName = TEXT("ImGuiWorldDebugger");

	bool IsEnableDebugWorld(UWorld* World)
	{
		return World->IsPreviewWorld() == false && World->IsGameWorld();
	}
	
	void SpawnDebugger(UWorld* World)
	{
		check(World);
		check(IsEnableDebugWorld(World));
	
		UClass* SpawnedDebuggerClass = DebuggerClass ? *DebuggerClass : AImGuiWorldDebuggerBase::StaticClass();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ImGuiWorldDebuggerName;
		SpawnParameters.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_ReturnNull;
		SpawnParameters.ObjectFlags = RF_Transient;
#if WITH_EDITOR
		SpawnParameters.bHideFromSceneOutliner = true;
#endif
		World->SpawnActor<AImGuiWorldDebuggerBase>(SpawnedDebuggerClass, SpawnParameters);
	}

	void DestroyDebugger(UWorld* World)
	{
		check(World);
		check(IsEnableDebugWorld(World));

		for (TActorIterator<AImGuiWorldDebuggerBase> It(World); It; ++It)
		{
			It->Destroy();
			// 直接删了，避免没GC导致再次创建同名的失败
			It->ConditionalBeginDestroy();
		}
	}

	bool bLaunchImGuiWorldDebugger = true;
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
					if (IsEnableDebugWorld(World))
					{
						if (Enable)
						{
							SpawnDebugger(World);
						}
						else
						{
							DestroyDebugger(World);
						}
					}
				}
			}
		}),
		ECVF_Default
	};

	void PostWorldInitialization(UWorld* World, const UWorld::InitializationValues /*IVS*/)
	{
		if (bLaunchImGuiWorldDebugger == false)
		{
			return;
		}

		if (IsEnableDebugWorld(World))
		{
			SpawnDebugger(World);
		}
	};

}

AImGuiWorldDebuggerBase::AImGuiWorldDebuggerBase()
	: bEnableImGuiWorldDebugger(true)
{
	PanelBuilder.DockSpaceName = TEXT("ImGuiWorldDebuggerDockSpace");
}

void AImGuiWorldDebuggerBase::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	FImGui_WS_Context* Context = UImGui_WS_Manager::GetImGuiContext(World);
	if (ensure(Context))
	{
		Context->OnDraw.AddUObject(this, &AImGuiWorldDebuggerBase::DrawDebugPanel);
	}
	PanelBuilder.Register(this);
}

void AImGuiWorldDebuggerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PanelBuilder.Unregister(this);
	if (const UWorld* World = GetWorld())
	{
		if (FImGui_WS_Context* Context = UImGui_WS_Manager::GetImGuiContext(World))
		{
			Context->OnDraw.RemoveAll(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void AImGuiWorldDebuggerBase::DrawDebugPanel(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebugger_DrawDebugPanel"), STAT_ImGuiWorldDebugger_DrawDebugPanel, STATGROUP_ImGui);
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::Separator();
			
			bool bEnable = bEnableImGuiWorldDebugger;
			if (ImGui::Checkbox("ImGui World Debugger", &bEnable))
			{
				bEnableImGuiWorldDebugger = bEnable;
				SaveConfig();
			}
			if (bEnableImGuiWorldDebugger)
			{
				PanelBuilder.DrawPanelStateMenu(this);
			}
			
			ImGui::EndMenu();
		}
		if (bEnableImGuiWorldDebugger)
		{
			if (ImGui::BeginMenu("Layout"))
			{
				PanelBuilder.DrawLayoutStateMenu(this);

				ImGui::Separator();
				if (ImGui::MenuItem("Reset Layout"))
				{
					PanelBuilder.LoadDefaultLayout(this);
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
	if (bEnableImGuiWorldDebugger == false)
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
		PanelBuilder.DrawPanels(this, DeltaSeconds);
		ImGui::End();
	}
}

#undef LOCTEXT_NAMESPACE
