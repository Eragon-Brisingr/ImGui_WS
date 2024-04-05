// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiUnrealLocalPanel.h"

#include "imgui.h"
#include "ImGuiUnrealContextManager.h"
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
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

namespace ImGui_WS::LocalPanel
{
class SDragResizeContainer : public SCompoundWidget
{
	class SDragResizeBox : public SCompoundWidget
	{
		using Super = SCompoundWidget;

		SDragResizeContainer* Owner = nullptr;
		SConstraintCanvas::FSlot* Slot = nullptr;
		TOptional<FVector4f> InitialPositionScale;
	public:
		SLATE_BEGIN_ARGS(SDragResizeBox)
		{}
		SLATE_NAMED_SLOT(FArguments, TitleContent)
		SLATE_NAMED_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		class SDragArea : public SBorder
		{
			using Super = SBorder;

			SDragResizeBox* Owner = nullptr;
			bool bBeingDragged = false;
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
				return FReply::Handled().ReleaseMouseCapture();
			}
			FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (bBeingDragged)
				{
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
				return FReply::Handled().ReleaseMouseCapture();
			}
			FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
			{
				if (bBeingResized)
				{
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

		void Construct(const FArguments& Args, const FVector4f& InInitialPositionScale, SDragResizeContainer* InOwner, SConstraintCanvas::FSlot* InSlot)
		{
			InitialPositionScale = InInitialPositionScale;
			Owner = InOwner;
			Slot = InSlot;
			TSharedRef<SWidget> Child = Args._Content.Widget;

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
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						Child
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					[
						SNew(SResizeArea, this)
						.BorderImage(FAppStyle::GetBrush("NoBorder"))
						.ContentScale(1.f)
						.Padding(0.f)
						[
							SNew(STextBlock).Text(LOCTEXT("ResizeText", "◢"))
						]
					]
				]
			];
		}
		void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
		{
			Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

			if (InitialPositionScale)
			{
				const FGeometry& CanvasGeometry = Owner->Canvas->GetTickSpaceGeometry();
				const FVector4f& V = *InitialPositionScale;
				const FVector2f Position = CanvasGeometry.GetLocalSize() * FVector2f{ V.X, V.Y };
				const FVector2f Size = CanvasGeometry.GetLocalSize() * FVector2f{ V.Z, V.W };
				const FVector2f Offset = CanvasGeometry.GetLocalSize() - (Position + Size);
				Slot->SetOffset(FMargin{ Position.X, Position.Y, Offset.X, Offset.Y });

				InitialPositionScale.Reset();
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
	};

	TSharedPtr<SConstraintCanvas> Canvas;
	TMap<TSharedRef<SWidget>, TSharedRef<SDragResizeBox>> ContentBoxMap;
public:
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

	void AddChild(const TSharedRef<SWidget>& Title, const TSharedRef<SWidget>& Content, const FVector2f& RelativePosition, const FVector2f& RelativeSize)
	{
		TSharedPtr<SDragResizeBox> DragResizeBox;
		SConstraintCanvas::FSlot* Slot;
		Canvas->AddSlot()
		.Expose(Slot)
		.Anchors(FAnchors{ 0.f, 0.f, 1.f, 1.f })
		.Offset(FMargin{ 0.f, 0.f, 100000.f, 100000.f })
		[
			SAssignNew(DragResizeBox, SDragResizeBox, FVector4f{ RelativePosition.X, RelativePosition.Y, RelativeSize.X, RelativeSize.Y }, this, Slot)
			.TitleContent()
			[
				Title
			]
			.Content()
			[
				Content
			]
		];
		ContentBoxMap.Add(Content, DragResizeBox.ToSharedRef());
	}

	void RemoveChild(const TSharedRef<SWidget>& Content)
	{
		if (!ContentBoxMap.Contains(Content))
		{
			return;
		}
		Canvas->RemoveSlot(ContentBoxMap.FindAndRemoveChecked(Content));
	}
};

TWeakPtr<SWindow> WindowPtr;
struct FViewportPanel
{
	TWeakPtr<SWidget> Overlay;
	TSharedRef<int32> ContextIndex = MakeShared<int32>();
};
TMap<TWeakObjectPtr<UWorld>, FViewportPanel> ViewportPanelMap;
enum class ELocalPanelMode : int32
{
	GameViewport = 0,
	SingleWindow = 1,
	DockWindow = 2,
};
TAutoConsoleVariable<int32> CVarLocalPanelMode
{
	TEXT("ImGui.WS.LocalPanelMode"),
	(int32)ELocalPanelMode::DockWindow,
	TEXT("0: open in game viewport\n")
	TEXT("1: open as single window (desktop platform only)\n")
	TEXT("2: open as dockable window (editor only)")
};
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
FName GetWindowTabId()
{
	static FName TabId = []
	{
		const FName Id = TEXT("ImGui_WS_Window");
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Id, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
		{
			static int32 ContextIndex = INDEX_NONE;
			TSharedPtr<SWidget> Panel = CreatePanel(ContextIndex);
			if (Panel == nullptr)
			{
				Panel = SNullWidget::NullWidget;
			}
			TSharedRef<SDockTab> NewTab = SNew(SDockTab)
			.TabRole(NomadTab)
			.Label(LOCTEXT("ImGui_WS_WindowTitle", "ImGui WS"))
			[
				Panel.ToSharedRef()
			];
			NewTab->SetTabIcon(FAppStyle::Get().GetBrush("LevelEditor.Tab"));
			return NewTab;
		}));
		return Id;
	}();
	return TabId;
}
void OpenImGuiWindow(UWorld* World)
{
	const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
	if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
	{
		FGlobalTabmanager::Get()->TryInvokeTab(GetWindowTabId());
	}
	else if (PanelMode == ELocalPanelMode::SingleWindow && PLATFORM_DESKTOP)
	{
		if (WindowPtr.IsValid())
		{
			return;
		}
		static int32 ContextIndex = INDEX_NONE;
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
				Panel.ToSharedRef()
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

		class SImGuiOverlay : public SDragResizeContainer
		{
		public:
			TMap<TWeakObjectPtr<UUnrealImGuiPanelBase>, TSharedRef<SWidget>> Panels;
			TSharedPtr<SWidget> ViewportPtr;

			void ClosePanel(UUnrealImGuiPanelBase* Panel)
			{
				if (Panels.Contains(Panel) == false)
				{
					return;
				}
				RemoveChild(Panels.FindAndRemoveChecked(Panel));
			};
		};

		const TSharedRef<SImGuiOverlay> Overlay = SNew(SImGuiOverlay);
		Overlay->AddChild(SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(20.f, 2.f, 2.f, 2.f)
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), TEXT("LargeText"))
				.Text(LOCTEXT("ImGuiPanelManagerTitle", "ImGui Panel Manager"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.HAlign(HAlign_Right)
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("MinimizeBox", "➖"))
				.ToolTipText(LOCTEXT("MinimizeBoxTooltip", "Minimize"))
				.OnClicked_Lambda([]
				{
					return FReply::Handled();
				})
			], SNew(SImGuiPanel).OnImGuiTick_Lambda([World, Overlay = &Overlay.Get(), ContextIndex = ViewportPanel.ContextIndex](float DeltaSeconds)
		{
			UImGuiUnrealContextWorldSubsystem* ImGuiUnrealContextWorld = UImGuiUnrealContextWorldSubsystem::Get(World);
			if (ImGuiUnrealContextWorld == nullptr)
			{
				return;
			}
			constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
			ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
			ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
			if (ImGui::Begin("Panel", nullptr, SinglePanelFlags | ImGuiWindowFlags_MenuBar))
			{
				if (ImGui::BeginMenuBar())
				{
					for (UUnrealImGuiPanelBuilder* Builder : ImGuiUnrealContextWorld->PanelBuilders)
					{
						for (UUnrealImGuiPanelBase* Panel : Builder->Panels)
						{
							int32 CategoryDepth = 0;
							for (const FText& Category : Panel->Categories)
							{
								if (ImGui::BeginMenu(TCHAR_TO_UTF8(*Category.ToString())))
								{
									CategoryDepth += 1;
								}
								else
								{
									break;
								}
							}
							if (CategoryDepth == Panel->Categories.Num())
							{
								bool IsOpen = Overlay->Panels.Contains(Panel);
								if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Panel->Title.ToString(), *Panel->GetClass()->GetName())), &IsOpen))
								{
									if (IsOpen)
									{
										ensure(Overlay->Panels.Contains(Panel) == false);
										TSharedRef<SImGuiPanel> ImGuiPanel = SNew(SImGuiPanel)
											.OnImGuiTick_Lambda([World, Builder, PanelPtr = TWeakObjectPtr<UUnrealImGuiPanelBase>(Panel)](float DeltaSeconds)
											{
												UUnrealImGuiPanelBase* Panel = PanelPtr.Get();
												if (Panel == nullptr)
												{
													return;
												}
												ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
												ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
												if (ImGui::Begin("Panel", nullptr, Panel->ImGuiWindowFlags | SinglePanelFlags))
												{
													Panel->Draw(World, Builder, DeltaSeconds);
												}
												ImGui::End();
											});
										Overlay->Panels.Add(Panel, ImGuiPanel);
										Overlay->AddChild(
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.FillWidth(1.f)
											.Padding(20.f, 2.f, 2.f, 2.f)
											[
												SNew(STextBlock)
												.TextStyle(FAppStyle::Get(), TEXT("LargeText"))
												.Text(Panel->Title)
											]
											+ SHorizontalBox::Slot()
											.AutoWidth()
											.Padding(2.f)
											[
												SNew(SButton)
												.Text(LOCTEXT("CloseBox", "✖"))
												.ToolTipText(LOCTEXT("CloseBoxTooltip", "Close"))
												.OnClicked_Lambda([Overlay, Panel]
												{
													Overlay->ClosePanel(Panel);
													return FReply::Handled();
												})
											], ImGuiPanel, FVector2f{ 0.1f, 0.1f }, FVector2f{ 0.4f, 0.4f });
									}
									else
									{
										Overlay->ClosePanel(Panel);
									}
								}
								if (ImGui::BeginItemTooltip())
								{
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*Panel->GetClass()->GetName()));
									ImGui::EndTooltip();
								}
							}
							for (int32 Idx = 0; Idx < CategoryDepth; ++Idx)
							{
								ImGui::EndMenu();
							}
						}
					}
					if (ImGui::BeginMenu("Viewport"))
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
									Overlay->ViewportPtr = Panel;
									Overlay->AddChild(
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.FillWidth(1.f)
										.Padding(20.f, 2.f, 2.f, 2.f)
										[
											SNew(STextBlock)
											.TextStyle(FAppStyle::Get(), TEXT("LargeText"))
											.Text(LOCTEXT("ImGuiViewportTitle", "ImGui Viewport"))
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.Padding(2.f)
										[
											SNew(SButton)
											.Text(LOCTEXT("CloseBox", "✖"))
											.ToolTipText(LOCTEXT("CloseBoxTooltip", "Close"))
											.OnClicked_Lambda([Overlay, Panel]
											{
												Overlay->RemoveChild(Panel.ToSharedRef());
												Overlay->ViewportPtr.Reset();
												return FReply::Handled();
											})
										], Panel.ToSharedRef(), FVector2f{ 0.1f, 0.1f }, FVector2f{ 0.8f, 0.8f });
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
						ImGui::EndMenu();
					}

					ImGui::EndMenuBar();
				}
			}
			ImGui::End();
		}), FVector2f{ 0.1f, 0.1f }, FVector2f{ 0.4f, 0.4f });

		ViewportPanel.Overlay = Overlay;
		GameViewport->AddViewportWidgetContent(Overlay, INT32_MAX - 1);
	}
}
FAutoConsoleCommand OpenImGuiWindowCommand
{
	TEXT("ImGui.WS.OpenPanel"),
	TEXT("Open ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateStatic(OpenImGuiWindow)
};
FAutoConsoleCommand CloseImGuiWindowCommand
{
	TEXT("ImGui.WS.ClosePanel"),
	TEXT("Close ImGui WS local panel"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		const ELocalPanelMode PanelMode{ CVarLocalPanelMode.GetValueOnAnyThread() };
		if (PanelMode == ELocalPanelMode::DockWindow && GIsEditor)
		{
			if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(GetWindowTabId()))
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
	})
};
}

#undef LOCTEXT_NAMESPACE
