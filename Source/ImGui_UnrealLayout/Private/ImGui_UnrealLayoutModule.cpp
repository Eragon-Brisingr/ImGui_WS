#include "ImGui_UnrealLayoutModule.h"

#include "ImGuiEditorDefaultLayout.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiPanel.h"
#include "Engine/BlueprintCore.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_UnrealLayoutModule::StartupModule()
{
#if WITH_EDITOR
    FCoreDelegates::OnPostEngineInit.AddLambda([this]
    {
        UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
        if (Manager == nullptr)
        {
            return;
        }
        FImGuiUnrealEditorContext* EditorContext = Manager->GetImGuiEditorContext();
        if (EditorContext == nullptr)
        {
            return;
        }
        if (EditorContext->EditorDrawer)
        {
            return;
        }
    	UImGuiEditorDefaultDebugger* Debugger = NewObject<UImGuiEditorDefaultDebugger>();
		Debugger->AddToRoot();
		Debugger->Register();
		DefaultEditorDebugger = Debugger;

        EditorContext->EditorDrawer = [this](float DeltaSeconds)
        {
        	if (UImGuiEditorDefaultDebugger* Debugger = DefaultEditorDebugger.Get())
        	{
        		Debugger->Draw(DeltaSeconds);
        	}
        };
    });
	FCoreDelegates::OnPreExit.AddLambda([this]
	{
		if (UImGuiEditorDefaultDebugger* Debugger = DefaultEditorDebugger.Get())
		{
			Debugger->Unregister();
			Debugger->RemoveFromRoot();
		}
	});
	OnCDOConstructHandle = UUnrealImGuiPanelBase::OnCDOConstruct_Editor.AddLambda([this](const UUnrealImGuiPanelBase* CDO)
	{
		if (GEditor && Cast<UBlueprintCore>(CDO->GetClass()->ClassGeneratedBy) == nullptr)
		{
			static bool bInvokeUpdated = false;
			if (bInvokeUpdated == false)
			{
				GEditor->GetTimerManager()->SetTimerForNextTick([EditorDebugger = DefaultEditorDebugger]
				{
					if (UImGuiEditorDefaultDebugger* Debugger = EditorDebugger.Get())
					{
						Debugger->PanelBuilder->ReconstructPanels_Editor(Debugger);
					}
				});
			}
		}
	});
	OnPostEditChangePropertyHandle = UImGuiSettings::OnPostEditChangeProperty.AddLambda([this](UImGuiSettings*, FPropertyChangedEvent& Event)
	{
		if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UImGuiSettings, BlueprintPanels))
		{
			if (UImGuiEditorDefaultDebugger* Debugger = DefaultEditorDebugger.Get())
			{
				Debugger->PanelBuilder->ReconstructPanels_Editor(Debugger);
			}
		}
	});
#endif
}

void FImGui_UnrealLayoutModule::ShutdownModule()
{
#if WITH_EDITOR
	UUnrealImGuiPanelBase::OnCDOConstruct_Editor.Remove(OnCDOConstructHandle);
	UImGuiSettings::OnPostEditChangeProperty.Remove(OnPostEditChangePropertyHandle);
#endif
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_UnrealLayoutModule, ImGui_UnrealLayout)