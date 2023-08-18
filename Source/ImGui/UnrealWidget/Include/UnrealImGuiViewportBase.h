// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiViewportBase.generated.h"

class UUnrealImGuiViewportExtentBase;
struct FUnrealImGuiViewportContext;

namespace UnrealImGui::Viewport
{
	namespace EFilterType
	{
		constexpr uint32 None = 0 << 1;
		constexpr uint32 NameFilter = 1 << 1;
		constexpr uint32 TypeFilter = 2 << 1;
		IMGUI_API extern const FString TypeFilter_FilterType;
	};
}

UCLASS(Abstract)
class IMGUI_API UUnrealImGuiViewportBase : public UUnrealImGuiPanelBase
{
	GENERATED_BODY()
public:
	UUnrealImGuiViewportBase();

	void Register(UObject* Owner) override;
	void Unregister(UObject* Owner) override;
	void Draw(UObject* Owner, float DeltaSeconds) override;

	virtual bool ShouldCreateExtent(UObject* Owner, const TSubclassOf<UUnrealImGuiViewportExtentBase>& ExtentType) const { return true; }
	virtual void DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty) {}
	virtual void DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext) {}

	UPROPERTY(Transient)
	TArray<UUnrealImGuiViewportExtentBase*> Extents;

	UUnrealImGuiViewportExtentBase* FindExtent(const TSubclassOf<UUnrealImGuiViewportExtentBase>& ExtentType) const;
	template<typename T>
	T* FindExtent() const { return (T*)FindExtent(T::StaticClass()); }

	struct FPassDrawer
	{
		UUnrealImGuiViewportExtentBase* Extent;
		int32 Priority = 0;
		TFunction<void(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)> Drawer;
	};
	TArray<FPassDrawer> PassDrawers;

	UPROPERTY(Config)
	int32 ZoomFactor = 5.f;
	float CurrentZoom;
	UPROPERTY(Config)
	FVector2D ViewLocation = { 0.f, 0.f };
	FVector2D CurrentViewLocation;

	FString FilterString;
	uint32 FilterType = 0;
	TWeakObjectPtr<UClass> FilterClass;
private:
	friend struct FUnrealImGuiViewportContext;
	static constexpr float MessageBoxWidth = 350.f;
};
