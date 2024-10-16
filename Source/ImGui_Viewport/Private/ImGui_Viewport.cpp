#include "ImGui_Viewport.h"

#include "SImGuiPanel.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"

#define LOCTEXT_NAMESPACE "ImGui"

TAutoConsoleVariable<bool> CVar_ShowImGuiViewport
{
	TEXT("ImGui.Viewport.Show"),
	true,
	TEXT("Set ImGui viewport visibility"),
};

FImGui_ViewportModule::FViewportLookup FImGui_ViewportModule::ViewportLookup;
#if WITH_EDITOR
TWeakPtr<SImGuiPanel> FImGui_ViewportModule::EditorViewport;
#endif

void FImGui_ViewportModule::StartupModule()
{
	UGameViewportClient::OnViewportCreated().AddRaw(this, &FImGui_ViewportModule::WhenViewportCreated);
}

void FImGui_ViewportModule::ShutdownModule()
{
	UGameViewportClient::OnViewportCreated().RemoveAll(this);
}

class FImGui_ViewportModule::SImGuiGameViewportOverlay : public SImGuiPanel
{
	using Super = SImGuiPanel;
	
	TWeakObjectPtr<UGameInstance> WeakGameInstance;
public:
	void Construct(const FArguments& Args, UGameInstance* GameInstance)
	{
		check(GameInstance);
		Super::Construct(Args);
		WeakGameInstance = GameInstance;
		ViewportLookup.Add(GameInstance, SharedThis(this));
	}

	~SImGuiGameViewportOverlay() override
	{
		UGameInstance* GameInstance = WeakGameInstance.Get();
		ensure(GameInstance);
		ViewportLookup.Remove(GameInstance);
	}
};

void FImGui_ViewportModule::WhenViewportCreated()
{
	auto Viewport = GEngine->GameViewport;
	if (Viewport == nullptr)
	{
		return;
	}
	UGameInstance* GameInstance = Viewport->GetGameInstance();
	if (GameInstance == nullptr)
	{
		return;
	}
	auto ImGuiPanel = SNew(SImGuiGameViewportOverlay, GameInstance)
		.Visibility_Lambda([]
		{
			return ImGui::Viewport::GetVisibility();
		})
		.OnImGuiTick_Lambda([WeakGameInstance = TWeakObjectPtr<UGameInstance>(GameInstance)](float DeltaSeconds)
		{
			if (auto GameInstance = WeakGameInstance.Get())
			{
				if (auto World = GameInstance->GetWorld())
				{
					ImGui::Viewport::OnImGuiTick.Broadcast(World, DeltaSeconds);
				}
			}
		});
	Viewport->AddViewportWidgetContent(ImGuiPanel, 1);
}

namespace ImGui::Viewport
{
	FOnImGuiTick OnImGuiTick;
	
	TSharedPtr<SImGuiPanel> GetViewport(const UWorld* World)
	{
		if (CVar_ShowImGuiViewport.GetValueOnGameThread() == false)
		{
			return nullptr;
		}

		TSharedPtr<SImGuiPanel> Viewport;
#if WITH_EDITOR
		if (World->WorldType == EWorldType::Editor || World->GetNetMode() == NM_DedicatedServer)
		{
			Viewport = FImGui_ViewportModule::EditorViewport.Pin();
		}
		else
#endif
		{
			Viewport = FImGui_ViewportModule::ViewportLookup.FindRef(World->GetGameInstance()).Pin();
		}
		if (Viewport && Viewport->GetContext()->WithinFrameScope)
		{
			return Viewport;
		}
		return nullptr;
	}

	EVisibility GetVisibility()
	{
		return CVar_ShowImGuiViewport.GetValueOnGameThread() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_ViewportModule, ImGui_Viewport)