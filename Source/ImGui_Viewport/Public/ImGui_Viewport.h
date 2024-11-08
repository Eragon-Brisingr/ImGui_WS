#pragma once

#include "CoreMinimal.h"
#include "SImGuiPanel.h"
#include "Modules/ModuleManager.h"

class SImGuiViewportOverlay;
struct EVisibility;
class SImGuiPanel;
class UGameInstance;

class FImGui_ViewportModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
    
    void WhenViewportCreated();

    class SImGuiGameViewportOverlay;

    using FViewportLookup = TMap<TWeakObjectPtr<UGameInstance>, TWeakPtr<SImGuiGameViewportOverlay>>;
    static FViewportLookup ViewportLookup;

#if WITH_EDITOR
    IMGUI_VIEWPORT_API static TWeakPtr<SImGuiViewportOverlay> EditorViewport;
#endif
};

class IMGUI_VIEWPORT_API SImGuiViewportOverlay : public SImGuiPanel
{
    using Super = SImGuiPanel;
protected:
    void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    void WhenImGuiTick(float DeltaSeconds) override;
	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	TOptional<FBox2f> HoverWindowBounds;
	double LastRenderSeconds = -1.f;
public:
    static constexpr auto FloatingContextName = "FloatingContext";
};

namespace ImGui::Viewport
{
    IMGUI_VIEWPORT_API TSharedPtr<SImGuiViewportOverlay> GetViewport(const UWorld* World);
    IMGUI_VIEWPORT_API EVisibility GetVisibility();

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnImGuiTick, UWorld* /*World*/, float /*DeltaSeconds*/);
    IMGUI_VIEWPORT_API extern FOnImGuiTick OnImGuiTick;
}
