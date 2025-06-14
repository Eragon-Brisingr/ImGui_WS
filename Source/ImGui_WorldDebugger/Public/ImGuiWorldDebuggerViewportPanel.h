// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiViewportBase.h"
#include "UnrealImGuiViewportExtent.h"
#include "Templates/SubclassOf.h"
#include "ImGuiWorldDebuggerViewportPanel.generated.h"

class UImGuiWorldDebuggerDrawerBase;
struct FImGuiWorldViewportContext;

UCLASS()
class IMGUI_WORLDDEBUGGER_API UImGuiWorldDebuggerViewportPanel : public UUnrealImGuiViewportBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerViewportPanel();

	bool ShouldCreatePanel(UObject* Owner) const override;
	bool ShouldCreateExtent(UObject* Owner, TSubclassOf<UUnrealImGuiViewportExtentBase> ExtentType) const override { return true; }

	void DrawCurrentViewFrustum(UObject* Owner, const FUnrealImGuiViewportContext& Context) override;
};

UCLASS()
class IMGUI_WORLDDEBUGGER_API UImGuiWorldDebuggerViewportActorExtent : public UUnrealImGuiViewportExtentBase
{
	GENERATED_BODY()
public:
	UImGuiWorldDebuggerViewportActorExtent();

	bool ShouldCreateExtent(UObject* Owner, UUnrealImGuiViewportBase* Viewport) const override;
	void Register(UObject* Owner, UUnrealImGuiViewportBase* Viewport) override;
	void Unregister(UObject* Owner, UUnrealImGuiViewportBase* Viewport) override;
	void DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty) override;
	void DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext) override;
	void DrawDetailsPanel(UObject* Owner, UImGuiWorldDebuggerDetailsPanel* DetailsPanel) override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Viewport)
	TSet<TObjectPtr<AActor>> SelectedActors;
	void SetSelectedEntities(const TSet<TObjectPtr<AActor>>& NewSelectedActors);
	void ResetSelection() override { SelectedActors.Reset(); }

	UFUNCTION(Blueprintable, Category = Viewport)
	AActor* GetFirstSelectActor() const;
	void FocusActor(AActor* Actor);
	void FocusActors(const TArray<AActor*>& Actors);

	void WhenFilterStringChanged(UUnrealImGuiViewportBase* Viewport, const FString& FilterString) override;
	void DrawFilterTooltip(UUnrealImGuiViewportBase* Viewport) override;
	void DrawFilterPopup(UUnrealImGuiViewportBase* Viewport) override;
	void FocusEntitiesByFilter(UUnrealImGuiViewportBase* Viewport) override;

	virtual bool IsShowActorsByFilter(const UUnrealImGuiViewportBase* Viewport, const AActor* Actor) const;

	int32 GetDrawableActorsCount() const { return DrawableActors.Num(); }
protected:
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>> DrawableActors;
	TMap<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>> ActorsToDraw;
	FDelegateHandle OnActorSpawnedHandle;
	FDelegateHandle OnActorDestroyedHandle;
	FDelegateHandle OnLevelAddHandle;

	bool CanAddToDraw(const UUnrealImGuiViewportBase* Viewport, const AActor* Actor, const UImGuiWorldDebuggerDrawerBase* Drawer) const;
private:
#if WITH_EDITOR
	friend class FImGui_WorldDebuggerModule;
	static void WhenEditorSelectionChanged(const TArray<AActor*>& SelectedActors);

	DECLARE_DELEGATE_TwoParams(FEditorSelectActors, UWorld* World, const TSet<TObjectPtr<AActor>>& SelectedActors);
	static FEditorSelectActors EditorSelectActors;
#endif
};
