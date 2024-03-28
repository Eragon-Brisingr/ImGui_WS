#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FImGui_SlateModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};
