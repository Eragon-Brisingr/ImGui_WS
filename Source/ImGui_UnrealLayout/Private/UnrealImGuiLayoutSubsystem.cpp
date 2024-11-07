// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLayoutSubsystem.h"

#include "UnrealImGuiPanelBuilder.h"
#include "Engine/World.h"

FUnrealImGuiLayoutManager* FUnrealImGuiLayoutManager::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}
	UWorld* World = WorldContextObject->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
#if WITH_EDITOR
	if (World->WorldType == EWorldType::Editor)
	{
		struct FEditorManager : FGCObject
		{
			FUnrealImGuiLayoutManager Context;
			void AddReferencedObjects(FReferenceCollector& Collector) override
			{
				Collector.AddPropertyReferencesWithStructARO(FUnrealImGuiLayoutManager::StaticStruct(), &Context);
			}
			FString GetReferencerName() const override
			{
				return TEXT("UnrealImGuiLayoutEditorManager");
			}
		};
		static FEditorManager Context;
		return &Context.Context;
	}
#endif
	if (UUnrealImGuiLayoutSubsystem* Subsystem = World->GetSubsystem<UUnrealImGuiLayoutSubsystem>())
	{
		return &Subsystem->Context;
	}
	return nullptr;
}

UUnrealImGuiPanelBase* FUnrealImGuiLayoutManager::FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const
{
	for (const auto& PanelBuilder : PanelBuilders)
	{
		if (auto Panel = PanelBuilder->FindPanel(PanelType))
		{
			return Panel;
		}
	}
	return nullptr;
}
