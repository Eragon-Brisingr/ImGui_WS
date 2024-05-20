#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWorkspaceItem;

class FImGui_UnrealPanelsEditorModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

    void RefreshGroupMenu();
private:
    TSharedPtr<FWorkspaceItem> ImGuiPanelsGroup;

    FDelegateHandle OnTabManagerChangedHandle;
};
