#include "ImGui_UnrealLayoutModule.h"

#include "ImGuiDelegates.h"
#include "ImGuiEditorDefaultLayout.h"
#include "ImGuiUnrealContextManager.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"

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
        EditorContext->InvokeCreateDebugger = [this]
        {
            GetEditorDebugger();
        };
        EditorContext->EditorDrawer = [this](float DeltaSeconds)
        {
            GetEditorDebugger()->Draw(DeltaSeconds);
        };
    });
#endif
}

void FImGui_UnrealLayoutModule::ShutdownModule()
{

}

#if WITH_EDITOR
UImGuiEditorDefaultDebugger* FImGui_UnrealLayoutModule::GetEditorDebugger()
{
    if (UImGuiEditorDefaultDebugger* Debugger = DefaultEditorDebugger.Get())
    {
        return Debugger;
    }
    UImGuiEditorDefaultDebugger* Debugger = NewObject<UImGuiEditorDefaultDebugger>();
    Debugger->AddToRoot();
    Debugger->Register();
    DefaultEditorDebugger = Debugger;
    return Debugger;
}
#endif

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_UnrealLayoutModule, ImGui_UnrealLayout)