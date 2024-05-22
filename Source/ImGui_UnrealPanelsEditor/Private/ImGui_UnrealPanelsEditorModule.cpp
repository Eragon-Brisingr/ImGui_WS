#include "ImGui_UnrealPanelsEditorModule.h"

#include "ImGuiEx.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "imgui_internal.h"
#include "LevelEditor.h"
#include "SImGuiPanel.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiPanelBuilder.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Styling/AppStyle.h"
#include "Textures/SlateIcon.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "ImGui_UnrealPanelsEditor"

void FImGui_UnrealPanelsEditorModule::StartupModule()
{
	ImGuiPanelsGroup = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(
		TEXT("ImGuiPanelsGroup"),
		LOCTEXT("WorkspaceMenu_ImGuiPanelsGroup", "ImGui Panels"),
		LOCTEXT("ImGuiPanelsGroupTooltipText", "Custom UI created with ImGui panel."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("WidgetReflector.TabIcon")),
		false);

	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		OnTabManagerChangedHandle = LevelEditorModule->OnTabManagerChanged().AddLambda([this]
		{
			RefreshGroupMenu();
		});
		if (UImGuiSettings* ImGuiSettings = GetMutableDefault<UImGuiSettings>())
		{
			OnPostEditChangePropertyHandle = ImGuiSettings->OnPostEditChangeProperty.AddLambda([this](UImGuiSettings*, FPropertyChangedEvent& Event)
			{
				if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UImGuiSettings, BlueprintPanels))
				{
					RefreshGroupMenu();
				}
			});
		}
	}
}

void FImGui_UnrealPanelsEditorModule::ShutdownModule()
{
	WorkspaceMenu::GetMenuStructure().GetToolsCategory()->RemoveItem(ImGuiPanelsGroup.ToSharedRef());
	
	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		LevelEditorModule->OnTabManagerChanged().Remove(OnTabManagerChangedHandle);
	}
	if (!IsEngineExitRequested())
	{
		if (UImGuiSettings* ImGuiSettings = GetMutableDefault<UImGuiSettings>())
		{
			ImGuiSettings->OnPostEditChangeProperty.Remove(OnPostEditChangePropertyHandle);
		}
	}
}

void FImGui_UnrealPanelsEditorModule::RefreshGroupMenu()
{
	FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
	if (LevelEditorModule == nullptr)
	{
		return;
	}

	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
	if (LevelEditorTabManager == nullptr)
	{
		return;
	}

	ImGuiPanelsGroup->ClearItems();

	struct FCategoryPanels
	{
		FCategoryPanels(const FName& Category)
			: Category{ Category }
		{}

		FName Category;
		struct FPanelInfo
		{
			FName Name;
			TSoftClassPtr<UUnrealImGuiPanelBase> Class;
		};
		TArray<FPanelInfo> Panels;
		TMap<FName, TUniquePtr<FCategoryPanels>> Children;
	};
	FCategoryPanels CategoryPanels{ NAME_None };
	
	auto UpdateCategoryPanels = [&](const FName& Name, const TArray<FName>& Categories, const TSoftClassPtr<UUnrealImGuiPanelBase>& PanelClass)
	{
		FCategoryPanels* Container = &CategoryPanels;
		for (const FName& Category : Categories)
		{
			if (const auto ContainerPtr = Container->Children.Find(Category))
			{
				Container = ContainerPtr->Get();
			}
			else
			{
				Container = Container->Children.Emplace(Category, MakeUnique<FCategoryPanels>(Category)).Get();
			}
		}
		Container->Panels.Add({ Name, PanelClass });
	};
	static auto RemoveNotLeafClass = [](TArray<UClass*>& Classes)
	{
		for (int32 Idx = 0; Idx < Classes.Num(); ++Idx)
		{
			const UClass* TestClass = Classes[Idx];
			const int32 ParentIdx = Classes.IndexOfByKey(TestClass->GetSuperClass());
			if (ParentIdx != INDEX_NONE)
			{
				Classes.RemoveAt(ParentIdx);
				if (ParentIdx < Idx)
				{
					Idx -= 1;
				}
			}
		}
	};
	TArray<UClass*> PanelClasses;
	GetDerivedClasses(UUnrealImGuiPanelBase::StaticClass(), PanelClasses);
	RemoveNotLeafClass(PanelClasses);
	for (const UClass* Class : PanelClasses)
	{
		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
		{
			continue;
		}

		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}
		
		const UUnrealImGuiPanelBase* CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
		UpdateCategoryPanels(CDO->Title, CDO->Categories, Class);
	}
	const UImGuiSettings* ImGuiSettings = GetDefault<UImGuiSettings>();
	for (const auto& BlueprintPanel : ImGuiSettings->BlueprintPanels)
	{
		if (BlueprintPanel.Get() != nullptr)
		{
			continue;
		}
		if (BlueprintPanel.IsNull())
		{
			continue;
		}
		UpdateCategoryPanels(*BlueprintPanel.GetAssetName(), { FName{ TEXT("Unload") } }, reinterpret_cast<const TSoftClassPtr<UUnrealImGuiPanelBase>&>(BlueprintPanel));
	}

	class SImGuiPanelContent : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SImGuiPanelContent)
		{}
		SLATE_END_ARGS()

		void Construct(const FArguments& Args, const TSoftClassPtr<UUnrealImGuiPanelBase>& Class)
		{
			PanelClass = Class;
			
			const TSharedRef<SImGuiPanel> ImGuiPanel = SNew(SImGuiPanel)
				.OnImGuiTick_Lambda([this](float DeltaSeconds)
				{
					UImGuiUnrealContextManager* ContextManager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
					if (ContextManager == nullptr)
					{
						return;
					}
					static int32 ContextIndex = 0;
					if (auto MainMenuBar = ImGui::FMainMenuBar{})
					{
						if (ImGui::BeginMenu("World Context"))
						{
							ContextManager->DrawContextContent(ContextIndex);
							ImGui::EndMenu();
						}
					}

					const float MenuSizeY = ImGui::GetCursorPosY();
					ImGui::SetNextWindowPos({ 0, MenuSizeY });
					const auto ViewportSize = ImGui::GetWindowViewport()->Size;
					ImGui::SetNextWindowSize({ ViewportSize.x, ViewportSize.y - MenuSizeY });
					static constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	
					UClass* Class = PanelClass.Get();
					if (Class == nullptr)
					{
						if (ImGui::FWindow Window{ "Panel", nullptr, SinglePanelFlags })
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s unload yet"), *PanelClass.ToString())));
						}
						return;
					}
					UWorld* World = ContextManager->GetContextIndexWorld(ContextIndex);
					
					UUnrealImGuiPanelBase* Panel = DrawPanel.Get();
					if (Panel == nullptr || Panel->GetWorld() != World)
					{
						if (Panel && Panel->GetWorld() != World)
						{
							Panel->LocalPanelClosed();
						}
						FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World);
						if (LayoutManager == nullptr)
						{
							return;
						}
						for (UUnrealImGuiPanelBuilder* PanelBuilder : LayoutManager->PanelBuilders)
						{
							for (UUnrealImGuiPanelBase* TestPanel : PanelBuilder->Panels)
							{
								if (TestPanel->GetClass() == Class)
								{
									Panel = TestPanel;
									Panel->LocalPanelOpened();
									DrawPanel = Panel;
									Builder = PanelBuilder;
									break;
								}
							}
							if (Panel)
							{
								break;
							}
						}
					}
					if (Panel == nullptr)
					{
						if (ImGui::FWindow Window{ "Panel", nullptr, SinglePanelFlags })
						{
							ImGui::TextUnformatted("No active panel instance");
						}
						return;
					}
					if (ImGui::FWindow Window{ "Panel", nullptr, Panel->ImGuiWindowFlags | SinglePanelFlags })
					{
						Panel->Draw(World, Builder.Get(), DeltaSeconds);
					}
				});
			ImGuiPanel->GetContext()->IO.ConfigFlags = ImGuiConfigFlags_DockingEnable;

			ChildSlot
			[
				ImGuiPanel
			];
		}

		~SImGuiPanelContent() override
		{
			if (UUnrealImGuiPanelBase* Panel = DrawPanel.Get())
			{
				Panel->LocalPanelClosed();
			}
		}
	private:
		TSoftClassPtr<UUnrealImGuiPanelBase> PanelClass;
		TWeakObjectPtr<UUnrealImGuiPanelBase> DrawPanel;
		TWeakObjectPtr<UUnrealImGuiPanelBuilder> Builder;
	};
	
	struct FLocal
	{
		TSharedPtr<FTabManager> TabManager;
		void AddChildGroup(const TSharedRef<FWorkspaceItem>& Group, const FCategoryPanels& Panels)
		{
			TSharedRef<FWorkspaceItem> ChildGroup = Group->AddGroup(Panels.Category, FText::FromName(Panels.Category), {}, false);
			AddItems(ChildGroup, Panels);
			for (const auto& [_, Child] : Panels.Children)
			{
				AddChildGroup(ChildGroup, *Child);
			}
		}
		void AddItems(const TSharedRef<FWorkspaceItem>& Group, const FCategoryPanels& Panels)
		{
			for (const auto& Panel : Panels.Panels)
			{
				FName RegistrationName = *FString::Printf(TEXT("%s_ImGuiPanel"), *Panel.Class.ToString());
				if (TabManager->HasTabSpawner(RegistrationName))
				{
					TabManager->UnregisterTabSpawner(RegistrationName);
				}
				TabManager->RegisterTabSpawner(RegistrationName, FOnSpawnTab::CreateLambda([Class = Panel.Class](const FSpawnTabArgs&)
				{
					TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab);

					UImGuiUnrealContextManager* Manager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
					if (Manager == nullptr)
					{
						return SpawnedTab;
					}
					FImGuiUnrealEditorContext* EditorContext = Manager->GetImGuiEditorContext();
					if (EditorContext == nullptr)
					{
						return SpawnedTab;
					}
					if (EditorContext->InvokeCreateDebugger)
					{
						EditorContext->InvokeCreateDebugger();
					}
										
					SpawnedTab->SetContent(SNew(SImGuiPanelContent, Class));
					return SpawnedTab;
				}))
				.SetDisplayName(FText::FromName(Panel.Name))
				.SetGroup(Group)
				.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("WidgetReflector.TabIcon")));
			}
		}
	};
	FLocal Local{ LevelEditorTabManager };
	for (const auto& [_, Child] : CategoryPanels.Children)
	{
		Local.AddChildGroup(ImGuiPanelsGroup.ToSharedRef(), *Child);
	}
	Local.AddItems(ImGuiPanelsGroup.ToSharedRef(), CategoryPanels);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_UnrealPanelsEditorModule, ImGui_UnrealPanelsEditor)
