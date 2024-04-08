// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanelBuilder.h"

#include "imgui.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiStat.h"
#include "Engine/AssetManager.h"

UUnrealImGuiPanelBuilder::UUnrealImGuiPanelBuilder()
{

}

void UUnrealImGuiPanelBuilder::Register(UObject* Owner)
{
	if (DockSpaceName == NAME_None)
	{
		checkNoEntry();
		return;
	}

	if (UImGuiUnrealContextWorldSubsystem* ImGuiUnrealContextWorld = UImGuiUnrealContextWorldSubsystem::Get(this))
	{
		ImGuiUnrealContextWorld->PanelBuilders.Add(this);
	}

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
		if (Panel->Title.IsEmpty())
		{
			Panel->Title = FText::FromName(Panel->GetClass()->GetFName());
		}
		return Panel;
	};

	static auto RegisterPanel = [](UObject* Owner, const UUnrealImGuiLayoutBase* Layout, UUnrealImGuiPanelBase* Panel, UUnrealImGuiPanelBuilder* Builder)
	{
		if (const bool* IsOpenPtr = Panel->PanelOpenState.Find(Layout->GetClass()->GetFName()))
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

	TSet<const UClass*> VisitedLayoutClasses;
	{
		TArray<UClass*> LayoutClasses;
		GetDerivedClasses(UUnrealImGuiLayoutBase::StaticClass(), LayoutClasses);
		RemoveNotLeafClass(LayoutClasses);
		for (const UClass* Class : LayoutClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			bool bIsAlreadyInSet;
			VisitedLayoutClasses.Add(Class, &bIsAlreadyInSet);
			if (bIsAlreadyInSet)
			{
				continue;
			}

			const UUnrealImGuiLayoutBase* CDO = Class->GetDefaultObject<UUnrealImGuiLayoutBase>();
			if (CDO->ShouldCreateLayout(Owner) == false)
			{
				continue;
			}

			UUnrealImGuiLayoutBase* Layout = NewObject<UUnrealImGuiLayoutBase>(this, Class, Class->GetFName(), RF_Transient);
			if (Layout->LayoutName.IsEmpty())
			{
				Layout->LayoutName = FText::FromName(Layout->GetClass()->GetFName());
			}
			const int32 Idx = Layouts.Add(Layout);
			if (ActiveLayoutClass == Class)
			{
				ActiveLayoutIndex = Idx;
			}
		}
		for (UUnrealImGuiLayoutBase* Layout : Layouts)
		{
			Layout->Register(Owner, *this);
		}
	}

	auto UpdateCategoryPanels = [this](UUnrealImGuiPanelBase* Panel)
	{
		FCategoryPanels* Container = &CategoryPanels;
		for (const FText& Category : Panel->Categories)
		{
			const FName CategoryName = *Category.ToString();
			if (const auto ContainerPtr = Container->Children.Find(CategoryName))
			{
				Container = ContainerPtr->Get();
			}
			else
			{
				Container = Container->Children.Emplace(CategoryName, MakeUnique<FCategoryPanels>(Category)).Get();
			}
		}
		Container->Panels.Add(Panel);
	};
	TSet<const UClass*> VisitedPanelClasses;
	{
		TArray<UClass*> PanelClasses;
		GetDerivedClasses(UUnrealImGuiPanelBase::StaticClass(), PanelClasses);
		RemoveNotLeafClass(PanelClasses);
		for (const UClass* Class : PanelClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			bool bIsAlreadyInSet;
			VisitedPanelClasses.Add(Class, &bIsAlreadyInSet);
			if (bIsAlreadyInSet)
			{
				continue;
			}

#if WITH_EDITOR
			if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
			{
				continue;
			}
#endif

			const UUnrealImGuiPanelBase* CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
			if (CDO->ShouldCreatePanel(Owner) == false)
			{
				continue;
			}

			UUnrealImGuiPanelBase* Panel = CreatePanel(this, Class);
			Panels.Add(Panel);
			UpdateCategoryPanels(Panel);
		}

		TArray<FSoftObjectPath> ToLoadClasses;
		const auto& ImGuiSettings = GetDefault<UImGuiSettings>()->BlueprintPanels;
		for (const TSoftClassPtr<UUnrealImGuiPanelBase>& Panel : ImGuiSettings)
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
				for (const FSoftObjectPath ObjectPath: ToLoadClasses)
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
					UUnrealImGuiPanelBase* Panel = CreatePanel(this, Class);
					UpdateCategoryPanels(Panel);
					Panels.Add(Panel);
				}
				const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
				for (; Idx < Panels.Num(); ++Idx)
				{
					RegisterPanel(Owner, Layout, Panels[Idx], this);
				}
			}));
		}
	}

	const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		RegisterPanel(Owner, Layout, Panel, this);
	}
}

void UUnrealImGuiPanelBuilder::Unregister(UObject* Owner)
{
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->CancelHandle();
	}
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		if (Panel->bIsOpen)
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

	if (UImGuiUnrealContextWorldSubsystem* ImGuiUnrealContextWorld = UImGuiUnrealContextWorldSubsystem::Get(this))
	{
		ImGuiUnrealContextWorld->PanelBuilders.Remove(this);
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
		Panel->DrawWindow(Layout, Owner, this, DeltaSeconds);
	}
}

void UUnrealImGuiPanelBuilder::DrawPanelStateMenu(UObject* Owner)
{
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
				DrawPanelState(Layout, Panels);
				ImGui::EndMenu();
			}
		}
		static void DrawPanelState(const UUnrealImGuiLayoutBase* Layout, const FCategoryPanels& Panels)
		{
			for (UUnrealImGuiPanelBase* Panel : Panels.Panels)
			{
				bool IsOpen = Panel->bIsOpen;
				if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Panel->Title.ToString(), *Panel->GetClass()->GetName())), &IsOpen))
				{
					Panel->SetOpenState(IsOpen);
					Panel->PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
					Panel->SaveConfig();
				}
				if (ImGui::BeginItemTooltip())
				{
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*Panel->GetClass()->GetName()));
					ImGui::EndTooltip();
				}
			}
		}
	};
	const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
	for (const auto& [_, Child] : CategoryPanels.Children)
	{
		FLocal::DrawCategory(Layout, *Child);
	}
	FLocal::DrawPanelState(Layout, CategoryPanels);
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

UUnrealImGuiPanelBase* UUnrealImGuiPanelBuilder::FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const
{
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		if (Panel->IsA(PanelType))
		{
			return Panel->bIsOpen ? Panel : nullptr;
		}
	}
	return nullptr;
}
