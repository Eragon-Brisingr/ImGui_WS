// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiUnrealLocalPanel.h"

#include "ImGuiDelegates.h"
#include "imgui_internal.h"
#include "ImGuiEx.h"
#include "ImGuiFontAtlas.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "SImGuiPanel.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiStat.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiStyles.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformFileManager.h"
#include "Slate/SObjectWidget.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SWindow.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void UImGuiLocalPanelManagerConfig::SaveSettings()
{
	SaveConfig(CPF_Config, nullptr, GConfig, false);
}

namespace ImGui_WS::LocalPanel
{
TAutoConsoleVariable<int32> CVarLocalPanelMode
{
	TEXT("ImGui.WS.LocalPanelMode"),
#if WITH_EDITOR
	(int32)EImGuiLocalPanelMode::DockWindow,
#elif PLATFORM_DESKTOP
	(int32)EImGuiLocalPanelMode::SingleWindow,
#else
	(int32)EImGuiLocalPanelMode::GameViewport,
#endif
	TEXT("0: open in game viewport\n")
	TEXT("1: open as single window (desktop platform only)\n")
	TEXT("2: open as dockable window (editor only)")
};
FAutoConsoleCommand OpenImGuiWindowCommand
{
	TEXT("ImGui.WS.OpenPanel"),
	TEXT("Open ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		OpenLocalWindow(World);
	})
};
FAutoConsoleCommand CloseImGuiWindowCommand
{
	TEXT("ImGui.WS.ClosePanel"),
	TEXT("Close ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		CloseLocalWindow(World);
	})
};

const auto& GetTextBlockStyles()
{
	struct FTextBlockStyles : FNoncopyable
	{
		FTextBlockStyle Large;
		FTextBlockStyle Tiny;

		FTextBlockStyles()
		{
			const FTextBlockStyle& Normal = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
			Large = Normal;
			Large.SetFontSize(14);
			Tiny = Normal;
			Tiny.SetFontSize(10);
		}
	};
	static FTextBlockStyles Styles;
	return Styles;
}

class SDragResizeContainer : public SCompoundWidget
{
public:
	class SDragResizeBox : public SCompoundWidget
	{
		using Super = SCompoundWidget;
	public:
		DECLARE_DELEGATE_OneParam(FOnDragged, const FVector2f& /*Pos*/);
		DECLARE_DELEGATE_OneParam(FOnResized, const FVector2f& /*Size*/);
		DECLARE_DELEGATE_OneParam(FOnMaximizeChanged, bool bMaximize);

		SLATE_BEGIN_ARGS(SDragResizeBox)
		{}
		SLATE_EVENT(FOnDragged, OnDragged);
		SLATE_EVENT(FOnResized, OnResized);
		SLATE_EVENT(FOnMaximizeChanged, OnMaximizeChanged);
		SLATE_NAMED_SLOT(FArguments, TitleContent)
		SLATE_NAMED_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		class SDragArea : public SBorder
		{
			using Super = SBorder;

			SDragResizeBox* Owner = nullptr;
			bool bBeingDragged = false;
			bool bIsDragging = false;
			FVector2f DragOffset;
		public:
			void Construct(const FArguments& InArgs, SDragResizeBox* InOwner)
			{
				Owner = InOwner;
				Super::Construct(InArgs);
			}
			bool SupportsKeyboardFocus() const override { return true; }
			FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (Owner->bIsMaximize)
				{
					return FReply::Handled();
				}
				if (MouseEvent.IsPointerEvent())
				{
					if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
					{
						return FReply::Handled();
					}
				}
				bBeingDragged = true;
				DragOffset = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
				return FReply::Handled().CaptureMouse(SharedThis(this));
			}
			FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (!bBeingDragged)
				{
					return FReply::Handled();
				}
				bBeingDragged = false;
				if (bIsDragging)
				{
					if (Owner->OnDragged.IsBound())
					{
						const TSharedPtr<SConstraintCanvas>& OwnerCanvas = Owner->Owner->Canvas;
						const FGeometry& CanvasGeometry = OwnerCanvas->GetTickSpaceGeometry();
						const FVector2f CanvasSize = CanvasGeometry.GetLocalSize();
						const FVector2f Position = CanvasGeometry.AbsoluteToLocal(MyGeometry.GetAbsolutePosition());
						Owner->OnDragged.Execute(Position / CanvasSize);
					}
					bIsDragging = false;
				}
				else
				{
					Super::OnMouseButtonUp(MyGeometry, MouseEvent);
				}
				return FReply::Handled().ReleaseMouseCapture();
			}
			FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (bBeingDragged)
				{
					bIsDragging = true;
					const TSharedPtr<SConstraintCanvas>& OwnerCanvas = Owner->Owner->Canvas;
					const FGeometry& CanvasGeometry = OwnerCanvas->GetTickSpaceGeometry();
					const FVector2f PanelSize = Owner->GetTickSpaceGeometry().GetLocalSize();
					FVector2f Position = CanvasGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - DragOffset;
					Position.X = FMath::Clamp(Position.X, 0.f, CanvasGeometry.GetLocalSize().X - PanelSize.X);
					Position.Y = FMath::Clamp(Position.Y, 0.f, CanvasGeometry.GetLocalSize().Y - PanelSize.Y);
					const FVector2f Offset = CanvasGeometry.Position + CanvasGeometry.GetLocalSize() - (Position + PanelSize);
					Owner->Slot->SetOffset(FMargin{ Position.X, Position.Y, Offset.X, Offset.Y });
				}
				return FReply::Handled();
			}
			FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				return Owner->OnDragAreaMouseButtonDoubleClick(MyGeometry, MouseEvent);
			}
		};

		class SResizeArea : public SBorder
		{
			using Super = SBorder;

			SDragResizeBox* Owner = nullptr;
			bool bBeingResized = false;
			bool bIsResizing = false;
			FVector2f ResizedOffset;
		public:
			void Construct(const FArguments& InArgs, SDragResizeBox* InOwner)
			{
				Owner = InOwner;
				Super::Construct(InArgs);
			}
			bool SupportsKeyboardFocus() const override { return true; }
			FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (MouseEvent.IsPointerEvent())
				{
					if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
					{
						return FReply::Handled();
					}
				}
				bBeingResized = true;
				ResizedOffset = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
				return FReply::Handled().CaptureMouse(SharedThis(this));
			}
			FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (!bBeingResized)
				{
					return FReply::Handled();
				}
				bBeingResized = false;
				if (bIsResizing && Owner->OnResized.IsBound())
				{
					const TSharedPtr<SConstraintCanvas>& OwnerCanvas = Owner->Owner->Canvas;
					const FGeometry& CanvasGeometry = OwnerCanvas->GetTickSpaceGeometry();
					const FVector2f CanvasSize = CanvasGeometry.GetLocalSize();
					const FVector2f Size = CanvasGeometry.GetAccumulatedRenderTransform().Inverse().TransformVector(Owner->GetTickSpaceGeometry().GetAbsoluteSize());
					Owner->OnResized.Execute(Size / CanvasSize);
				}
				bIsResizing = false;
				return FReply::Handled().ReleaseMouseCapture();
			}
			FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (bBeingResized)
				{
					bIsResizing = true;
					const TSharedPtr<SConstraintCanvas>& OwnerCanvas = Owner->Owner->Canvas;
					const FGeometry& CanvasGeometry = OwnerCanvas->GetTickSpaceGeometry();
					const FVector2f Position = Owner->GetTickSpaceGeometry().Position;
					const FVector2f MaxSize = CanvasGeometry.GetLocalSize() - Position;
					FVector2f PanelSize = (CanvasGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - ResizedOffset) - Position;
					PanelSize.X = FMath::Clamp(PanelSize.X, 200.f, MaxSize.X);
					PanelSize.Y = FMath::Clamp(PanelSize.Y, 100.f, MaxSize.Y);
					const FVector2f Offset = CanvasGeometry.Position + CanvasGeometry.GetLocalSize() - (Position + PanelSize);
					Owner->Slot->SetOffset(FMargin{ Position.X, Position.Y, Offset.X, Offset.Y });
				}
				return FReply::Handled();
			}
			FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override
			{
				return FCursorReply::Cursor(EMouseCursor::ResizeSouthEast);
			}
		};

		void Construct(const FArguments& Args)
		{
			OnDragged = Args._OnDragged;
			OnResized = Args._OnResized;
			OnMaximizeChanged = Args._OnMaximizeChanged;
			const TSharedRef<SWidget> Content = Args._Content.Widget;

			ChildSlot
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SDragArea, this)
					.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor_Lambda([this]
					{
						if (HasFocusedDescendants())
						{
							return FLinearColor{ 0.05f, 0.05f, 0.05f };
						}
						return FLinearColor{ 0.016f, 0.016f, 0.016f };
					})
					[
						Args._TitleContent.Widget
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SOverlay)
					.Visibility(AccessWidgetVisibilityAttribute(Content))
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						Content
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					[
						SNew(SResizeArea, this)
						.ToolTipText(LOCTEXT("ResizeAreaTooltip", "Resize"))
						.BorderImage(FAppStyle::GetBrush("NoBorder"))
						.Padding(0.f)
						.Visibility_Lambda([this]
						{
							return bIsMaximize ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible;
						})
						[
							SNew(SBox)
							.WidthOverride(32.f)
							.HeightOverride(32.f)
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("TreeArrow_Expanded"))
								.RenderTransformPivot(FVector2D{ 0.5f })
								.RenderTransform(FSlateRenderTransform{ FQuat2D(FMath::DegreesToRadians(-45)) })
							]
						]
					]
				]
			];
		}
		void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
		{
			Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

			if (RequirePositionScale)
			{
				const FVector4f& V = *RequirePositionScale;
				SetPositionAndSize(FVector2f{ V.X, V.Y }, FVector2f{ V.Z, V.W });

				RequirePositionScale.Reset();
			}
		}
		void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override
		{
			Super::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);

			if (NewWidgetPath.ContainsWidget(this))
			{
				Slot->SetZOrder(1);
			}
			else
			{
				Slot->SetZOrder(0);
			}
		}
		void SetMaximize(bool bMaximize)
		{
			if (bIsMaximize == bMaximize)
			{
				return;
			}
			bIsMaximize = bMaximize;
			if (bIsMaximize)
			{
				if (RequirePositionScale)
				{
					MaximizeBeforePositionSize = *RequirePositionScale;
					RequirePositionScale.Reset();
				}
				else
				{
					auto Offset = Slot->GetOffset();
					const FGeometry& CanvasGeometry = Owner->Canvas->GetTickSpaceGeometry();
					const FVector2f CanvasSize = CanvasGeometry.GetLocalSize();
					const FVector2f Min = FVector2f{ Offset.Left, Offset.Top } / CanvasSize;
					const FVector2f Size = (FVector2f::One() - FVector2f{ Offset.Right, Offset.Bottom } / CanvasSize) - Min;
					MaximizeBeforePositionSize = FVector4f{ Min.X, Min.Y, Size.X, Size.Y };
				}
				Slot->SetOffset(FMargin{ 0.f });
			}
			else
			{
				SetPositionAndSize(FVector2f{ MaximizeBeforePositionSize.X, MaximizeBeforePositionSize.Y }, FVector2f{ MaximizeBeforePositionSize.Z, MaximizeBeforePositionSize.W });
			}
			OnMaximizeChanged.ExecuteIfBound(bIsMaximize);
		}
		void SetPositionAndSize(const FVector2f& RelativePosition, const FVector2f& RelativeSize)
		{
			const FGeometry& CanvasGeometry = Owner->Canvas->GetTickSpaceGeometry();
			const FVector2f CanvasSize = CanvasGeometry.GetLocalSize();
			const FVector2f Position = CanvasSize * RelativePosition;
			const FVector2f Size = CanvasSize * RelativeSize;
			const FVector2f Offset = CanvasSize - (Position + Size);
			Slot->SetOffset(FMargin{ Position.X, Position.Y, Offset.X, Offset.Y });
		}
		virtual FReply OnDragAreaMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) { return FReply::Handled(); }

		SDragResizeContainer* Owner = nullptr;
		SConstraintCanvas::FSlot* Slot = nullptr;
		bool bIsMaximize = false;
		FVector4f MaximizeBeforePositionSize;
		TOptional<FVector4f> RequirePositionScale;
		FOnDragged OnDragged;
		FOnResized OnResized;
		FOnMaximizeChanged OnMaximizeChanged;
	};

	SLATE_BEGIN_ARGS(SDragResizeContainer)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args)
	{
		SetVisibility(EVisibility::SelfHitTestInvisible);
		ChildSlot
		[
			SAssignNew(Canvas, SConstraintCanvas)
		];
	}

	void AddChild(const TSharedRef<SDragResizeBox>& DragResizeBox, const FVector2f& RelativePosition, const FVector2f& RelativeSize, bool bMaximize)
	{
		SConstraintCanvas::FSlot* Slot;
		Canvas->AddSlot()
		.Expose(Slot)
		.Anchors(FAnchors{ 0.f, 0.f, 1.f, 1.f })
		.Alignment(FVector2D::ZeroVector)
		.Offset(FMargin{ 0.f, 0.f, 100000.f, 100000.f })
		[
			DragResizeBox
		];
		DragResizeBox->Owner = this;
		DragResizeBox->Slot = Slot;
		DragResizeBox->RequirePositionScale = FVector4f{ RelativePosition.X, RelativePosition.Y, RelativeSize.X, RelativeSize.Y };
		if (bMaximize)
		{
			DragResizeBox->SetMaximize(true);
		}
	}

	void RemoveChild(const TSharedRef<SDragResizeBox>& DragResizeBox)
	{
		Canvas->RemoveSlot(DragResizeBox);
	}
private:
	TSharedPtr<SConstraintCanvas> Canvas;
};

TWeakPtr<SWindow> WindowPtr;
struct FViewportPanel
{
	TWeakPtr<SWidget> Overlay;
	TSharedRef<int32> ContextIndex = MakeShared<int32>();
};
TMap<TWeakObjectPtr<UGameViewportClient>, FViewportPanel> ViewportPanelMap;

const FName WindowTabId = TEXT("ImGui_WS_Window");
bool IsLocalWindowOpened(const UWorld* World)
{
	if (WindowPtr.IsValid())
	{
		return true;
	}
	if (const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(World ? World->GetGameViewport() : nullptr))
	{
		return ViewportPanel->Overlay.IsValid();
	}
	return FGlobalTabmanager::Get()->FindExistingLiveTab(WindowTabId) != nullptr;
}

bool IsLocalWindowOpened(const UWorld* World, EImGuiLocalPanelMode PanelMode)
{
	switch (PanelMode)
	{
	case EImGuiLocalPanelMode::GameViewport:
		{
			const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(World ? World->GetGameViewport() : nullptr);
			return ViewportPanel ? ViewportPanel->Overlay.IsValid() : false;
		}
	case EImGuiLocalPanelMode::SingleWindow:
		{
			return WindowPtr.IsValid();
		}
	case EImGuiLocalPanelMode::DockWindow:
		{
			return FGlobalTabmanager::Get()->FindExistingLiveTab(WindowTabId) != nullptr;
		}
	}
	return false;
}

TSharedPtr<SImGuiPanel> CreatePanel(int32& ContextIndex)
{
	UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
	if (Manager == nullptr)
	{
		return nullptr;
	}

	const TSharedRef<SImGuiPanel> Panel = SNew(SImGuiPanel)
		.OnImGuiTick_Lambda([ManagerPtr = TWeakObjectPtr<UImGuiUnrealContextManager>(Manager), &ContextIndex](float DeltaSeconds)
		{
			if (UImGuiUnrealContextManager* Manager = ManagerPtr.Get())
			{
				DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiSinglePanel_Tick"), STAT_ImGuiSinglePanel_Tick, STATGROUP_ImGui);
				Manager->DrawViewport(ContextIndex, DeltaSeconds);
			}
		});

	ImGuiContext* OldContext = ImGui::GetCurrentContext();
	ON_SCOPE_EXIT
	{
		ImGui::SetCurrentContext(OldContext);
	};
	ImGui::SetCurrentContext(Panel->GetContext());

	ImGuiIO& IO = ImGui::GetIO();

	// Enable Docking
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	UnrealImGui::DefaultStyle();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
	// Make sure that directory is created.
	PlatformFile.CreateDirectory(*IniDirectory);
	static const UnrealImGui::FUTF8String IniFilePath = IniDirectory / TEXT("ImGui_LocalWindow.ini");
	IO.IniFilename = IniFilePath.GetData();
	return Panel;
}

static constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
class SImGuiLocalPanelOverlay : public SDragResizeContainer, public FGCObject
{
	using Super = SDragResizeContainer;
public:
	void Construct(const FArguments& Args, UGameViewportClient* GameViewport)
	{
		Config = NewObject<UImGuiLocalPanelManagerConfig>(GameViewport, TEXT("ImGuiLocalPanelOverlayConfig"));
		Super::Construct(Args);
		FImGuiDelegates::OnImGuiLocalPanelEnable.Broadcast();
	}
	~SImGuiLocalPanelOverlay() override
	{
		FImGuiDelegates::OnImGuiLocalPanelDisable.Broadcast();
	}
	TMap<TWeakObjectPtr<UUnrealImGuiPanelBase>, TSharedRef<SDragResizeBox>> PanelBoxMap;
	TSharedPtr<SDragResizeBox> ViewportPtr;
	TObjectPtr<UImGuiLocalPanelManagerConfig> Config;
	UnrealImGui::FUTF8String FilterString;

	void OpenPanel(UWorld* World, UUnrealImGuiPanelBuilder* Builder, UUnrealImGuiPanelBase* Panel)
	{
		ensure(PanelBoxMap.Contains(Panel) == false);
		const TSharedRef<SImGuiPanel> ImGuiPanel = SNew(SImGuiPanel);
		const FImGuiLocalPanelConfig PanelConfig = Config->PanelConfigMap.FindRef(Panel->GetClass());
		ImGuiPanel->GetContext()->IO.ConfigFlags = ImGuiConfigFlags_DockingEnable;
		UnrealImGui::DefaultStyle(&ImGuiPanel->GetContext()->Style);
		class SImGuiPanelBox : public SDragResizeBox
		{
			using Super = SDragResizeBox;
		public:
			TWeakObjectPtr<UUnrealImGuiPanelBase> PanelPtr;
			void Construct(const FArguments& Args, UUnrealImGuiPanelBase* Panel)
			{
				PanelPtr = Panel;
				Super::Construct(Args);
				Panel->LocalPanelOpened();
			}
			FReply OnDragAreaMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				SetMaximize(!bIsMaximize);
				return FReply::Handled();
			}
			~SImGuiPanelBox() override
			{
				if (UUnrealImGuiPanelBase* Panel = PanelPtr.Get())
				{
					Panel->LocalPanelClosed();
				}
			}
		};
		TSharedRef<SHorizontalBox> TitleContent = SNew(SHorizontalBox);
		const auto DragResizeBox = SNew(SImGuiPanelBox, Panel)
			.TitleContent()
			[
				TitleContent
			]
			.Content()
			[
				ImGuiPanel
			]
			.OnDragged_Lambda([this, Panel](const FVector2f& Pos)
			{
				Config->PanelConfigMap.FindOrAdd(Panel->GetClass()).Pos = Pos;
				Config->SaveSettings();
			})
			.OnResized_Lambda([this, Panel](const FVector2f& Size)
			{
				Config->PanelConfigMap.FindOrAdd(Panel->GetClass()).Size = Size;
				Config->SaveSettings();
			})
			.OnMaximizeChanged_Lambda([this, Panel](bool bMaximize)
			{
				Config->PanelConfigMap.FindOrAdd(Panel->GetClass()).bMaximize = bMaximize;
				Config->SaveSettings();
			});

		ImGuiPanel->OnImGuiTick.BindLambda([this, DragResizeBox = &DragResizeBox.Get(), World, Builder, PanelPtr = TWeakObjectPtr<UUnrealImGuiPanelBase>(Panel)](float DeltaSeconds)
		{
			UUnrealImGuiPanelBase* Panel = PanelPtr.Get();
			if (Panel == nullptr)
			{
				for (auto It = PanelBoxMap.CreateIterator(); It; ++It)
				{
					if (&It.Value().Get() == DragResizeBox)
					{
						RemoveChild(It.Value());
						It.RemoveCurrent();
						break;
					}
				}
				return;
			}
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiLocalPanel_Tick"), STAT_ImGuiLocalPanel_Tick, STATGROUP_ImGui);
			ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
			ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
			if (ImGui::FWindow Window{ TCHAR_TO_UTF8(*Panel->Title.ToString()), nullptr, Panel->ImGuiWindowFlags | SinglePanelFlags })
			{
				UImGuiUnrealContextManager::OnPreDraw.Broadcast(World);
				Panel->Draw(World, Builder, DeltaSeconds);
				UImGuiUnrealContextManager::OnPostDraw.Broadcast(World);
			}
		});

		TitleContent->AddSlot()
		.Padding(20.f, 2.f, 2.f, 2.f)
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&GetTextBlockStyles().Large)
			.Text(FText::FromName(Panel->Title))
		];
		TitleContent->AddSlot()
		.AutoWidth()
		.Padding(2.f)
		[
			SNew(SButton)
			.ToolTipText_Lambda([DragResizeBox = &DragResizeBox.Get()]
			{
				return DragResizeBox->bIsMaximize ? LOCTEXT("MinimizeBoxTooltip", "Minimize") : LOCTEXT("MaximizeBoxTooltip", "Maximize");
			})
			.OnClicked_Lambda([DragResizeBox = &DragResizeBox.Get()]
			{
				DragResizeBox->SetMaximize(!DragResizeBox->bIsMaximize);
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image_Lambda([DragResizeBox = &DragResizeBox.Get()]
				{
					return DragResizeBox->bIsMaximize ? FAppStyle::GetBrush("Icons.Minus") : FAppStyle::GetBrush("Icons.Plus");
				})
			]
		];
		TitleContent->AddSlot()
		.AutoWidth()
		.Padding(2.f)
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("CloseBoxTooltip", "Close"))
			.OnClicked_Lambda([this, Panel]
			{
				ClosePanel(Panel);
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.X"))
			]
		];
		
		AddChild(DragResizeBox, PanelConfig.Pos, PanelConfig.Size, PanelConfig.bMaximize);
		PanelBoxMap.Add(Panel, DragResizeBox);

		FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), ImGuiPanel, EFocusCause::SetDirectly);
	}
	void ClosePanel(UUnrealImGuiPanelBase* Panel)
	{
		if (PanelBoxMap.Contains(Panel) == false)
		{
			return;
		}
		RemoveChild(PanelBoxMap.FindAndRemoveChecked(Panel));
	}

	void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Config);
	}
	FString GetReferencerName() const override
	{
		return TEXT("SImGuiLocalPanelOverlay");
	}
};

class SPanelManager : public SDragResizeContainer::SDragResizeBox
{
	using Super = SDragResizeBox;
public:
	SLATE_BEGIN_ARGS(SPanelManager)
	{}
	SLATE_END_ARGS()
	TSharedPtr<SImGuiLocalPanelOverlay> Overlay;
	TWeakObjectPtr<UGameViewportClient> GameViewportPtr;
	TWeakObjectPtr<UImGuiLocalPanelManagerWidget> LocalPanelManagerWidget = nullptr;
	void Construct(const FArguments& Args, const TSharedRef<SImGuiLocalPanelOverlay>& InOverlay, UGameViewportClient* GameViewport, const TSharedRef<int32>& ContextIndex)
	{
		Overlay = InOverlay;
		GameViewportPtr = GameViewport;
		auto ImGuiSettings = GetDefault<UImGuiSettings>();
		TSharedPtr<SWidget> FloatButton;
		if (UClass* Class = ImGuiSettings->LocalPanelManagerWidget.LoadSynchronous())
		{
			if (ensure(Class->IsChildOf(UImGuiLocalPanelManagerWidget::StaticClass())))
			{
				UImGuiLocalPanelManagerWidget* PanelManagerWidget = CreateWidget<UImGuiLocalPanelManagerWidget>(GameViewport->GetGameInstance(), Class);
				if (ensure(PanelManagerWidget))
				{
					LocalPanelManagerWidget = PanelManagerWidget;
					FloatButton = PanelManagerWidget->TakeWidget();
				}
			}
		}

		auto ImGuiContent = SNew(SImGuiPanel)
			.Visibility_Lambda([this]
			{
				if (Overlay->Config == nullptr)
				{
					return EVisibility::Collapsed;
				}
				return Overlay->Config->bManagerCollapsed ? EVisibility::Collapsed : EVisibility::Visible;
			})
			.OnImGuiTick_Lambda([this, ContextIndex](float DeltaSeconds)
			{
				DrawContent(ContextIndex);
			});

		UnrealImGui::DefaultStyle(&ImGuiContent->GetContext()->Style);
		
		Super::FArguments SuperArgs;
		SuperArgs.TitleContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(20.f, 2.f, 2.f, 2.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&GetTextBlockStyles().Large)
				.Text(LocalPanelManagerWidget.IsValid() ? FText::FromString(LocalPanelManagerWidget->Title) : LOCTEXT("ImGuiPanelManagerTitle", "ImGui Panel Manager"))
			]
			+ SHorizontalBox::Slot()
			.Padding(10.f, 2.f, 2.f, 4.f)
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.TextStyle(&GetTextBlockStyles().Tiny)
				.Text_Lambda([]
				{
					return FText::FromString(FDateTime::Now().ToString(TEXT("%m.%d-%H.%M.%S")));
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(2.f)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("MinimizeManagerTooltip", "Minimize"))
				.OnClicked_Lambda([this]
				{
					Minimize();
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Minus"))
				]
			]
		]
		.Content()
		[
			ImGuiContent
		]
		.OnDragged_Lambda([this](const FVector2f& Pos)
		{
			CachedPos = Pos;
			Overlay->Config->ManagerConfig.Pos = Pos;
			Overlay->Config->SaveSettings();
		})
		.OnResized_Lambda([this](const FVector2f& Size)
		{
			CachedSize = Size;
			Overlay->Config->ManagerConfig.Size = Size;
			Overlay->Config->SaveSettings();
		});

		Super::Construct(SuperArgs);
		const TSharedRef<SWidget> PanelWidget = ChildSlot.GetWidget();

		if (FloatButton == nullptr)
		{
			FloatButton = SNew(SBox)
				.WidthOverride(60.f)
				.HeightOverride(60.f)
				.ToolTipText(LOCTEXT("ImGuiPanelManagerMinimizeTooltip", "Open ImGui Panel Manager"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(2.f)
					.FillHeight(1.f)
					[
						SNew(STextBlock)
						.TextStyle(&GetTextBlockStyles().Large)
						.Text(LOCTEXT("ImGuiPanelManagerMinimizeTitle", "ImGui"))
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(4.f)
					[
						SNew(STextBlock)
						.TextStyle(&GetTextBlockStyles().Tiny)
						.Text_Lambda([]
						{
							return FText::FromString(FDateTime::Now().ToString(TEXT("%H.%M.%S")));
						})
					]
				];
		}

		ChildSlot
		[
			SAssignNew(WidgetSwitcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()
			[
				PanelWidget
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SDragArea, this)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor_Lambda([this]
				{
					if (HasFocusedDescendants())
					{
						return FLinearColor{ 0.05f, 0.05f, 0.05f };
					}
					return FLinearColor{ 0.016f, 0.016f, 0.016f };
				})
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnMouseButtonUp_Lambda([this](const FGeometry&, const FPointerEvent&)
				{
					ExpandPanel();
					return FReply::Handled();
				})
				[
					FloatButton.ToSharedRef()
				]
			]
		];
	}

	void DrawContent(const TSharedRef<int32>& ContextIndex)
	{
		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiLocalPanelManager_Tick"), STAT_LocalPanelManager_Tick, STATGROUP_ImGui);

		UWorld* World = GameViewportPtr->GetWorld();
		if (World == nullptr)
		{
			return;
		}
		
		FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World);
		if (LayoutManager == nullptr)
		{
			return;
		}
		ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
		ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
		if (ImGui::FWindow Window{ "ImGui Panel Manager", nullptr, SinglePanelFlags | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar })
		{
			static auto DrawPanelCheckBox = [](UWorld* InWorld, SImGuiLocalPanelOverlay* InOverlay, UUnrealImGuiPanelBuilder* Builder, UUnrealImGuiPanelBase* Panel)
			{
				bool IsOpen = InOverlay->PanelBoxMap.Contains(Panel);
				if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Panel->Title.ToString(), *Panel->GetClass()->GetName())), &IsOpen))
				{
					if (IsOpen)
					{
						GetMutableDefault<UImGuiPerUserSettings>()->RecordRecentlyOpenPanel(Panel->GetClass());
						InOverlay->OpenPanel(InWorld, Builder, Panel);
					}
					else
					{
						InOverlay->ClosePanel(Panel);
					}
				}
				if (ImGui::FItemTooltip Tooltip{})
				{
					FString CategoryPath;
					for (int32 Idx = 0; Idx < Panel->Categories.Num() - 1; ++Idx)
					{
						CategoryPath += Panel->Categories[Idx].ToString() + TEXT(",");
					}
					if (Panel->Categories.Num() > 0)
					{
						CategoryPath += Panel->Categories.Last().ToString();
					}
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*FText::Format(LOCTEXT("PanelCheckBoxTooltip", "Class: {0}\nCategoryPath:{1}"), FText::FromString(Panel->GetClass()->GetPathName()), FText::FromString(CategoryPath)).ToString()));
				}
			};
			if (ImGui::FMenuBar MenuBar{})
			{
				if (ImGui::FMenu Menu{ "Viewport" })
				{
					if (ImGui::FMenu SettingsMenu{ "Settings" })
					{
						UnrealImGui::ShowStyleSelector();
						UnrealImGui::ShowGlobalDPISettings();
					}
					bool IsOpen = Overlay->ViewportPtr.IsValid();
					if (ImGui::Checkbox("ImGui Full Viewport", &IsOpen))
					{
						if (IsOpen)
						{
							OpenFullViewportPanel(ContextIndex);
						}
						else
						{
							if (Overlay->ViewportPtr)
							{
								Overlay->RemoveChild(Overlay->ViewportPtr.ToSharedRef());
								Overlay->ViewportPtr.Reset();
							}
						}
					}
					ImGui::Separator();
				}
				for (UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
				{
					struct FLocal
					{
						UWorld* World;
						SImGuiLocalPanelOverlay* Overlay;
						using FCategoryPanels = UUnrealImGuiPanelBuilder::FCategoryPanels;
						void DrawCategory(UUnrealImGuiPanelBuilder* Builder, const FCategoryPanels& Panels) const
						{
							if (ImGui::FMenu Menu{ TCHAR_TO_UTF8(*Panels.Category.ToString()) })
							{
								for (const auto& [_, Child] : Panels.Children)
								{
									DrawCategory(Builder, *Child);
								}
								DrawPanelState(Builder, Panels);
							}
						}
						void DrawPanelState(UUnrealImGuiPanelBuilder* Builder, const FCategoryPanels& Panels) const
						{
							for (UUnrealImGuiPanelBase* Panel : Panels.Panels)
							{
								DrawPanelCheckBox(World, Overlay, Builder, Panel);
							}
						}
					};
					const FLocal Local{ World, Overlay.Get() };
					for (const auto& [_, Child] : Builder->CategoryPanels.Children)
					{
						Local.DrawCategory(Builder, *Child);
					}
					Local.DrawPanelState(Builder, Builder->CategoryPanels);
				}

				if (auto PanelManagerWidget = LocalPanelManagerWidget.Get())
				{
					PanelManagerWidget->DrawMenu();
				}
			}

			struct FTreeNodeSeparator : ImGui::FTreeNodeEx
			{
				FTreeNodeSeparator(const char* label, ImGuiTreeNodeFlags flags = 0)
					: FTreeNodeEx(label, flags)
				{
					ImGui::SameLine(); ImGui::Separator();
				}
			};

			const float ItemWidth = ImGui::GetFontSize() * 10.f;
			const int32 ColumnsNum = FMath::Max(FMath::FloorToInt32(ImGui::GetWindowSize().x / ItemWidth), 1);

			auto DrawPanels = [&](const TArray<TSoftClassPtr<UObject>>& Panels)
			{
				int32 ItemCounter = 0;
				for (int32 Idx = 0; Idx < Panels.Num(); ++Idx)
				{
					const auto& PanelClass = Panels[Idx];
					UUnrealImGuiPanelBuilder* Builder = nullptr;
					UUnrealImGuiPanelBase* Panel = nullptr;
					for (UUnrealImGuiPanelBuilder* PanelBuilder : LayoutManager->PanelBuilders)
					{
						Panel = PanelBuilder->GetPanelsMap().FindRef(PanelClass.Get());
						if (Panel != nullptr)
						{
							Builder = PanelBuilder;
							break;
						}
					}
					if (Panel)
					{
						const int32 ColumnIdx = ItemCounter % ColumnsNum;
						if (ColumnIdx == 0)
						{
							ImGui::TableNextRow();
						}
						if (ImGui::TableSetColumnIndex(ColumnIdx))
						{
							DrawPanelCheckBox(World, Overlay.Get(), Builder, Panel);
							ItemCounter += 1;
						}
					}
				}
			};

			if (FTreeNodeSeparator TreeNodeSeparator{ "Favorite Panels", ImGuiTreeNodeFlags_DefaultOpen })
			{
				if (ImGui::FTable Table{ "FavoritePanels", ColumnsNum })
				{
					DrawPanels(GetDefault<UImGuiSettings>()->FavoritePanels);
				}
			}

			if (FTreeNodeSeparator TreeNodeSeparator{ "Recently Panels", ImGuiTreeNodeFlags_DefaultOpen })
			{
				if (ImGui::FTable Table{ "RecentlyPanels", ColumnsNum })
				{
					DrawPanels(GetDefault<UImGuiPerUserSettings>()->RecentlyOpenPanels);
				}
			}

			if (FTreeNodeSeparator TreeNodeSeparator{ "Search Panel" })
			{
				UnrealImGui::FUTF8String& FilterString = Overlay->FilterString;
				ImGui::InputTextWithHint("##SearchPanel", "Search Panel", FilterString);
				if (ImGui::FListBox ListBox{ "##FilteredPanel", ImVec2{ 0.f, ImGui::GetFontSize() * 10 } })
				{
					if (FilterString.Len() == 0)
					{
						int32 PanelNum = 0;
						for (const UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
						{
							PanelNum += Builder->Panels.Num();
						}
						ImGuiListClipper Clipper;
						Clipper.Begin(PanelNum);
						while (Clipper.Step())
						{
							for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; Idx++)
							{
								ImGui::PushID(Idx);
								UUnrealImGuiPanelBuilder* Builder = nullptr;
								int32 PanelIdx = Idx;
								for (UUnrealImGuiPanelBuilder* PanelBuilder : LayoutManager->PanelBuilders)
								{
									if (PanelIdx < PanelBuilder->Panels.Num())
									{
										Builder = PanelBuilder;
										break;
									}
									PanelIdx -= PanelBuilder->Panels.Num();
								}
								DrawPanelCheckBox(World, Overlay.Get(), Builder, Builder->Panels[PanelIdx]);
								ImGui::PopID();
							}
						}
					}
					else
					{
						TArray<UUnrealImGuiPanelBase*> FilteredPanels;
						const FString TestString = FilterString.ToString();
						for (const UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
						{
							for (UUnrealImGuiPanelBase* Panel : Builder->Panels)
							{
								if (Panel->Title.ToString().Contains(TestString) || Panel->GetClass()->GetName().Contains(TestString))
								{
									FilteredPanels.Add(Panel);
								}
							}
						}
						ImGuiListClipper Clipper;
						Clipper.Begin(FilteredPanels.Num());
						while (Clipper.Step())
						{
							for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; Idx++)
							{
								ImGui::PushID(Idx);
								UUnrealImGuiPanelBuilder* Builder = Cast<UUnrealImGuiPanelBuilder>(FilteredPanels[Idx]->GetOuter());
								if (ensure(Builder))
								{
									DrawPanelCheckBox(World, Overlay.Get(), Builder, FilteredPanels[Idx]);
								}
								ImGui::PopID();
							}
						}
					}
				}
			}

			if (FTreeNodeSeparator TreeNodeSeparator{ "Category Panel" })
			{
				struct FLocal
				{
					const int32 ColumnsNum;
					UWorld* World;
					SImGuiLocalPanelOverlay* Overlay;
					using FCategoryPanels = UUnrealImGuiPanelBuilder::FCategoryPanels;
					void DrawCategory(ImGuiTreeNodeFlags_ ImGuiTreeNodeFlags, UUnrealImGuiPanelBuilder* Builder, const FCategoryPanels& Panels) const
					{
						for (const auto& [_, Child] : Panels.Children)
						{
							if (ImGui::FTreeNodeEx TreeNodeEx{ TCHAR_TO_UTF8(*Child->Category.ToString()), ImGuiTreeNodeFlags })
							{
								DrawCategory(ImGuiTreeNodeFlags_None, Builder, *Child);
							}
						}
						if (ImGui::FTable Table{ "PanelTable", ColumnsNum })
						{
							for (int32 Idx = 0; Idx < Panels.Panels.Num(); ++Idx)
							{
								UUnrealImGuiPanelBase* Panel = Panels.Panels[Idx];
								const int32 ColumnIdx = Idx % ColumnsNum;
								if (ColumnIdx == 0)
								{
									ImGui::TableNextRow();
								}
								if (ImGui::TableSetColumnIndex(ColumnIdx))
								{
									DrawPanelCheckBox(World, Overlay, Builder, Panel);
								}
							}
						}
					}
				};
				constexpr auto TreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
				if (LayoutManager->PanelBuilders.Num() == 1)
				{
					auto Builder = LayoutManager->PanelBuilders[0];
					FLocal{ ColumnsNum, World, Overlay.Get() }.DrawCategory(TreeNodeFlags, Builder, Builder->CategoryPanels);
				}
				else
				{
					for (UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
					{
						for (const auto& [_, Child] : Builder->CategoryPanels.Children)
						{
							if (ImGui::FTreeNodeEx TreeNodeEx{ TCHAR_TO_UTF8(*Builder->GetOuter()->GetClass()->GetName()), TreeNodeFlags })
							{
								FLocal{ ColumnsNum, World, Overlay.Get() }.DrawCategory(TreeNodeFlags, Builder, Builder->CategoryPanels);
							}
						}
					}
				}
			}

			if (auto PanelManagerWidget = LocalPanelManagerWidget.Get())
			{
				PanelManagerWidget->DrawContent();
			}
		}
	}
	void OpenFullViewportPanel(const TSharedRef<int32>& ContextIndex)
	{
		ensure(Overlay->ViewportPtr.IsValid() == false);
		if (const TSharedPtr<SImGuiPanel> Panel = CreatePanel(*ContextIndex))
		{
			class SImGuiFullViewportPanelBox : public SDragResizeBox
			{
				using Super = SDragResizeBox;
			public:
				FReply OnDragAreaMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
				{
					SetMaximize(!bIsMaximize);
					return FReply::Handled();
				}
			};
			TSharedRef<SHorizontalBox> TitleContent = SNew(SHorizontalBox);
			const auto DragResizeBox = SNew(SImGuiFullViewportPanelBox)
				.TitleContent()
				[
					TitleContent
				]
				.Content()
				[
					Panel.ToSharedRef()
				]
				.OnDragged_Lambda([this](const FVector2f& Pos)
				{
					Overlay->Config->ViewportConfig.Pos = Pos;
					Overlay->Config->SaveSettings();
				})
				.OnResized_Lambda([this](const FVector2f& Size)
				{
					Overlay->Config->ViewportConfig.Size = Size;
					Overlay->Config->SaveSettings();
				})
				.OnMaximizeChanged_Lambda([this](bool bMaximize)
				{
					Overlay->Config->ViewportConfig.bMaximize = bMaximize;	
				});

			TitleContent->AddSlot()
			.FillWidth(1.f)
			.Padding(20.f, 2.f, 2.f, 2.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&GetTextBlockStyles().Large)
				.Text(LOCTEXT("ImGuiFullViewportTitle", "ImGui Full Viewport"))
			];
			TitleContent->AddSlot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.ToolTipText_Lambda([DragResizeBox = &DragResizeBox.Get()]
				{
					return DragResizeBox->bIsMaximize ? LOCTEXT("MinimizeBoxTooltip", "Minimize") : LOCTEXT("MaximizeBoxTooltip", "Maximize");
				})
				.OnClicked_Lambda([DragResizeBox = &DragResizeBox.Get()]
				{
					DragResizeBox->SetMaximize(!DragResizeBox->bIsMaximize);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image_Lambda([DragResizeBox = &DragResizeBox.Get()]
					{
						return DragResizeBox->bIsMaximize ? FAppStyle::GetBrush("Icons.Minus") : FAppStyle::GetBrush("Icons.Plus");
					})
				]
			];
			TitleContent->AddSlot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("CloseBoxTooltip", "Close"))
				.OnClicked_Lambda([this]
				{
					if (Overlay->ViewportPtr.IsValid())
					{
						Overlay->RemoveChild(Overlay->ViewportPtr.ToSharedRef());
						Overlay->ViewportPtr.Reset();
					}
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.X"))
				]
			];
			
			Overlay->ViewportPtr = DragResizeBox;
			Overlay->AddChild(DragResizeBox, Overlay->Config->ViewportConfig.Pos, Overlay->Config->ViewportConfig.Size, Overlay->Config->ViewportConfig.bMaximize);
			
			FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), Panel, EFocusCause::SetDirectly);
		}
	}
	FReply OnDragAreaMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		Minimize();
		return FReply::Handled();
	}
	void ExpandPanel()
	{
		if (Overlay->Config->bManagerCollapsed == false)
		{
			return;
		}
		Overlay->Config->bManagerCollapsed = false;
		Overlay->Config->SaveSettings();
		Slot->SetAnchors(FAnchors{ 0.f, 0.f, 1.f, 1.f });
		Slot->SetAutoSize(false);
		WidgetSwitcher->SetActiveWidgetIndex(0);
		RequirePositionScale = FVector4f{ CachedPos.X, CachedPos.Y, CachedSize.X, CachedSize.Y };
	}
	void Minimize()
	{
		if (Overlay->Config->bManagerCollapsed)
		{
			return;
		}
		Overlay->Config->bManagerCollapsed = true;
		Overlay->Config->SaveSettings();
		ForceMinimize();
	}
	void ForceMinimize()
	{
		Slot->SetAnchors(FAnchors{ 0.f, 0.f, 0.f, 0.f });
		Slot->SetAutoSize(true);
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}
	FVector2f CachedSize;
	FVector2f CachedPos;
private:
	TSharedPtr<SWidgetSwitcher> WidgetSwitcher;
};

void OpenLocalWindow(UWorld* World)
{
	const EImGuiLocalPanelMode PanelMode{ static_cast<uint8>(CVarLocalPanelMode.GetValueOnAnyThread()) };
	OpenLocalWindow(World, PanelMode);
}

void OpenLocalWindow(UWorld* World, EImGuiLocalPanelMode PanelMode)
{
	if (IsLocalWindowOpened(World, PanelMode))
	{
		return;
	}

	class SImGuiPanelLifetime : public SCompoundWidget
	{
		SLATE_BEGIN_ARGS(SImGuiPanelLifetime)
		{}
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& Args)
		{
			FImGuiDelegates::OnImGuiLocalPanelEnable.Broadcast();

			ChildSlot
			[
				Args._Content.Widget
			];
		}
		~SImGuiPanelLifetime() override
		{
			FImGuiDelegates::OnImGuiLocalPanelDisable.Broadcast();
		}
	};

	if (PanelMode == EImGuiLocalPanelMode::DockWindow && GIsEditor)
	{
		static FName Id = []
		{
			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WindowTabId, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
			{
				static int32 ContextIndex = 0;
				TSharedPtr<SWidget> Panel = CreatePanel(ContextIndex);
				if (Panel == nullptr)
				{
					Panel = SNullWidget::NullWidget;
				}
				TSharedRef<SDockTab> NewTab = SNew(SDockTab)
				.TabRole(NomadTab)
				.Label(LOCTEXT("ImGui_WS_WindowTitle", "ImGui WS"))
				[
					SNew(SImGuiPanelLifetime)
					[
						Panel.ToSharedRef()
					]
				];
				NewTab->SetTabIcon(FAppStyle::Get().GetBrush("LevelEditor.Tab"));
				return NewTab;
			}));
			return WindowTabId;
		}();
		FGlobalTabmanager::Get()->TryInvokeTab(Id);
	}
	else if (PanelMode == EImGuiLocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
	{
		if (WindowPtr.IsValid())
		{
			return;
		}
		static int32 ContextIndex = 0;
		const TSharedPtr<SImGuiPanel> Panel = CreatePanel(ContextIndex);
		if (Panel == nullptr)
		{
			return;
		}
		const TSharedRef<SWindow> Window =
			SNew(SWindow)
			.Title(LOCTEXT("ImGui_WS_WindowTitle", "ImGui WS"))
			.ClientSize(FVector2f(1000.f, 800.f))
			[
				SNew(SImGuiPanelLifetime)
				[
					Panel.ToSharedRef()
				]
			];
		WindowPtr = Window;
		FSlateApplication::Get().AddWindow(Window);
	}
	else
	{
		for (auto It = ViewportPanelMap.CreateIterator(); It; ++It)
		{
			if (It.Key().IsValid())
			{
				continue;
			}
			It.RemoveCurrent();
		}
		UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
		if (GameViewport == nullptr)
		{
			return;
		}
		FViewportPanel& ViewportPanel = ViewportPanelMap.FindOrAdd(GameViewport);
		if (ViewportPanel.Overlay.IsValid())
		{
			return;
		}
		const UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
		if (Manager == nullptr)
		{
			return;
		}
		*ViewportPanel.ContextIndex = Manager->GetWorldSubsystems().IndexOfByPredicate([World](const UImGuiUnrealContextWorldSubsystem* E)
		{
			return E && E->GetWorld() == World;
		});

		const TSharedRef<SImGuiLocalPanelOverlay> Overlay = SNew(SImGuiLocalPanelOverlay, GameViewport);
		const auto PanelManager = SNew(SPanelManager, Overlay, GameViewport, ViewportPanel.ContextIndex);
		PanelManager->CachedPos = Overlay->Config->ManagerConfig.Pos;
		PanelManager->CachedSize = Overlay->Config->ManagerConfig.Size;
		Overlay->AddChild(PanelManager, PanelManager->CachedPos, PanelManager->CachedSize, false);
		if (Overlay->Config->bManagerCollapsed)
		{
			PanelManager->ForceMinimize();
		}

		ViewportPanel.Overlay = Overlay;
		GameViewport->AddViewportWidgetContent(Overlay, INT32_MAX - 1);
	}
}

void CloseLocalWindow(UWorld* World)
{
	const EImGuiLocalPanelMode PanelMode{ static_cast<uint8>(CVarLocalPanelMode.GetValueOnAnyThread()) };
	CloseLocalWindow(World, PanelMode);
}
void CloseLocalWindow(UWorld* World, EImGuiLocalPanelMode PanelMode)
{
	if (PanelMode == EImGuiLocalPanelMode::DockWindow && GIsEditor)
	{
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(WindowTabId))
		{
			Tab->RequestCloseTab();
		}
	}
	else if (PanelMode == EImGuiLocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
	{
		if (WindowPtr.IsValid())
		{
			WindowPtr.Pin()->RequestDestroyWindow();
		}
	}
	else
	{
		UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
		if (GameViewport == nullptr)
		{
			return;
		}
		const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(GameViewport);
		if (ViewportPanel && ViewportPanel->Overlay.IsValid())
		{
			GameViewport->RemoveViewportWidgetContent(ViewportPanel->Overlay.Pin().ToSharedRef());
			ViewportPanelMap.Remove(GameViewport);
		}
	}
}
}

#undef LOCTEXT_NAMESPACE