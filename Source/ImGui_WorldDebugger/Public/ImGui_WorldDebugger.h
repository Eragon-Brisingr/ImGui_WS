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
};
