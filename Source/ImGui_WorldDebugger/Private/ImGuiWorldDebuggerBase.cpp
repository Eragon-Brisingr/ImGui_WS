// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerBase.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiStat.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

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
		SpawnParameters.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		SpawnParameters.ObjectFlags = RF_Transient;
		World->SpawnActor<AImGuiWorldDebuggerBase>(SpawnedDebuggerClass, SpawnParameters);
	}

	void DestroyDebugger(UWorld* World)
	{
		check(World);
		check(IsEnableDebugWorld(World));

		for (TActorIterator<AImGuiWorldDebuggerBase> It(World); It; ++It)
		{
			It->Destroy();
		}
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
	}

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
				if (IsEnableDebugWorld(World))
				{
					SpawnDebugger(World);
				}
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
				if (IsEnableDebugWorld(World))
				{
					DestroyDebugger(World);
				}
			}
		}
	}
}

AImGuiWorldDebuggerBase::AImGuiWorldDebuggerBase()
	: bEnableImGuiWorldDebugger(true)
{
	PanelBuilder = CreateDefaultSubobject<UUnrealImGuiPanelBuilder>(GET_MEMBER_NAME_CHECKED(ThisClass, PanelBuilder));
	PanelBuilder->DockSpaceName = TEXT("ImGuiWorldDebuggerDockSpace");
}

void AImGuiWorldDebuggerBase::BeginPlay()
{
	Super::BeginPlay();

	FImGuiUnrealContext* Context = UImGuiUnrealContextManager::GetImGuiContext(GetWorld());
	if (ensure(Context))
	{
		Context->OnDraw.AddUObject(this, &AImGuiWorldDebuggerBase::DrawDebugPanel);
	}
	PanelBuilder->Register(this);
}

void AImGuiWorldDebuggerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PanelBuilder->Unregister(this);
	if (FImGuiUnrealContext* Context = UImGuiUnrealContextManager::GetImGuiContext(GetWorld()))
	{
		Context->OnDraw.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AImGuiWorldDebuggerBase::DrawDebugPanel(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebugger_DrawDebugPanel"), STAT_ImGuiWorldDebugger_DrawDebugPanel, STATGROUP_ImGui);

	auto Config = GetMutableDefault<AImGuiWorldDebuggerBase>();
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

#undef LOCTEXT_NAMESPACE
