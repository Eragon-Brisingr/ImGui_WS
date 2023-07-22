// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "ImGuiWorldDebuggerViewportPanel.generated.h"

class UImGuiWorldDebuggerDrawerBase;
struct FImGuiWorldViewportContext;

/**
 * 
 */
UCLASS(Transient, DefaultToInstanced, config = ImGuiWorldDebugger, PerObjectConfig)
class IMGUI_WS_API UImGuiWorldDebuggerViewportPanel : public UImGuiWorldDebuggerPanelBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerViewportPanel();
	
	UPROPERTY(Config)
	int32 ZoomFactor = 5.f;
	UPROPERTY(Config)
	FVector2D ViewLocation = { 0.f, 0.f };
	FVector2D CurrentViewLocation;
	UPROPERTY(Config)
	int32 NavMeshAgentIndex = 0;
	UPROPERTY(Config)
	int32 WorldPartitionGridIndex = 0;
	UPROPERTY(Config)
	int32 WorldPartitionGridLevelRange[2] = { 3, 4 };

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> SelectedActors;
	void SetSelectedEntities(const TSet<TWeakObjectPtr<AActor>>& NewSelectedActors);

	void Register(AImGuiWorldDebuggerBase* WorldDebugger) override;
	void Unregister(AImGuiWorldDebuggerBase* WorldDebugger) override;

	void Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds) override;
	AActor* GetFirstSelectActor() const;
	void FocusActor(AActor* Actor);
	void FocusActors(const TArray<AActor*>& Actors);
	
	FString FilterActorString;
	void SetFilterString(const FString& FilterString);
	uint32 FilterActorType = 0;
	struct EFilterActorType
	{
		static constexpr uint32 None = 0 << 1;
		static constexpr uint32 NameFilter = 1 << 1;
		static constexpr uint32 TypeFilter = 2 << 1;
		static const FString TypeFilter_FilterType;
	};
	TWeakObjectPtr<UClass> FilterActorClass;
	virtual void WhenFilterStringChanged(const FString& FilterString);
	virtual void DrawFilterTooltip();
	virtual void DrawFilterPopup(UWorld* World);
	virtual bool IsShowActorsByFilter(const AActor* Actor);
	virtual void FocusActorsByFilter(UWorld* World);

	int32 GetDrawableActorsCount() const { return DrawableActors.Num(); }
protected:
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>> DrawableActors;
	FDelegateHandle OnActorSpawnedHandler;
	FDelegateHandle OnLevelAdd_DelegateHandle;
	UFUNCTION()
	void WhenActorEndPlay(AActor* Actor, const EEndPlayReason::Type EndPlayReason);

	virtual void DrawDebugInfoMenu(bool& bIsConfigDirty) {}
	virtual void DrawDebugInfoUnderActors(const FImGuiWorldViewportContext& DebuggerContext) {}
	virtual void DrawDebugInfoUpperActors(const FImGuiWorldViewportContext& DebuggerContext) {}

private:
#if WITH_EDITOR
	friend class FImGui_EditorModule;
	static void WhenEditorSelectionChanged(const TArray<AActor*>& SelectedActors);

	DECLARE_DELEGATE_TwoParams(FEditorSelectActors, UWorld* World, const TSet<TWeakObjectPtr<AActor>>& SelectedActors);
	static FEditorSelectActors EditorSelectActors;
#endif
};

struct IMGUI_WS_API FImGuiWorldViewportContext
{
	FImGuiWorldViewportContext(UImGuiWorldDebuggerViewportPanel* Viewport, struct ImDrawList* DrawList, const FVector2D& RectMin, const FVector2D& MapSize, const FVector2D& ViewLocation, float Zoom, const FVector2D& MousePos, bool IsSelectDragging, const FVector2D& MouseClickedPos, float DeltaSeconds)
		: Viewport(Viewport)
		, DrawList(DrawList)
		, RectMin(RectMin)
		, MapSize(MapSize)
		, Zoom(Zoom)
		, WorldToMapTransform{ FScale2D{Zoom}, -ViewLocation * Zoom + MapSize / 2.f + RectMin }
		, MapToWorldTransform{ WorldToMapTransform.Inverse() }
		, ViewBounds{ MapToWorldTransform.TransformPoint(RectMin), MapToWorldTransform.TransformPoint(MapSize + RectMin) }
		, MousePos(MousePos)
		, RelativeMousePos{ MousePos.X - RectMin.X, MousePos.Y - RectMin.Y }
		, bIsSelectDragging(IsSelectDragging)
		, SelectDragBounds{ IsSelectDragging ? TArray<FVector2D>{ MapToWorldTransform.TransformPoint(MouseClickedPos), MapToWorldTransform.TransformPoint(MousePos) } : FBox2D() }
		, DeltaSeconds(DeltaSeconds)
		, bIsSelected(false)
		, bIsConfigDirty(false)
	{}

	const UImGuiWorldDebuggerViewportPanel* Viewport;
	struct ImDrawList* DrawList;
	const FVector2D RectMin;
	const FVector2D MapSize;
	const FVector2D ViewLocation;
	const float Zoom;
	const FTransform2D WorldToMapTransform;
	const FTransform2D MapToWorldTransform;
	const FBox2D ViewBounds;
	const FVector2D MousePos;
	const FVector2D RelativeMousePos;
	const uint8 bIsSelectDragging : 1;
	const FBox2D SelectDragBounds;
	const float DeltaSeconds;

	uint8 bIsSelected : 1;
	
	FVector2D GetScreenCenter() const
	{
		return RectMin + RectMin / 2.f;
	}
	FVector2D WorldToMapLocation(const FVector2D& WorldLocation) const
	{
		return WorldToMapTransform.TransformPoint(WorldLocation) - RectMin;
	}
	FVector2D WorldToScreenLocation(const FVector2D& WorldLocation) const
	{
		return WorldToMapTransform.TransformPoint(WorldLocation);
	}
	FVector2D MapToWorldLocation(const FVector2D& WorldLocation) const
	{
		return MapToWorldTransform.TransformPoint(WorldLocation + RectMin);
	}
	FVector2D ScreenToWorldLocation(const FVector2D& WorldLocation) const
	{
		return MapToWorldTransform.TransformPoint(WorldLocation);
	}
	FVector2D AddScreenOffset(const FVector2D& Position, const FVector2D& ScreenOffset) const
	{
		return Position + ScreenOffset / Zoom;
	}

	void DrawRect(const FBox2D& Box, const FLinearColor& Color, float Rounding = 0.f) const;
	void DrawRectFilled(const FBox2D& Box, const FLinearColor& Color, float Rounding = 0.f) const;
	void DrawLine(const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness = 1.f) const;
	void DrawCircleFilled(const FVector2D& Center, float Radius, const FLinearColor& Color, int NumSegments = 0) const;
	void DrawCircle(const FVector2D& Center, float Radius, const FLinearColor& Color, int NumSegments = 0, float Thickness = 1.0f) const;
	void DrawText(const FVector2D& Position, const FVector2D& ScreenOffset, const FString& Text, const FLinearColor& Color) const;
	void AddMessageText(const FString& Message, const FLinearColor& Color) const { Messages.Add(FMessage{ Message, Color }); }
	void MarkConfigDirty() const { bIsConfigDirty |= true; }
private:
	friend class UImGuiWorldDebuggerViewportPanel;

	using ImU32 = uint32;
	static ImU32 LinearColorToU32(const FLinearColor& Color);
	
	void AddMessageTextImmediately(const FString& Message, const FLinearColor& Color) const;
	mutable uint8 MessageRowIdx = 0;
	static constexpr float MessageRowHeight = 14.f;
	struct FMessage
	{
		FString Text;
		FLinearColor Color;
	};
	mutable TArray<FMessage> Messages;
	mutable uint8 bIsConfigDirty : 1;
};
