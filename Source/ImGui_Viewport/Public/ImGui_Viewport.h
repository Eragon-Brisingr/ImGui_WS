#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

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
    IMGUI_VIEWPORT_API static TWeakPtr<SImGuiPanel> EditorViewport;
#endif
};

namespace ImGui::Viewport
{
    IMGUI_VIEWPORT_API TSharedPtr<SImGuiPanel> GetViewport(const UWorld* World);
    IMGUI_VIEWPORT_API EVisibility GetVisibility();

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnImGuiTick, UWorld* /*World*/, float /*DeltaSeconds*/);
    IMGUI_VIEWPORT_API extern FOnImGuiTick OnImGuiTick;
}
