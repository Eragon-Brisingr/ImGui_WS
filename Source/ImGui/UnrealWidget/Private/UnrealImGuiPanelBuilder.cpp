// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanelBuilder.h"

#include "imgui.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanel.h"
#include "UnrealImGuiStat.h"

FUnrealImGuiPanelBuilder::FUnrealImGuiPanelBuilder()
{

}

void FUnrealImGuiPanelBuilder::Register(UObject* Owner)
{
	if (DockSpaceName == NAME_None)
	{
		checkNoEntry();
		return;
	}

	// 只对继承树叶节点的类型生效
	auto RemoveNotLeafClass = [](TArray<UClass*>& Classes)
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
	
	TSet<const UClass*> VisitedLayoutClasses;
	{
		TArray<UClass*> LayoutClasses;
		GetDerivedClasses(UUnrealImGuiLayoutBase::StaticClass(), LayoutClasses);
		RemoveNotLeafClass(LayoutClasses);
		TSet<FName> ExistLayoutNames;
		for (UClass* Class : LayoutClasses)
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

			const FName LayoutName = *CDO->LayoutName.ToString();
			if (ensure(LayoutName != NAME_None && ExistLayoutNames.Contains(LayoutName) == false))
			{
				ExistLayoutNames.Add(LayoutName);

				UUnrealImGuiLayoutBase* Layout = NewObject<UUnrealImGuiLayoutBase>(Owner, Class, Class->GetFName(), RF_Transient);
				const int32 Idx = Layouts.Add(Layout);
				if (ActiveLayoutClass == Class)
				{
					ActiveLayoutIndex = Idx;
				}
			}
		}
		for (UUnrealImGuiLayoutBase* Layout : Layouts)
		{
			Layout->Register(Owner, *this);
		}
	}

	TSet<const UClass*> VisitedPanelClasses;
	{
		TArray<UClass*> PanelClasses;
		GetDerivedClasses(UUnrealImGuiPanelBase::StaticClass(), PanelClasses);
		RemoveNotLeafClass(PanelClasses);
		TSet<FName> ExistPanelNames;
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

			const UUnrealImGuiPanelBase* CDO = Class->GetDefaultObject<UUnrealImGuiPanelBase>();
			if (CDO->ShouldCreatePanel(Owner) == false)
			{
				continue;
			}

			const FName PanelName = *CDO->Title.ToString();
			if (ensure(PanelName != NAME_None && ExistPanelNames.Contains(PanelName) == false))
			{
				ExistPanelNames.Add(PanelName);

				UUnrealImGuiPanelBase* Panel = NewObject<UUnrealImGuiPanelBase>(Owner, Class, Class->GetFName(), RF_Transient);
				Panels.Add(Panel);
			}
		}
	}

	const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
	for (UUnrealImGuiPanelBase* Panel : Panels)
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
		Panel->Register(Owner);
	}
}

void FUnrealImGuiPanelBuilder::Unregister(UObject* Owner)
{
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
		Panel->Unregister(Owner);
	}
}

void FUnrealImGuiPanelBuilder::LoadDefaultLayout(UObject* Owner)
{
	UUnrealImGuiLayoutBase* Layout = Layouts[ActiveLayoutIndex];
	Layout->LoadDefaultLayout(Owner, *this);
}

void FUnrealImGuiPanelBuilder::DrawPanels(UObject* Owner, float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiPanelBuilder_DrawPanels"), STAT_UnrealImGuiPanelBuilder_DrawPanels, STATGROUP_ImGui);
	
	UUnrealImGuiLayoutBase* Layout = Layouts[ActiveLayoutIndex];
	Layout->CreateDockSpace(Owner, *this);
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		Panel->DrawWindow(Layout, Owner, DeltaSeconds);
	}
}

void FUnrealImGuiPanelBuilder::DrawLayoutStateMenu(UObject* Owner)
{
	for (int32 Idx = 0; Idx < Layouts.Num(); ++Idx)
	{
		const UUnrealImGuiLayoutBase* Layout = Layouts[Idx];
		if (ImGui::RadioButton(TCHAR_TO_UTF8(*Layout->LayoutName.ToString()), ActiveLayoutIndex == Idx))
		{
			ActiveLayoutIndex = Idx;
			ActiveLayoutClass = Layout->GetClass();
			Owner->SaveConfig();
		}
	}
}

void FUnrealImGuiPanelBuilder::DrawPanelStateMenu(UObject* Owner)
{
	for (UUnrealImGuiPanelBase* Panel : Panels)
	{
		bool IsOpen = Panel->bIsOpen;
		if (ImGui::Checkbox(TCHAR_TO_UTF8(*Panel->Title.ToString()), &IsOpen))
		{
			const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
			Panel->SetOpenState(IsOpen);
			Panel->PanelOpenState.Add(Layout->GetClass()->GetFName(), IsOpen);
			Panel->SaveConfig();
		}
	}
}

UUnrealImGuiPanelBase* FUnrealImGuiPanelBuilder::FindPanel(const TSubclassOf<UUnrealImGuiPanelBase>& PanelType) const
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
