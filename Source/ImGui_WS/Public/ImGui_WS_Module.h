// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Modules/ModuleManager.h"

class FImGui_WSModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

