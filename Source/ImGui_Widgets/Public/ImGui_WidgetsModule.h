#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FImGui_WidgetsModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};
