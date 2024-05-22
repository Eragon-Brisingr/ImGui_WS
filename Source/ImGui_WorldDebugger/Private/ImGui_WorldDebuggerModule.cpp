#include "ImGui_WorldDebuggerModule.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Selection.h"
#endif
#include "ImGuiDelegates.h"
#include "ImGuiWorldDebuggerBase.h"
#include "ImGuiWorldDebuggerViewportPanel.h"

#define LOCTEXT_NAMESPACE "ImGuiUnrealEx"

void FImGui_WorldDebuggerModule::StartupModule()
{
	OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&ImGuiWorldDebuggerBootstrap::PostWorldInitialization);

	OnImGui_WS_EnableHandle = FImGuiDelegates::OnImGui_WS_Enable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireCreateDebugger);
	OnImGui_WS_DisableHandle = FImGuiDelegates::OnImGui_WS_Disable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireDestroyDebugger);
	OnImGuiLocalPanelEnableHandle = FImGuiDelegates::OnImGuiLocalPanelEnable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireCreateDebugger);
	OnImGuiLocalPanelDisableHandle = FImGuiDelegates::OnImGuiLocalPanelDisable.AddStatic(&ImGuiWorldDebuggerBootstrap::RequireDestroyDebugger);

#if WITH_EDITOR
	SelectObjectEventHandle = USelection::SelectObjectEvent.AddLambda([](UObject* Object)
	{
		if (Object->IsA<AActor>())
		{
			TArray<AActor*> SelectedActors;
			for (FSelectionIterator It = GEditor->GetSelectedActorIterator(); It; ++It)
			{
				if (AActor* Actor = Cast<AActor>(*It))
				{
					SelectedActors.Add(Actor);
				}
			}
			UImGuiWorldDebuggerViewportActorExtent::WhenEditorSelectionChanged(SelectedActors);
		}
	});
	SelectNoneEventHandle = USelection::SelectNoneEvent.AddLambda([]()
	{
		UImGuiWorldDebuggerViewportActorExtent::WhenEditorSelectionChanged(TArray<AActor*>{});
	});

	UImGuiWorldDebuggerViewportActorExtent::EditorSelectActors.BindLambda([](UWorld* World, const TSet<TObjectPtr<AActor>>& SelectedMetaEntities)
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		SelectedActors->BeginBatchSelectOperation();
		GEditor->SelectNone(false, true, true);
		for (AActor* Actor : SelectedMetaEntities)
		{
			if (Actor)
			{
				GEditor->SelectActor(Actor, true, false, true);
			}
		}
		SelectedActors->EndBatchSelectOperation(false);
		GEditor->NoteSelectionChange();
	});
#endif
}

void FImGui_WorldDebuggerModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(OnPostWorldInitializationHandle);

	FImGuiDelegates::OnImGui_WS_Enable.Remove(OnImGui_WS_EnableHandle);
	FImGuiDelegates::OnImGui_WS_Disable.Remove(OnImGui_WS_DisableHandle);
	FImGuiDelegates::OnImGuiLocalPanelEnable.Remove(OnImGuiLocalPanelEnableHandle);
	FImGuiDelegates::OnImGuiLocalPanelDisable.Remove(OnImGuiLocalPanelDisableHandle);

#if WITH_EDITOR
	USelection::SelectObjectEvent.Remove(SelectObjectEventHandle);
	USelection::SelectNoneEvent.Remove(SelectNoneEventHandle);
#endif
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_WorldDebuggerModule, ImGui_WorldDebugger)