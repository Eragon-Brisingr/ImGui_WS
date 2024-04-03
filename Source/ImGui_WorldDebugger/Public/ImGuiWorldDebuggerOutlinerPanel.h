// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "ImGuiWorldDebuggerOutlinerPanel.generated.h"

/**
 * 
*/
UCLASS()
class IMGUI_WORLDDEBUGGER_API UImGuiWorldDebuggerOutlinerPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerOutlinerPanel();

	void Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;
	void Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder) override;
	void Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds) override;
private:
	FDelegateHandle OnActorSpawnedHandler;
	FDelegateHandle OnActorDestroyedHandler;
	FDelegateHandle OnLevelAdd_DelegateHandle;
	
	enum EOutlinerTableColumnID : int32
	{
		OutlinerTableColumnID_Name,
		OutlinerTableColumnID_Type,

		OutlinerTableColumnID_Max
	};

	TArray<TWeakObjectPtr<AActor>> DisplayActors;
	void RefreshDisplayActors();

	// ID, SortDirection...
	TArray<int32> Orders;
	void RefreshSortOrder();

	FString FilterString;
	uint8 bInvokeRefreshSortOrder : 1;
	bool CanActorDisplay(const AActor* Actor) const;
};
