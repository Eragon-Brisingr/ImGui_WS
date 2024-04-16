#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FImGui_WorldDebuggerModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

private:
    FDelegateHandle OnPostWorldInitializationHandle;

    FDelegateHandle OnImGui_WS_EnableHandle;
    FDelegateHandle OnImGui_WS_DisableHandle;
    FDelegateHandle OnImGuiLocalPanelEnableHandle;
    FDelegateHandle OnImGuiLocalPanelDisableHandle;

#if WITH_EDITOR
    FDelegateHandle SelectObjectEventHandle;
    FDelegateHandle SelectNoneEventHandle;
#endif
};
