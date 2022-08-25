// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiPanel.h"
#include "ImGuiEditorDefaultLayout.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UImGuiEditorDefaultLayoutBase : public UUnrealImGuiLayoutBase
{
	GENERATED_BODY()
};

UCLASS()
class UImGuiEditorDefaultLayout : public UImGuiEditorDefaultLayoutBase
{
	GENERATED_BODY()
public:
	enum EDockId
	{
		Viewport,
		Outliner,
		Details,
		Utils,
	};

	UImGuiEditorDefaultLayout()
	{
		LayoutName = FText::FromString(TEXT("Default"));
	}

	void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) override;
};


UCLASS(Abstract)
class UImGuiEditorDefaultPanelBase : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
};

class FImGuiEditorDefaultLayoutBuilder : public FUnrealImGuiPanelBuilder, public FGCObject
{
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FString GetReferencerName() const override;

public:
	FImGuiEditorDefaultLayoutBuilder();

	void Draw(float DeltaSeconds);
};
