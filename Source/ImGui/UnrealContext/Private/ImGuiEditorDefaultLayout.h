// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanelBuilder.h"
#include "ImGuiEditorDefaultLayout.generated.h"

UCLASS(Abstract)
class UImGuiEditorDefaultLayoutBase : public UUnrealImGuiLayoutBase
{
	GENERATED_BODY()
public:
	bool ShouldCreateLayout(UObject* Owner) const override;
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

	void LoadDefaultLayout(UObject* Owner, const UUnrealImGuiPanelBuilder& LayoutBuilder) override;
};

UCLASS()
class UImGuiEditorDefaultDebugger : public UObject
{
	GENERATED_BODY()
public:
	UImGuiEditorDefaultDebugger();

	UWorld* GetWorld() const override;

	UPROPERTY()
	TObjectPtr<UUnrealImGuiPanelBuilder> PanelBuilder;

	void Register();
	void Draw(float DeltaSeconds);
};
