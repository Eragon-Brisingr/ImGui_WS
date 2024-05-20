// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiUnrealContextManager.h"

#include "imgui.h"
#include "imgui_notify.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"


UImGuiUnrealContextWorldSubsystem* UImGuiUnrealContextWorldSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}
	const UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
	return World->GetSubsystem<UImGuiUnrealContextWorldSubsystem>();
}

void UImGuiUnrealContextWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UImGuiUnrealContextManager* Manager = UImGuiUnrealContextManager::GetChecked();
	Manager->WorldSubsystems.Add(this);
}

void UImGuiUnrealContextWorldSubsystem::Deinitialize()
{
	UImGuiUnrealContextManager* Manager = UImGuiUnrealContextManager::GetChecked();
	Manager->WorldSubsystems.RemoveSingle(this);

	Super::Deinitialize();
}

UImGuiUnrealContextManager* UImGuiUnrealContextManager::GetChecked()
{
	check(GEngine);
	UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
	check(Manager);
	return Manager;
}

FImGuiUnrealContext* UImGuiUnrealContextManager::GetImGuiContext(const UWorld* World)
{
	if (World == nullptr)
	{
		return nullptr;
	}
	if (World->IsGameWorld())
	{
		UImGuiUnrealContextWorldSubsystem* WorldSubsystem = World->GetSubsystem<UImGuiUnrealContextWorldSubsystem>();
		return WorldSubsystem ? &WorldSubsystem->Context : nullptr;
	}
	return GetImGuiEditorContext();
}

FImGuiUnrealEditorContext* UImGuiUnrealContextManager::GetImGuiEditorContext()
{
#if WITH_EDITOR
	UImGuiUnrealContextManager* Manager = UImGuiUnrealContextManager::GetChecked();
	return &Manager->EditorContext;
#else
	return nullptr;
#endif
}

void UImGuiUnrealContextManager::DrawViewport(int32& ContextIndex, float DeltaSeconds)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("ImGui_WS"))
		{
			DrawContextContent(ContextIndex);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

#if WITH_EDITOR
	if (ContextIndex == EditorContextIndex || ContextIndex >= WorldSubsystems.Num())
	{
		const FImGuiUnrealEditorContext& DrawContext = EditorContext;
		if (DrawContext.bAlwaysDrawDefaultLayout || DrawContext.OnDraw.IsBound() == false)
		{
			if (GWorld && DrawContext.EditorDrawer)
			{
				DrawContext.EditorDrawer(DeltaSeconds);
			}
		}
		DrawContext.OnDraw.Broadcast(DeltaSeconds);
	}
	else
#endif
	if (ContextIndex < WorldSubsystems.Num())
	{
		const UImGuiUnrealContextWorldSubsystem* WorldSubsystem = WorldSubsystems[ContextIndex];
		WorldSubsystem->Context.OnDraw.Broadcast(DeltaSeconds);
	}

	{
		// Notify
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
		ImGui::RenderNotifications();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
}

void UImGuiUnrealContextManager::DrawContextContent(int32& ContextIndex)
{
#if WITH_EDITOR
	if (ImGui::RadioButton(TCHAR_TO_UTF8(*FString::Printf(TEXT("Editor"))), ContextIndex == EditorContextIndex || ContextIndex >= WorldSubsystems.Num()))
	{
		ContextIndex = EditorContextIndex;
	}
	if (WorldSubsystems.Num() > 0)
	{
		ImGui::Separator();
	}
#endif
	for (int32 Idx = 0; Idx < WorldSubsystems.Num(); ++Idx)
	{
		const UWorld* World = WorldSubsystems[Idx]->GetWorld();
		FString WorldDesc;
		switch(World->GetNetMode())
		{
		case NM_Client:
#if WITH_EDITOR
			WorldDesc = FString::Printf(TEXT("Client %d"), World->GetOutermost()->GetPIEInstanceID() - 1);
#else
			WorldDesc = FString::Printf(TEXT("Client %d"), Idx);
#endif
			break;
		case NM_DedicatedServer:
			WorldDesc = TEXT("DedicatedServer");
			break;
		case NM_ListenServer:
			WorldDesc = TEXT("Server");
			break;
		case NM_Standalone:
			WorldDesc = TEXT("Standalone");
		default:
			break;
		}
		if (ImGui::RadioButton(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d. %s"), Idx, *WorldDesc)), Idx == ContextIndex))
		{
			ContextIndex = Idx;
		}
	}
}
