// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiReferenceViewer.generated.h"

UCLASS()
class IMGUI_UNREALPANELS_API UUnrealImGuiReferenceViewer : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiReferenceViewer();

	void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) override;

	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> Objects = { nullptr };

	bool bIgnoreSelfPackage = true;
	bool bIgnoreTransient = true;
	
	struct FReferencer
	{
		TWeakObjectPtr<UObject> Asset;
		TArray<FString> ReferencePaths;
	};
	TArray<FReferencer> References;

	struct FDependency
	{
		TWeakObjectPtr<UObject> Asset;
		TArray<FString> ReferencePaths;
	};
	TArray<FDependency> Dependencies;
	
	void Refresh();
};
