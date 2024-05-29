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

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPrePanelDraw, UWorld*);
	IMGUI_UNREALPANELSEDITOR_API static FOnPrePanelDraw OnPrePanelDraw;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostPanelDraw, UWorld*);
	IMGUI_UNREALPANELSEDITOR_API static FOnPostPanelDraw OnPostPanelDraw;
private:
    TSharedPtr<FWorkspaceItem> ImGuiPanelsGroup;

    FDelegateHandle OnTabManagerChangedHandle;
    FDelegateHandle OnPostEditChangePropertyHandle;
};
