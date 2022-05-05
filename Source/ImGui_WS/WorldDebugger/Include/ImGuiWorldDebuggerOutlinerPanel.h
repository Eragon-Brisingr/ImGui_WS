// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "ImGuiWorldDebuggerOutlinerPanel.generated.h"

/**
 * 
*/
UCLASS()
class IMGUI_WS_API UImGuiWorldDebuggerOutlinerPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerOutlinerPanel();

	void Register(AImGuiWorldDebuggerBase* WorldDebugger) override;
	void Unregister(AImGuiWorldDebuggerBase* WorldDebugger) override;
	void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) override;
private:
	FDelegateHandle OnActorSpawnedHandler;
	FDelegateHandle OnLevelAdd_DelegateHandle;
	
	enum EOutlinerTableColumnID : int32
	{
		OutlinerTableColumnID_Name,
		OutlinerTableColumnID_Type,
	};

	TArray<TWeakObjectPtr<AActor>> DisplayActors;
	void RefreshDisplayActors();

	// ID, SortDirection...
	TArray<int32> Orders;
	void RefreshSortOrder();

	FString FilterString;
	bool CanActorDisplay(const AActor* Actor) const;

	UFUNCTION()
	void WhenActorDestroy(AActor* Actor);
};
