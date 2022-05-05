// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPanelBuilder.h"

#include "imgui.h"
#include "UnrealImGuiLayout.h"
#include "UnrealImGuiPanel.h"

void FUnrealImGuiPanelBuilder::Register(UObject* Owner)
{
	if (DockSpaceName == NAME_None || SupportPanelType == nullptr || SupportLayoutType == nullptr)
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
	
	{
		TArray<UClass*> LayoutClasses;
		GetDerivedClasses(SupportLayoutType, LayoutClasses);
		RemoveNotLeafClass(LayoutClasses);
		TSet<FName> ExistLayoutNames;
		for (UClass* Class : LayoutClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			const FName LayoutName = *Class->GetDefaultObject<UUnrealImGuiLayoutBase>()->LayoutName.ToString();
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

	{
		TArray<UClass*> PanelClasses;
		GetDerivedClasses(SupportPanelType, PanelClasses);
		RemoveNotLeafClass(PanelClasses);
		TSet<FName> ExistPanelNames;
		for (UClass* Class : PanelClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}
			
			const FName PanelName = *Class->GetDefaultObject<UUnrealImGuiPanelBase>()->Title.ToString();
			if (ensure(PanelName != NAME_None && ExistPanelNames.Contains(PanelName) == false))
			{
				ExistPanelNames.Add(PanelName);

				UUnrealImGuiPanelBase* Panel = NewObject<UUnrealImGuiPanelBase>(Owner, Class, Class->GetFName(), RF_Transient);
				Panels.Add(Panel);
			}
		}

		const UUnrealImGuiLayoutBase* Layout = GetActiveLayout();
		for (UUnrealImGuiPanelBase* Panel : Panels)
		{
			if (const bool* IsOpenPtr = Panel->PanelOpenState.Find(Layout->GetClass()->GetFName()))
			{
				Panel->bIsOpen = *IsOpenPtr;
			}
			else
			{
				Panel->bIsOpen = true;
			}
			Panel->Register(Owner);
		}
	}
}

void FUnrealImGuiPanelBuilder::Unregister(UObject* Owner)
{
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
			Panel->bIsOpen = IsOpen;
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
