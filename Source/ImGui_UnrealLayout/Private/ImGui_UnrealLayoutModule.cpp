#include "ImGui_UnrealLayoutModule.h"

#include "ImGuiEditorDefaultLayout.h"
#include "ImGuiUnrealContextManager.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FImGui_UnrealLayoutModule"

void FImGui_UnrealLayoutModule::StartupModule()
{
#if WITH_EDITOR
    FCoreDelegates::OnPostEngineInit.AddLambda([]
    {
        UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
        if (Manager == nullptr)
        {
            return;
        }
        if (Manager->EditorDrawer)
        {
            return;
        }
        Manager->EditorDrawer = [](float DeltaSeconds)
        {
            static UImGuiEditorDefaultDebugger* DefaultDebugger = []
            {
                UImGuiEditorDefaultDebugger* Debugger = NewObject<UImGuiEditorDefaultDebugger>();
                Debugger->AddToRoot();
                Debugger->Register();
                return Debugger;
            }();
            DefaultDebugger->Draw(DeltaSeconds);
        };
    });
#endif
}

void FImGui_UnrealLayoutModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_UnrealLayoutModule, ImGui_UnrealLayout)