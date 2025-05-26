// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanelBuilder.h"

#include "imgui.h"
#include "ImGuiSettings.h"
#include "UnrealImGuiLayoutSubsystem.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiStat.h"
#include "Engine/AssetManager.h"

template <bool bReenter>
void UUnrealImGuiPanelBuilder::ConstructPanels(UObject* Owner)
{
	// 只对继承树叶节点的类型生效
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

	static auto CreatePanel = [](UUnrealImGuiPanelBuilder* Builder, const UClass* Class)
	{
		UUnrealImGuiPanelBase* Panel = NewObject<UUnrealImGuiPanelBase>(Builder, Class, Class->GetFName(), RF_Transient);
		if (Panel->Title == NAME_None)
		{
			Panel->Title = Panel->GetClass()->GetFName();
		}
		return Panel;
	};

	static auto RegisterPanel = [](UObject* Owner, const UUnrealImGuiLayoutBase* Layout, UUnrealImGuiPanelBase* Panel, UUnrealImGuiPanelBuilder* Builder)
	{
		Panel->InitialConfigObject();
		if (const bool* IsOpenPtr = Panel->ConfigObjectPrivate->PanelOpenState.Find(Layout->GetClass()->GetFName()))
		{
			Panel->SetOpenState(*IsOpenPtr);
		}
		else if (const auto* LayoutSettings = Panel->DefaultDockSpace.Find(Layout->GetClass()->GetFName()))
		{
			Panel->SetOpenState(LayoutSettings->bOpen);
		}
		else
		{
			Panel->SetOpenState(Panel->DefaultState.bOpen);
		}
		Panel->Register(Owner, Builder);
	};

	{
		const int32 OldLayoutNum = Layouts.Num();
		
		TArray<UClass*> LayoutClasses;
		GetDerivedClasses(UUnrealImGuiLayoutBase::StaticClass(), LayoutClasses);
		RemoveNotLeafClass(LayoutClasses);
		for (const UClass* Class : LayoutClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			const UUnrealImGuiLayoutBase* CDO = Class->GetDefaultObject<UUnrealImGuiLayoutBase>();
			if (CDO->ShouldCreateLayout(Owner) == false)
			{
				continue;
			}

			if constexpr (bReenter)
			{
				if (Layouts.ContainsByPredicate([&](const UUnrealImGuiLayoutBase* E){ return E->GetClass() == Class; }))
				{
					continue;
				}
			}
			
			UUnrealImGuiLayoutBase* Layout = NewObject<UUnrealImGuiLayoutBase>(this, Class, Class->GetFName(), RF_Transient);
			if (Layout->LayoutName == NAME_None)
			{
				Layout->LayoutName = Layout->GetClass()->GetFName();
			}
			const int32 Idx = Layouts.Add(Layout);
			if (ActiveLayoutClass == Class)
			{
				ActiveLayoutIndex = Idx;
			}
		}
		for (int32 Idx = OldLayoutNum; Idx < Layouts.Num(); ++Idx)
		{
			UUnrealImGuiLayoutBase* Layout = Layouts[Idx];
			Layout->Register(Owner, *this);
		}
	}

	auto UpdateCategoryPanels = [this](UUnrealImGuiPanelBase* Panel)
	{
		FCategoryPanels* Container = &CategoryPanels;
		for (const FName& Category : Panel->Categories)
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
		Container->Panels.Add(Panel);
	};
	{
		const int32 OldPanelNum = Panels.Num();

		TArray<UClass*> PanelClasses;
		GetDerivedClasses(UUnrealImGuiPanelBase::StaticClass(), PanelClasses);
		RemoveNotLeafClass(PanelClasses);
		for (const UClass* Class : PanelClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
			{
				continue;
			}

#if WITH_EDITOR
			if (Class->GetName().StartsWith(TEXT("SKEL_")))
			{
				continue;
			}
#endif

			const UUnrealImGuiPanelBase* CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
			if (CDO->ShouldCreatePanel(Owner) == false)
			{
				continue;
			}

			if constexpr (bReenter)
			{
				if (PanelsMap.Contains(Class))
				{
					continue;
				}
			}

			UUnrealImGuiPanelBase* Panel = CreatePanel(this, Class);
			Panels.Add(Panel);
			PanelsMap.Add(Class, Panel);
			UpdateCategoryPanels(Panel);
		}
		const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
		for (int32 Idx = OldPanelNum; Idx < Panels.Num(); ++Idx)
		{
			RegisterPanel(Owner, Layout, Panels[Idx], this);
		}

		TArray<FSoftObjectPath> ToLoadClasses;
		const auto& ImGuiSettings = GetDefault<UImGuiSettings>()->BlueprintPanels;
		for (const TSoftClassPtr<UObject>& Panel : ImGuiSettings)
		{
			if (Panel.Get() == nullptr)
			{
				ToLoadClasses.AddUnique(Panel.ToSoftObjectPath());
			}
		}
		if (ToLoadClasses.Num() > 0)
		{
			StreamableHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(ToLoadClasses, FStreamableDelegate::CreateWeakLambda(Owner, [ToLoadClasses, Owner, this, UpdateCategoryPanels]
			{
				StreamableHandle.Reset();
				TArray<UClass*> Classes;
				for (const FSoftObjectPath& ObjectPath: ToLoadClasses)
				{
					UClass* Class = Cast<UClass>(ObjectPath.ResolveObject());
					if (Class == nullptr)
					{
						continue;
					}
					if (Class->GetDefaultObject<UUnrealImGuiPanelBase>()->ShouldCreatePanel(Owner) == false)
					{
						continue;
					}
					Classes.Add(Class);
				}
				RemoveNotLeafClass(Classes);
				int32 Idx = Panels.Num();
				for (const UClass* Class : Classes)
				{
					if (PanelsMap.Contains(Class))
					{
						continue;
					}
					
					UUnrealImGuiPanelBase* Panel = CreatePanel(this, Class);
					UpdateCategoryPanels(Panel);
					Panels.Add(Panel);
					PanelsMap.Add(Class, Panel);
				}
				const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
				for (; Idx < Panels.Num(); ++Idx)
				{
					RegisterPanel(Owner, Layout, Panels[Idx], this);
				}
			}));
		}
	}
}

void UUnrealImGuiPanelBuilder::Register(UObject* Owner)
{
	if (DockSpaceName == NAME_None)
	{
		checkNoEntry();
		return;
	}

	if (FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(this))
	{
		LayoutManager->PanelBuilders.Add(this);
	}

	ConstructPanels<false>(Owner);
}

void UUnrealImGuiPanelBuilder::Unregister(UObject* Owner)
{
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->CancelHandle();
	}
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		if (Panel->IsOpenedInLayout())
		{
			Panel->SetOpenState(false);
		}
	}
	for (UUnrealImGuiLayoutBase* Layout : Layouts)
	{
		Layout->Unregister(Owner, *this);
	}
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		Panel->Unregister(Owner, this);
	}
	Layouts.Reset();
	Panels.Reset();
	PanelsMap.Reset();

	if (FUnrealImGuiLayoutManager* LayoutManager = FUnrealImGuiLayoutManager::Get(this))
	{
		LayoutManager->PanelBuilders.Remove(this);
	}
}

void UUnrealImGuiPanelBuilder::LoadDefaultLayout(UObject* Owner)
{
	UUnrealImGuiLayoutBase* Layout = Layouts[ActiveLayoutIndex];
	Layout->LoadDefaultLayout(Owner, *this);
}

void UUnrealImGuiPanelBuilder::DrawPanels(UObject* Owner, float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiPanelBuilder_DrawPanels"), STAT_UnrealImGuiPanelBuilder_DrawPanels, STATGROUP_ImGui);
	
	UUnrealImGuiLayoutBase* Layout = Layouts[ActiveLayoutIndex];
	Layout->CreateDockSpace(Owner, *this);
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		if (Panel == nullptr)
		{
			continue;
		}
		Panel->DrawWindow(Layout, Owner, this, DeltaSeconds);
	}
}

void UUnrealImGuiPanelBuilder::DrawPanelStateMenu(UObject* Owner)
{
	ImGui::SeparatorText("Panels");

	struct FLocal
	{
		static void DrawCategory(const UUnrealImGuiLayoutBase* Layout, const FCategoryPanels& Panels)
		{
			if (ImGui::BeginMenu(TCHAR_TO_UTF8(*Panels.Category.ToString())))
			{
				for (const auto& [_, Child] : Panels.Children)
				{
					DrawCategory(Layout, *Child);
				}
				DrawPanelsState(Layout, Panels);
				ImGui::EndMenu();
			}
		}
		static void DrawPanelsState(const UUnrealImGuiLayoutBase* Layout, const FCategoryPanels& Panels)
		{
			for (UUnrealImGuiPanelBase* Panel : Panels.Panels)
			{
				DrawPanelState(Layout, Panel);
			}
		}
		static void DrawPanelState(const UUnrealImGuiLayoutBase* Layout, UUnrealImGuiPanelBase* Panel)
		{
			bool IsOpen = Panel->IsOpenedInLayout();
			if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Panel->Title.ToString(), *Panel->GetClass()->GetName())), &IsOpen))
			{
				Panel->SetOpenState(IsOpen);
				if (IsOpen)
				{
					GetMutableDefault<UImGuiPerUserSettings>()->RecordRecentlyOpenPanel(Panel->GetClass());
				}
				UUnrealImGuiPanelBase* ConfigObject = Panel->ConfigObjectPrivate;
				ConfigObject->PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
				ConfigObject->SaveConfig();
			}
			if (ImGui::BeginItemTooltip())
			{
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*Panel->GetClass()->GetName()));
				ImGui::EndTooltip();
			}
		}
	};
	const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
	for (const auto& [_, Child] : CategoryPanels.Children)
	{
		FLocal::DrawCategory(Layout, *Child);
	}
	FLocal::DrawPanelsState(Layout, CategoryPanels);

	ImGui::SeparatorText("Utility");
	if (ImGui::BeginMenu("Favorite"))
	{
		bool bHasFavorite = false;
		for (const auto& PanelClass : GetDefault<UImGuiSettings>()->FavoritePanels)
		{
			const int32 Idx = Panels.IndexOfByPredicate([&](const UUnrealImGuiPanelBase* E) { return E->GetClass() == PanelClass.Get(); });
			if (Idx == INDEX_NONE)
			{
				continue;
			}
			FLocal::DrawPanelState(Layout, Panels[Idx]);
			bHasFavorite = true;
		}
		if (!bHasFavorite)
		{
			ImGui::TextUnformatted("No favorite panel");
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Recently"))
	{
		bool bHasRecently = false;
		for (const auto& PanelClass : GetDefault<UImGuiPerUserSettings>()->RecentlyOpenPanels)
		{
			const int32 Idx = Panels.IndexOfByPredicate([&](const UUnrealImGuiPanelBase* E) { return E->GetClass() == PanelClass.Get(); });
			if (Idx == INDEX_NONE)
			{
				continue;
			}
			FLocal::DrawPanelState(Layout, Panels[Idx]);
			bHasRecently = true;
		}
		if (bHasRecently == false)
		{
			ImGui::TextUnformatted("No recently history");
		}
		ImGui::EndMenu();
	}
}

void UUnrealImGuiPanelBuilder::DrawLayoutStateMenu(UObject* Owner)
{
	for (int32 Idx = 0; Idx < Layouts.Num(); ++Idx)
	{
		const UUnrealImGuiLayoutBase* Layout = Layouts[Idx];
		if (ImGui::RadioButton(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Layout->LayoutName.ToString(), *Layout->GetClass()->GetName())), ActiveLayoutIndex == Idx))
		{
			ActiveLayoutIndex = Idx;
			ActiveLayoutClass = Layout->GetClass();
			SaveConfig();
		}
		if (ImGui::BeginItemTooltip())
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*Layout->GetClass()->GetName()));
			ImGui::EndTooltip();
		}
	}
}

#if WITH_EDITOR
void UUnrealImGuiPanelBuilder::ReconstructPanels_Editor(UObject* Owner)
{
	ConstructPanels<true>(Owner);
}
#endif

UUnrealImGuiPanelBase* UUnrealImGuiPanelBuilder::FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const
{
	if (UUnrealImGuiPanelBase* Panel = PanelsMap.FindRef(PanelType))
	{
		return Panel->IsOpened() ? Panel : nullptr;
	}
	return nullptr;
}
