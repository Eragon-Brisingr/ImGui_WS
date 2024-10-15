#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ILevelEditor;

class FImGui_ViewportEditorModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
private:
    void WhenLevelEditorCreated(TSharedPtr<ILevelEditor> LevelEditor);
};
