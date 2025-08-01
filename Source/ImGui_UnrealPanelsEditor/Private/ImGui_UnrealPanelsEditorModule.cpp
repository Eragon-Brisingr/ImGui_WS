﻿#include "ImGui_UnrealPanelsEditorModule.h"

#include "ImGuiEx.h"
#include "ImGuiFontAtlas.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "imgui_internal.h"
#include "LevelEditor.h"
#include "SImGuiPanel.h"
#include "ToolMenus.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiStat.h"
#include "UnrealImGuiStyles.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Engine/BlueprintCore.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformFileManager.h"
#include "Textures/SlateIcon.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "ImGui_UnrealPanelsEditor"

void FImGui_UnrealPanelsEditorModule::StartupModule()
{
	ImGuiPanelsGroup = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(
		TEXT("ImGuiPanelsGroup"),
		LOCTEXT("WorkspaceMenu_ImGuiPanelsGroup", "ImGui Panels"),
		LOCTEXT("ImGuiPanelsGroupTooltipText", "Custom UI created with ImGui panel."),
		FSlateIcon(FSlateIcon("ImGuiEditorStyle", TEXT("ImGui.Icon"))),
		false);

	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		static bool bTabManagerUpdated = false;
		OnTabManagerChangedHandle = LevelEditorModule->OnTabManagerChanged().AddLambda([this]
		{
			bTabManagerUpdated = true;
			RefreshGroupMenu();
		});
		OnPostEditChangePropertyHandle = UImGuiSettings::OnPostEditChangeProperty.AddLambda([this](UImGuiSettings*, FPropertyChangedEvent& Event)
		{
			if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UImGuiSettings, BlueprintPanels))
			{
				RefreshGroupMenu();
			}
		});
		OnCDOConstructHandle = UUnrealImGuiPanelBase::OnCDOConstruct_Editor.AddLambda([this](const UUnrealImGuiPanelBase* CDO)
		{
			if (bTabManagerUpdated && GEditor && Cast<UBlueprintCore>(CDO->GetClass()->ClassGeneratedBy) == nullptr)
			{
				static bool bInvokeUpdated = false;
				if (bInvokeUpdated == false)
				{
					GEditor->GetTimerManager()->SetTimerForNextTick([this]
					{
						RefreshGroupMenu();
					});
				}
			}
		});
	}

	if (FSlateApplication::IsInitialized() && GetDefault<UImGuiSettings>()->bDisplayToolbarButton)
	{
		ToolMenusHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
			FToolMenuSection& Section = Menu->AddSection("PlayGameExtensions", TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));

			FToolMenuEntry AssetValidationEntry = FToolMenuEntry::InitComboButton(
				"ImGuiPanels",
				FUIAction{},
				FOnGetContent::CreateLambda([]
				{
					FMenuBuilder MenuBuilder(true, nullptr);
					MenuBuilder.SetSearchable(true);

					auto GetPanelName = [](const TSoftClassPtr<UObject>& PanelClass)
					{
						if (UClass* Class = PanelClass.Get())
						{
							auto CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
							return CDO->Title != NAME_None ? CDO->Title.ToString() : CDO->GetName();
						}
						return PanelClass.GetAssetName();
					};

					auto CreateMenuEntry = [&](const TArray<TSoftClassPtr<UObject>>& Panels)
					{
						for (const auto& PanelClass : Panels)
						{
							const FName TabId = FName{ *FString::Printf(TEXT("%s_ImGuiPanel"), *PanelClass.ToString()) };
						
							MenuBuilder.AddMenuEntry(FText::FromString(GetPanelName(PanelClass)), FText::FromString(PanelClass.ToString()),
								FSlateIcon("ImGuiEditorStyle", TEXT("ImGui.Icon")),
								FUIAction(
									FExecuteAction::CreateLambda([TabId]
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

										LevelEditorTabManager->TryInvokeTab(TabId);
									}), FCanExecuteAction{}, FGetActionCheckState::CreateLambda([TabId]
									{
										FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
										if (LevelEditorModule == nullptr)
										{
											return ECheckBoxState::Undetermined;
										}
										TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
										if (LevelEditorTabManager == nullptr)
										{
											return ECheckBoxState::Undetermined;
										}
										return LevelEditorTabManager->FindExistingLiveTab(TabId) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
									})), NAME_None, EUserInterfaceActionType::Check);
						}
					};
					
					MenuBuilder.BeginSection(TEXT("Recently"), LOCTEXT("Recently", "Recently"));
					CreateMenuEntry(GetDefault<UImGuiPerUserSettings>()->RecentlyOpenPanels);
					MenuBuilder.EndSection();

					auto Settings = GetDefault<UImGuiSettings>();
					if (Settings->FavoritePanels.Num() > 0)
					{
						MenuBuilder.BeginSection(TEXT("Favorite"), LOCTEXT("Favorite", "Favorite"));
						CreateMenuEntry(Settings->FavoritePanels);
						MenuBuilder.EndSection();
					}

					return MenuBuilder.MakeWidget();
				}),
				LOCTEXT("ToolbarImGuiPanels_Label", "ImGui Panels"),
				LOCTEXT("ToolbarImGuiPanels_ToolTip", "Open ImGui Panels\nAdd Favorite Panels in ProjectSetting-Plugins-ImGui_WS settings"),
				FSlateIcon("ImGuiEditorStyle", TEXT("ImGui.Icon"))
			);

			AssetValidationEntry.StyleNameOverride = "CalloutToolbar";
			Section.AddEntry(AssetValidationEntry);
		}));
	}
}

void FImGui_UnrealPanelsEditorModule::ShutdownModule()
{
	WorkspaceMenu::GetMenuStructure().GetToolsCategory()->RemoveItem(ImGuiPanelsGroup.ToSharedRef());

	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		LevelEditorModule->OnTabManagerChanged().Remove(OnTabManagerChangedHandle);
	}
	UImGuiSettings::OnPostEditChangeProperty.Remove(OnPostEditChangePropertyHandle);
	UUnrealImGuiPanelBase::OnCDOConstruct_Editor.Remove(OnCDOConstructHandle);
	
	if (UObjectInitialized() && ToolMenusHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(ToolMenusHandle);
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
		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}

		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}
		
		const UUnrealImGuiPanelBase* CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
		UpdateCategoryPanels(CDO->Title.IsNone() ? Class->GetFName() : CDO->Title, CDO->Categories, Class);
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
					DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiEditorPanel_Tick"), STAT_ImGuiEditorPanel_Tick, STATGROUP_ImGui);

					UImGuiUnrealContextManager* ContextManager = GEngine->GetEngineSubsystem<UImGuiUnrealContextManager>();
					if (ContextManager == nullptr)
					{
						return;
					}
					
					UClass* Class = PanelClass.Get();
					if (Class && Class->HasAnyClassFlags(CLASS_NewerVersionExists))
					{
						PanelClass = PanelClass.ToSoftObjectPath();
						Class = PanelClass.Get();
					}

					static int32 ContextIndex = 0;
					if (auto MainMenuBar = ImGui::FMainMenuBar{})
					{
						if (ImGui::BeginMenu("World Context"))
						{
							ContextManager->DrawContextContent(ContextIndex);
							ImGui::EndMenu();
						}
						if (Class && ImGui::BeginMenu("Settings"))
						{
							UnrealImGui::ShowStyleSelector();

							auto& IO = ImGui::GetIO();
							auto Settings = GetMutableDefault<UImGuiPerUserSettings>();
							ImGui::SetNextItemWidth(ImGui::GetFontSize());
							float* DPIScaleSettings = Settings->CustomPanelDPIScaleMap.Find(Class);
							bool bOverrideDPIScale = DPIScaleSettings ? true : false; 
							if (ImGui::Checkbox("##Override DPI Scale", &bOverrideDPIScale))
							{
								if (bOverrideDPIScale)
								{
									DPIScaleSettings = &Settings->CustomPanelDPIScaleMap.Add(Class, UnrealImGui::GetGlobalDPIScale());
									IO.FontGlobalScale = *DPIScaleSettings;
								}
								else
								{
									Settings->CustomPanelDPIScaleMap.Remove(Class);
									DPIScaleSettings = nullptr;
									IO.FontGlobalScale = UnrealImGui::GetGlobalDPIScale();
								}
								Settings->SaveConfig();
							}
							if (ImGui::BeginItemTooltip())
							{
								ImGui::Text("Override DPI Scale");
								ImGui::EndTooltip();
							}
							ImGui::SameLine();
							ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
							ImGui::BeginDisabled(!bOverrideDPIScale);
							float DPIScale = DPIScaleSettings ? *DPIScaleSettings : UnrealImGui::GetGlobalDPIScale();
							if (ImGui::InputFloat("DPI Scale", &DPIScale, 0.01f))
							{
								DPIScale = FMath::Clamp(DPIScale, 0.5f, 3.f);
								IO.FontGlobalScale = DPIScale;
								Settings->CustomPanelDPIScaleMap.Add(Class, DPIScale);
								Settings->SaveConfig();
							}
							ImGui::EndDisabled();

							ImGui::EndMenu();
						}
					}

					const float MenuSizeY = ImGui::GetCursorPosY();
					ImGui::SetNextWindowPos({ 0, MenuSizeY });
					const auto ViewportSize = ImGui::GetWindowViewport()->Size;
					ImGui::SetNextWindowSize({ ViewportSize.x, ViewportSize.y - MenuSizeY });
					static constexpr auto SinglePanelFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	
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
						Panel = nullptr;
						FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(World);
						if (LayoutManager == nullptr)
						{
							return;
						}
						for (UUnrealImGuiPanelBuilder* PanelBuilder : LayoutManager->PanelBuilders)
						{
							if (UUnrealImGuiPanelBase* TestPanel = PanelBuilder->GetPanelsMap().FindRef(Class))
							{
								Panel = TestPanel;
								Panel->LocalPanelOpened();
								DrawPanel = Panel;
								Builder = PanelBuilder;
								break;
							}
						}
					}
					if (Panel == nullptr)
					{
						if (ImGui::FWindow Window{ "NoFindInstancePanel", nullptr, SinglePanelFlags })
						{
							ImGui::TextUnformatted("No active panel instance");
						}
						return;
					}
					if (ImGui::FWindow Window{ TCHAR_TO_UTF8(*Panel->Title.ToString()), nullptr, Panel->ImGuiWindowFlags | SinglePanelFlags })
					{
						UImGuiUnrealContextManager::OnPreDraw.Broadcast(World);
						Panel->Draw(World, Builder.Get(), DeltaSeconds);
						UImGuiUnrealContextManager::OnPostDraw.Broadcast(World);
					}
				});
			auto Context = ImGuiPanel->GetContext();
			auto& IO = Context->IO;
			IO.ConfigFlags = ImGuiConfigFlags_DockingEnable;
			auto Settings = GetMutableDefault<UImGuiPerUserSettings>();
			UnrealImGui::DefaultStyle(&Context->Style);
			if (auto DPIScale = Settings->CustomPanelDPIScaleMap.Find(Class))
			{
				IO.FontGlobalScale = *DPIScale;
			}
			else
			{
				IO.FontGlobalScale = UnrealImGui::GetGlobalDPIScale();
			}

			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
			// Make sure that directory is created.
			PlatformFile.CreateDirectory(*IniDirectory);
			static const FUtf8String IniFilePath{ IniDirectory / TEXT("ImGui_EditorPanels.ini") };
			IO.IniFilename = reinterpret_cast<const char*>(*IniFilePath);

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
				const FName RegistrationName = *FString::Printf(TEXT("%s_ImGuiPanel"), *Panel.Class.ToString());
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

					// Ignore FTabManager::RestoreFrom case
					if (GFrameCounter > 0)
					{
						GetMutableDefault<UImGuiPerUserSettings>()->RecordRecentlyOpenPanel(Class);
					}

					SpawnedTab->SetContent(SNew(SImGuiPanelContent, Class));
					return SpawnedTab;
				}))
				.SetDisplayName(FText::FromName(Panel.Name))
				.SetTooltipText(FText::FromString(Panel.Class.ToString()))
				.SetGroup(Group)
				.SetIcon(FSlateIcon("ImGuiEditorStyle", TEXT("ImGui.Icon")));
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
