// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnrealImGuiLayoutSubsystem.generated.h"

class UUnrealImGuiPanelBase;
class UUnrealImGuiPanelBuilder;

USTRUCT()
struct IMGUI_UNREALLAYOUT_API FUnrealImGuiLayoutManager
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<UUnrealImGuiPanelBuilder>> PanelBuilders;
	
	static FUnrealImGuiLayoutManager* Get(const UObject* WorldContextObject);

	UUnrealImGuiPanelBase* FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const;
	template<typename T>
	T* FindPanel() const { return (T*)FindPanel(T::StaticClass()); }
};

UCLASS()
class IMGUI_UNREALLAYOUT_API UUnrealImGuiLayoutSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE; }

	UPROPERTY()
	FUnrealImGuiLayoutManager Context;
};

UCLASS()
class IMGUI_UNREALLAYOUT_API UUnrealImGuiPanelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = ImGui, meta = (WorldContext = WorldContextObject, DeterminesOutputType = PanelType))
	static UUnrealImGuiPanelBase* FindPanel(const UObject* WorldContextObject, TSubclassOf<UUnrealImGuiPanelBase> PanelType)
	{
		if (auto Manager = FUnrealImGuiLayoutManager::Get(WorldContextObject))
		{
			return Manager->FindPanel(PanelType);
		}
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = ImGui, meta = (DeterminesOutputType = PanelType))
	static UUnrealImGuiPanelBase* FindPanelByWorld(UWorld* World, TSubclassOf<UUnrealImGuiPanelBase> PanelType)
	{
		if (auto Manager = FUnrealImGuiLayoutManager::Get((UObject*)World))
		{
			return Manager->FindPanel(PanelType);
		}
		return nullptr;
	}
};
