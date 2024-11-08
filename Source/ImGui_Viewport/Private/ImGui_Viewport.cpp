#include "ImGui_Viewport.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/HittestGrid.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

TAutoConsoleVariable<bool> CVar_ShowImGuiViewport
{
	TEXT("ImGui.Viewport.Show"),
	true,
	TEXT("Set ImGui viewport visibility"),
};

TAutoConsoleVariable<int32> CVar_ImGuiViewportZOrder
{
	TEXT("ImGui.Viewport.ZOrder"),
	10000,
	TEXT("Set ImGui viewport Z order"),
};

FImGui_ViewportModule::FViewportLookup FImGui_ViewportModule::ViewportLookup;
#if WITH_EDITOR
TWeakPtr<SImGuiViewportOverlay> FImGui_ViewportModule::EditorViewport;
#endif

void FImGui_ViewportModule::StartupModule()
{
	UGameViewportClient::OnViewportCreated().AddRaw(this, &FImGui_ViewportModule::WhenViewportCreated);
}

void FImGui_ViewportModule::ShutdownModule()
{
	UGameViewportClient::OnViewportCreated().RemoveAll(this);
}

class FImGui_ViewportModule::SImGuiGameViewportOverlay : public SImGuiViewportOverlay
{
	using Super = SImGuiViewportOverlay;
	
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
	Viewport->AddViewportWidgetContent(ImGuiPanel, CVar_ImGuiViewportZOrder.GetValueOnGameThread());
}

void SImGuiViewportOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const auto ScopedContext = ImGuiScopedContext();

	constexpr int32 FloatContextFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGui::SetNextWindowBgAlpha(0.4f);
	if (ImGui::Begin(FloatingContextName, nullptr, FloatContextFlags))
	{
		// Prepare FloatingContext panel
	}
	ImGui::End();
}

void SImGuiViewportOverlay::WhenImGuiTick(float DeltaSeconds)
{
	Super::WhenImGuiTick(DeltaSeconds);

	if (auto FloatingWindow = ImGui::FindWindowByName(FloatingContextName))
	{
		if (FloatingWindow->ContentSize.y > 0)
		{
			ImGui::SetWindowPos(FloatingWindow, { 50, 50 });
		}
		else
		{
			if (FloatingWindow->DrawList)
			{
				FloatingWindow->DrawList->_ResetForNewFrame();
			}
		}
	}

	// not block input when no panel clicked
	HoverWindowBounds.Reset();
	if (Context->IO.MouseDown[0])
	{
		HoverWindowBounds = FBox2f{ FVector2f::ZeroVector, FVector2f{ Context->IO.DisplaySize } };
	}
	else if (auto Window = Context->ActiveIdWindow ? Context->ActiveIdWindow : Context->HoveredWindow)
	{
		auto g = Context;
		ImVec2 padding_regular = g->Style.TouchExtraPadding;
		ImVec2 padding_for_resize = g->IO.ConfigWindowsResizeFromEdges ? g->WindowsHoverPadding : padding_regular;
		ImVec2 hit_padding = (Window->Flags & (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) ? padding_regular : padding_for_resize;
		HoverWindowBounds = FBox2f{ FVector2f{ Window->OuterRectClipped.Min - hit_padding }, FVector2f{ Window->OuterRectClipped.Max + hit_padding } };
	}
	else
	{
		auto& IO = Context->IO;
		const FVector2f Position = FSlateApplication::Get().GetCursorPos() - GetTickSpaceGeometry().GetAbsolutePosition();
		IO.AddMousePosEvent(Position.X, Position.Y);
	}
}

int32 SImGuiViewportOverlay::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FHittestGrid& PreviousGrid = Args.GetHittestGrid();
	PreviousGrid.RemoveWidget(this);
	if (HoverWindowBounds)
	{
		FSlateWidgetPersistentState& MutablePersistentState = const_cast<FSlateWidgetPersistentState&>(GetPersistentState());
		const auto Scale = MutablePersistentState.AllottedGeometry.Scale;
		const TGuardValue PersistentStateAllottedGeometryGuard{ MutablePersistentState.AllottedGeometry,
			MutablePersistentState.AllottedGeometry.MakeChild(HoverWindowBounds->GetSize() / Scale, FSlateLayoutTransform{ HoverWindowBounds->Min / Scale }) };
		PreviousGrid.AddWidget(this, 0, LayerId, GetProxyHandle().GetWidgetSortOrder());
		
		/* draw hit test area for debug
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			MutablePersistentState.AllottedGeometry.ToPaintGeometry(),
			FAppStyle::GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			FLinearColor(0, 0, 0, 0.4f)
		);*/
	}

	return Super::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

namespace ImGui::Viewport
{
	FOnImGuiTick OnImGuiTick;
	
	TSharedPtr<SImGuiViewportOverlay> GetViewport(const UWorld* World)
	{
		if (CVar_ShowImGuiViewport.GetValueOnGameThread() == false)
		{
			return nullptr;
		}

		TSharedPtr<SImGuiViewportOverlay> Viewport;
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
		return CVar_ShowImGuiViewport.GetValueOnGameThread() ? EVisibility::Visible : EVisibility::Collapsed;
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_ViewportModule, ImGui_Viewport)