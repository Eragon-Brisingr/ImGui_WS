// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiUnrealLocalPanel.h"

#include "ImGuiDelegates.h"
#include "imgui_internal.h"
#include "ImGuiEx.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "SImGuiPanel.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiString.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformFileManager.h"
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

namespace ImGui_WS::LocalPanel
{
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

		SLATE_BEGIN_ARGS(SDragResizeBox)
		{}
		SLATE_EVENT(FOnDragged, OnDragged);
		SLATE_EVENT(FOnResized, OnResized);
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
				return FReply::Handled();
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
				const FGeometry& CanvasGeometry = Owner->Canvas->GetTickSpaceGeometry();
				const FVector4f& V = *RequirePositionScale;
				const FVector2f Position = CanvasGeometry.GetLocalSize() * FVector2f{ V.X, V.Y };
				const FVector2f Size = CanvasGeometry.GetLocalSize() * FVector2f{ V.Z, V.W };
				const FVector2f Offset = CanvasGeometry.GetLocalSize() - (Position + Size);
				Slot->SetOffset(FMargin{ Position.X, Position.Y, Offset.X, Offset.Y });

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

		SDragResizeContainer* Owner = nullptr;
		SConstraintCanvas::FSlot* Slot = nullptr;
		TOptional<FVector4f> RequirePositionScale;
		FOnDragged OnDragged;
		FOnResized OnResized;
	};

	SLATE_BEGIN_ARGS(SDragResizeContainer)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args)
	{
		ChildSlot
		[
			SAssignNew(Canvas, SConstraintCanvas)
		];
	}

	void AddChild(const TSharedRef<SDragResizeBox>& DragResizeBox, const FVector2f& RelativePosition, const FVector2f& RelativeSize)
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
TMap<TWeakObjectPtr<UWorld>, FViewportPanel> ViewportPanelMap;
TAutoConsoleVariable<int32> CVarLocalPanelMode
{
	TEXT("ImGui.WS.LocalPanelMode"),
#if WITH_EDITOR
	(int32)ELocalPanelMode::DockWindow,
#elif PLATFORM_DESKTOP
	(int32)ELocalPanelMode::SingleWindow,
#else
	(int32)ELocalPanelMode::GameViewport,
#endif
	TEXT("0: open in game viewport\n")
	TEXT("1: open as single window (desktop platform only)\n")
	TEXT("2: open as dockable window (editor only)")
};
const FName WindowTabId = TEXT("ImGui_WS_Window");
bool IsLocalWindowOpened(const UWorld* World)
{
	if (WindowPtr.IsValid())
	{
		return true;
	}
	if (const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(World))
	{
		return ViewportPanel->Overlay.IsValid();
	}
	return FGlobalTabmanager::Get()->FindExistingLiveTab(WindowTabId) != nullptr;
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
				Manager->DrawViewport(ContextIndex, DeltaSeconds);
			}
		});

	ImGuiContext* OldContent = ImGui::GetCurrentContext();
	ON_SCOPE_EXIT
	{
		ImGui::SetCurrentContext(OldContent);
	};
	ImGui::SetCurrentContext(Panel->GetContext());

	ImGuiIO& IO = ImGui::GetIO();

	// Enable Docking
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
	// Make sure that directory is created.
	PlatformFile.CreateDirectory(*IniDirectory);
	static const UnrealImGui::FUTF8String IniFilePath = IniDirectory / TEXT("Imgui_WS_Window.ini");
	IO.IniFilename = IniFilePath.GetData();
	return Panel;
}
void OpenLocalWindow(UWorld* World)
{
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

	const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
	if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
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
	else if (PanelMode == ELocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
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
		FViewportPanel& ViewportPanel = ViewportPanelMap.FindOrAdd(World);
		if (ViewportPanel.Overlay.IsValid())
		{
			return;
		}
		UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
		if (GameViewport == nullptr)
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

		static constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		class SImGuiLocalPanelOverlay : public SDragResizeContainer, public FGCObject
		{
			using Super = SDragResizeContainer;
		public:
			void Construct(const FArguments& Args, UWorld* World)
			{
				Config = NewObject<UImGuiLocalPanelManagerConfig>(World, TEXT("ImGuiLocalPanelOverlayConfig"));
				Super::Construct(Args);
				FImGuiDelegates::OnImGuiLocalPanelEnable.Broadcast();
			}
			~SImGuiLocalPanelOverlay() override
			{
				FImGuiDelegates::OnImGuiLocalPanelDisable.Broadcast();
			}
			TMap<TObjectPtr<UUnrealImGuiPanelBase>, TSharedRef<SDragResizeBox>> PanelBoxMap;
			TSharedPtr<SDragResizeBox> ViewportPtr;
			TObjectPtr<UImGuiLocalPanelManagerConfig> Config;
			UnrealImGui::FUTF8String FilterString;

			void OpenPanel(UWorld* World, UUnrealImGuiPanelBuilder* Builder, UUnrealImGuiPanelBase* Panel)
			{
				ensure(PanelBoxMap.Contains(Panel) == false);
				const TSharedRef<SImGuiPanel> ImGuiPanel = SNew(SImGuiPanel)
					.OnImGuiTick_Lambda([World, Builder, PanelPtr = TWeakObjectPtr<UUnrealImGuiPanelBase>(Panel)](float DeltaSeconds)
					{
						UUnrealImGuiPanelBase* Panel = PanelPtr.Get();
						if (Panel == nullptr)
						{
							return;
						}
						ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
						ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
						if (ImGui::FWindow Window{ "Panel", nullptr, Panel->ImGuiWindowFlags | SinglePanelFlags })
						{
							Panel->Draw(World, Builder, DeltaSeconds);
						}
					});
				const FImGuiLocalPanelConfig PanelConfig = Config->PanelConfigMap.FindRef(Panel->GetClass());
				ImGuiPanel->GetContext()->IO.ConfigFlags = ImGuiConfigFlags_DockingEnable;
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
					~SImGuiPanelBox() override
					{
						if (UUnrealImGuiPanelBase* Panel = PanelPtr.Get())
						{
							Panel->LocalPanelClosed();
						}
					}
				};
				const auto DragResizeBox = SNew(SImGuiPanelBox, Panel)
					.TitleContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(20.f, 2.f, 2.f, 2.f)
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&GetTextBlockStyles().Large)
							.Text(FText::FromName(Panel->Title))
						]
						+ SHorizontalBox::Slot()
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
						]
					]
					.Content()
					[
						ImGuiPanel
					]
					.OnDragged_Lambda([this, Panel](const FVector2f& Pos)
					{
						Config->PanelConfigMap.FindOrAdd(Panel->GetClass()).Pos = Pos;
						Config->SaveConfig();
					})
					.OnResized_Lambda([this, Panel](const FVector2f& Size)
					{
						Config->PanelConfigMap.FindOrAdd(Panel->GetClass()).Size = Size;
						Config->SaveConfig();
					});
				AddChild(DragResizeBox, PanelConfig.Pos, PanelConfig.Size);
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
				Collector.AddReferencedObjects(PanelBoxMap);
			}
			FString GetReferencerName() const override
			{
				return TEXT("SImGuiLocalPanelOverlay");
			}
		};
		const TSharedRef<SImGuiLocalPanelOverlay> Overlay = SNew(SImGuiLocalPanelOverlay, World);
		class SPanelManager : public SDragResizeContainer::SDragResizeBox
		{
			using Super = SDragResizeBox;
		public:
			SLATE_BEGIN_ARGS(SPanelManager)
			{}
			SLATE_END_ARGS()
			SImGuiLocalPanelOverlay* Overlay;
			UWorld* World;
			void Construct(const FArguments& Args, SImGuiLocalPanelOverlay* InOverlay, UWorld* InWorld, const TSharedRef<int32>& ContextIndex)
			{
				Overlay = InOverlay;
				World = InWorld;
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
						.Text(LOCTEXT("ImGuiPanelManagerTitle", "ImGui Panel Manager"))
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
							if (Overlay->Config->bManagerCollapsed)
							{
								return FReply::Handled();
							}
							Overlay->Config->bManagerCollapsed = true;
							Overlay->Config->SaveConfig();
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
					SNew(SImGuiPanel)
					.Visibility_Lambda([this]
					{
						return Overlay->Config->bManagerCollapsed ? EVisibility::Collapsed : EVisibility::Visible;
					})
					.OnImGuiTick_Lambda([this, ContextIndex](float DeltaSeconds)
					{
						FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World);
						if (LayoutManager == nullptr)
						{
							return;
						}
						ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
						ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
						if (ImGui::FWindow Window{ "Panel", nullptr, SinglePanelFlags | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar })
						{
							static auto DrawPanelCheckBox = [](UWorld* InWorld, SImGuiLocalPanelOverlay* InOverlay, UUnrealImGuiPanelBuilder* Builder, UUnrealImGuiPanelBase* Panel)
							{
								bool IsOpen = InOverlay->PanelBoxMap.Contains(Panel);
								if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Panel->Title.ToString(), *Panel->GetClass()->GetName())), &IsOpen))
								{
									if (IsOpen)
									{
										auto& RecentlyPanels = InOverlay->Config->RecentlyPanels;
										const bool bRemoved = RecentlyPanels.RemoveSingle(Panel->GetClass()) > 0;
										RecentlyPanels.Insert(Panel->GetClass(), 0);
										constexpr int32 MaxRecentlyPanelNum = 20;
										if (bRemoved == false && RecentlyPanels.Num() > MaxRecentlyPanelNum)
										{
											RecentlyPanels.RemoveAt(MaxRecentlyPanelNum, RecentlyPanels.Num() - MaxRecentlyPanelNum);
										}
										InOverlay->Config->SaveConfig();

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
									for (const FName& Category : Panel->Categories)
									{
										CategoryPath += TEXT(",") + Category.ToString();
									}
									if (CategoryPath.Len() > 0)
									{
										CategoryPath.RemoveAt(CategoryPath.Len() - 1);
									}
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*FText::Format(LOCTEXT("PanelCheckBoxTooltip", "Class: {0}\nCategoryPath:{1}"), FText::FromString(Panel->GetClass()->GetPathName()), FText::FromString(CategoryPath)).ToString()));
								}
							};
							if (ImGui::FMenuBar MenuBar{})
							{
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
									const FLocal Local{ World, Overlay };
									for (const auto& [_, Child] : Builder->CategoryPanels.Children)
									{
										Local.DrawCategory(Builder, *Child);
									}
									Local.DrawPanelState(Builder, Builder->CategoryPanels);
								}
								if (ImGui::FMenu Menu{ "Viewport" })
								{
									ImGui::Separator();
									bool IsOpen = Overlay->ViewportPtr.IsValid();
									if (ImGui::Checkbox("ImGui Debug Viewport", &IsOpen))
									{
										if (IsOpen)
										{
											ensure(Overlay->ViewportPtr.IsValid() == false);
											const TSharedPtr<SImGuiPanel> Panel = CreatePanel(*ContextIndex);
											if (Panel)
											{
												const auto DragResizeBox = SNew(SDragResizeContainer::SDragResizeBox)
													.TitleContent()
													[
														SNew(SHorizontalBox)
														+ SHorizontalBox::Slot()
														.FillWidth(1.f)
														.Padding(20.f, 2.f, 2.f, 2.f)
														.VAlign(VAlign_Center)
														[
															SNew(STextBlock)
															.TextStyle(&GetTextBlockStyles().Large)
															.Text(LOCTEXT("ImGuiViewportTitle", "ImGui Viewport"))
														]
														+ SHorizontalBox::Slot()
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
														]
													]
													.Content()
													[
														Panel.ToSharedRef()
													]
													.OnDragged_Lambda([this](const FVector2f& Pos)
													{
														Overlay->Config->ViewportConfig.Pos = Pos;
														Overlay->Config->SaveConfig();
													})
													.OnResized_Lambda([this](const FVector2f& Size)
													{
														Overlay->Config->ViewportConfig.Size = Size;
														Overlay->Config->SaveConfig();
													});
												Overlay->ViewportPtr = DragResizeBox;
												Overlay->AddChild(DragResizeBox, Overlay->Config->ViewportConfig.Pos, Overlay->Config->ViewportConfig.Size);
											}
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
								}
							}

							constexpr float ItemWidth = 100.f;
							const int32 ColumnsNum = FMath::Max(FMath::FloorToInt32(ImGui::GetWindowSize().x / ItemWidth), 1);
							ImGui::Text("Recently Panels"); ImGui::SameLine(); ImGui::Separator();
							if (ImGui::FTreeNodeEx TreeNodeEx{ "Recently Panels", ImGuiTreeNodeFlags_DefaultOpen })
							{
								if (ImGui::FTable Table{ "RecentlyPanels", ColumnsNum })
								{
									int32 ItemCounter = 0;
									for (int32 Idx = 0; Idx < Overlay->Config->RecentlyPanels.Num(); ++Idx)
									{
										const auto& RecentlyPanel = Overlay->Config->RecentlyPanels[Idx];
										UUnrealImGuiPanelBuilder* Builder = nullptr;
										UUnrealImGuiPanelBase* Panel = nullptr;
										for (UUnrealImGuiPanelBuilder* PanelBuilder : LayoutManager->PanelBuilders)
										{
											const int32 PanelIdx = PanelBuilder->Panels.IndexOfByPredicate([&](const UUnrealImGuiPanelBase* E){ return E->GetClass() == RecentlyPanel; });
											if (PanelIdx != INDEX_NONE)
											{
												Builder = PanelBuilder;
												Panel = Builder->Panels[PanelIdx];
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
												DrawPanelCheckBox(World, Overlay, Builder, Panel);
												ItemCounter += 1;
											}
										}
									}
								}
							}

							ImGui::Text("Search Panel"); ImGui::SameLine(); ImGui::Separator();
							if (ImGui::FTreeNodeEx TreeNodeEx{ "Search Panel" })
							{
								UnrealImGui::FUTF8String& FilterString = Overlay->FilterString;
								ImGui::InputTextWithHint("##SearchPanel", "Search Panel", FilterString);
								if (ImGui::FListBox ListBox{ "##FilteredPanel", ImVec2{ 0.f, 100.f } })
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
												DrawPanelCheckBox(World, Overlay, Builder, Builder->Panels[PanelIdx]);
												ImGui::PopID();
											}
										}
									}
									else
									{
										TArray<UUnrealImGuiPanelBase*> FilteredPanels;
										const FString TestString = FilterString.ToString().ToLower();
										for (const UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
										{
											for (UUnrealImGuiPanelBase* Panel : Builder->Panels)
											{
												if (Panel->Title.ToString().ToLower().Contains(TestString) || Panel->GetClass()->GetName().ToLower().Contains(TestString))
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
													DrawPanelCheckBox(World, Overlay, Builder, FilteredPanels[Idx]);
												}
												ImGui::PopID();
											}
										}
									}
								}
							}

							ImGui::Text("Category Panel"); ImGui::SameLine(); ImGui::Separator();
							for (UUnrealImGuiPanelBuilder* Builder : LayoutManager->PanelBuilders)
							{
								struct FLocal
								{
									const int32 ColumnsNum;
									UWorld* World;
									SImGuiLocalPanelOverlay* Overlay;
									using FCategoryPanels = UUnrealImGuiPanelBuilder::FCategoryPanels;
									void DrawCategory(const FString& CategoryName, ImGuiTreeNodeFlags_ ImGuiTreeNodeFlags, UUnrealImGuiPanelBuilder* Builder, const FCategoryPanels& Panels) const
									{
										if (ImGui::FTreeNodeEx TreeNodeEx{ TCHAR_TO_UTF8(*CategoryName), ImGuiTreeNodeFlags })
										{
											for (const auto& [_, Child] : Panels.Children)
											{
												DrawCategory(Child->Category.ToString(), ImGuiTreeNodeFlags_None, Builder, *Child);
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
									}
								};
								FLocal{ ColumnsNum, World, Overlay }.DrawCategory(TCHAR_TO_UTF8(*Builder->GetOuter()->GetClass()->GetName()), ImGuiTreeNodeFlags_DefaultOpen, Builder, Builder->CategoryPanels);
							}
						}
					})
				]
				.OnDragged_Lambda([this](const FVector2f& Pos)
				{
					CachedPos = Pos;
					Overlay->Config->ManagerConfig.Pos = Pos;
					Overlay->Config->SaveConfig();
				})
				.OnResized_Lambda([this](const FVector2f& Size)
				{
					CachedSize = Size;
					Overlay->Config->ManagerConfig.Size = Size;
					Overlay->Config->SaveConfig();
				});

				Super::Construct(SuperArgs);
				const TSharedRef<SWidget> PanelWidget = ChildSlot.GetWidget();
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
							if (Overlay->Config->bManagerCollapsed == false)
							{
								return FReply::Handled();
							}
							Overlay->Config->bManagerCollapsed = false;
							Overlay->Config->SaveConfig();
							RestorePanel();
							return FReply::Handled();
						})
						[
							SNew(SBox)
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
							]
						]
					]
				];
			}

			void RestorePanel()
			{
				Slot->SetAnchors(FAnchors{ 0.f, 0.f, 1.f, 1.f });
				Slot->SetAutoSize(false);
				WidgetSwitcher->SetActiveWidgetIndex(0);
				RequirePositionScale = FVector4f{ CachedPos.X, CachedPos.Y, CachedSize.X, CachedSize.Y };
			}
			void Minimize()
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
		const auto PanelManager = SNew(SPanelManager, &Overlay.Get(), World, ViewportPanel.ContextIndex);
		PanelManager->CachedPos = Overlay->Config->ManagerConfig.Pos;
		PanelManager->CachedSize = Overlay->Config->ManagerConfig.Size;
		Overlay->AddChild(PanelManager, PanelManager->CachedPos, PanelManager->CachedSize);
		if (Overlay->Config->bManagerCollapsed)
		{
			PanelManager->Minimize();
		}

		ViewportPanel.Overlay = Overlay;
		GameViewport->AddViewportWidgetContent(Overlay, INT32_MAX - 1);
	}
}
void CloseLocalWindow(UWorld* World)
{
	const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
	if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
	{
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(WindowTabId))
		{
			Tab->RequestCloseTab();
		}
	}
	else if (PanelMode == ELocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
	{
		if (WindowPtr.IsValid())
		{
			WindowPtr.Pin()->RequestDestroyWindow();
		}
	}
	else
	{
		const FViewportPanel* ViewportPanel = ViewportPanelMap.Find(World);
		if (ViewportPanel && ViewportPanel->Overlay.IsValid())
		{
			if (UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr)
			{
				GameViewport->RemoveViewportWidgetContent(ViewportPanel->Overlay.Pin().ToSharedRef());
			}
			ViewportPanelMap.Remove(World);
		}
	}
}
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
}

#undef LOCTEXT_NAMESPACE
