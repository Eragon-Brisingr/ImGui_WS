// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FImGuiModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

