#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UImGuiEditorDefaultDebugger;

class FImGui_UnrealLayoutModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

#if WITH_EDITOR
    TWeakObjectPtr<UImGuiEditorDefaultDebugger> DefaultEditorDebugger;

    UImGuiEditorDefaultDebugger* GetEditorDebugger();
#endif
};
