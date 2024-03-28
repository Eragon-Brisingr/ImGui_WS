// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanelBuilder.generated.h"

class UUnrealImGuiPanelBase;
class UUnrealImGuiLayoutBase;

USTRUCT()
struct IMGUI_API FUnrealImGuiPanelBuilder
{
	GENERATED_BODY()
public:
	FUnrealImGuiPanelBuilder();
	
	// Layout name
	FName DockSpaceName = NAME_None;

	void Register(UObject* Owner);
	void Unregister(UObject* Owner);

	void LoadDefaultLayout(UObject* Owner);
	void DrawPanels(UObject* Owner, float DeltaSeconds);

	void DrawLayoutStateMenu(UObject* Owner);
	void DrawPanelStateMenu(UObject* Owner);

	UUnrealImGuiPanelBase* FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const;
	template<typename T>
	T* FindPanel() const
	{
		static_assert(TIsDerivedFrom<T, UUnrealImGuiPanelBase>::Value);
		return (T*)FindPanel(T::StaticClass());
	}

	UPROPERTY(Transient)
	TArray<UUnrealImGuiLayoutBase*> Layouts;
	int32 ActiveLayoutIndex = 0;
	UUnrealImGuiLayoutBase* GetActiveLayout() const { return Layouts[ActiveLayoutIndex]; }
	UPROPERTY(Config)
	TSoftClassPtr<UUnrealImGuiLayoutBase> ActiveLayoutClass;

	UPROPERTY(Transient)
	TArray<UUnrealImGuiPanelBase*> Panels;
};
