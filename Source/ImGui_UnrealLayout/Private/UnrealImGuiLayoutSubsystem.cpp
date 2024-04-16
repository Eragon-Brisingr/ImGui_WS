// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLayoutSubsystem.h"

#include "Engine/World.h"

UUnrealImGuiLayoutSubsystem* UUnrealImGuiLayoutSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
	return World->GetSubsystem<UUnrealImGuiLayoutSubsystem>();
}
