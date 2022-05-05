// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FImGui_EditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;
private:
	FDelegateHandle SelectObjectEventHandle;
	FDelegateHandle SelectNoneEventHandle;
};
