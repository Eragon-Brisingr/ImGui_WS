// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiObjectBrowser.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_notify.h"
#include "UnrealImGuiPropertyDetails.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiWrapper.h"

#define LOCTEXT_NAMESPACE "UnrealImGui"

UUnrealImGuiObjectBrowserPanel::UUnrealImGuiObjectBrowserPanel()
	: bDisplayAllProperties(true)
	, bEnableEditVisibleProperty(false)
{
	DefaultState = FDefaultPanelState{ false, true };
	Title = LOCTEXT("Object Browser", "Object Browser");
}

void UUnrealImGuiObjectBrowserPanel::Draw(UObject* Owner, float DeltaSeconds)
{
	if (ImGui::BeginChild("ObjectPathViewer", { 0.f, 30.f }, false, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
	{
		if (ImGui::Button("Root"))
		{
			SelectedObject = nullptr;
		}
		if (SelectedObject)
		{
			TArray<UObject*, TInlineAllocator<16>> Outers;
			for (UObject* TestObject = SelectedObject; TestObject; TestObject = TestObject->GetOuter())
			{
				Outers.Add(TestObject);
			}

			for (int32 Idx = Outers.Num() - 1; Idx >= 0; --Idx)
			{
				ImGui::SameLine();
				ImGui::Text(">");

				ImGui::SameLine();
				UObject* Object = Outers[Idx];
				if (ImGui::Button(TCHAR_TO_UTF8(*Object->GetName())))
				{
					SelectedObject = Object;
				}
			}
		}
		ImGui::EndChild();
	}

	const ImGuiWindowClass* WindowClass = &ImGui::GetCurrentWindowRead()->WindowClass;
	if (DockSpaceId == INDEX_NONE)
	{
		DockSpaceId = ImGui::GetID("ObjectBrowser");
		const ImGuiID DockId = ImGui::DockBuilderAddNode(DockSpaceId, ImGuiDockNodeFlags_AutoHideTabBar);
		ImGuiID RemainAreaId;
		const ImGuiID ViewportId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Left, 0.5f, nullptr, &RemainAreaId);
		ImGui::DockBuilderDockWindow("ObjectBrowserContent", ViewportId);
		ImGui::DockBuilderDockWindow("ObjectBrowserDetails", RemainAreaId);
		ImGui::DockBuilderFinish(DockId);
	}
	ImGui::DockSpace(DockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_AutoHideTabBar, WindowClass);
	
	ImGui::SetNextWindowClass(WindowClass);
	if (ImGui::Begin("ObjectBrowserContent"))
	{
		static UnrealImGui::FUTF8String FilterString;
		const bool bInvokeSearch = UnrealImGui::InputTextWithHint("##Filter", "Filter (Input full path then press Enter to search object)", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
		if (bInvokeSearch && ImGui::IsItemDeactivatedAfterEdit())
		{
			SelectedObject = FindObject<UObject>(nullptr, *FilterString.ToString());
			if (SelectedObject == nullptr)
			{
				SelectedObject = LoadObject<UObject>(nullptr, *FilterString.ToString());
				if (SelectedObject)
				{
					ImGui::InsertNotification(ImGuiToastType_Info, "load \"%s\" object", *FilterString);
				}
				else
				{
					ImGui::InsertNotification(ImGuiToastType_Error, "\"%s\" object not find", *FilterString);
				}
			}
			FilterString.Empty();
		}
		TArray<UObject*> DisplayObjects;
		int32 AllObjectCount = 0;
		{
			const FString Filter = FilterString.ToString();
			if (SelectedObject == nullptr)
			{
				GetObjectsOfClass(UPackage::StaticClass(), DisplayObjects);
				AllObjectCount = DisplayObjects.Num();
				if (Filter.Len() > 0)
				{
					DisplayObjects.RemoveAllSwap([&Filter](const UObject* E){ return E->GetName().Contains(Filter) == false; });
				}
			}
			else
			{
				ForEachObjectWithOuter(SelectedObject, [&](UObject* SubObject)
				{
					AllObjectCount += 1;
					if (Filter.Len() > 0 && SubObject->GetName().Contains(Filter) == false)
					{
						return;
					}
					DisplayObjects.Add(SubObject);
				}, false);
			}
		}
		ImGui::SameLine();
		ImGui::Text("Filter %d | Total %d", AllObjectCount, DisplayObjects.Num());
		ImGui::Separator();
		
		const FString Filter = FilterString.ToString();

		constexpr ImGuiTableFlags OutlinerTableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
			ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

		if (ImGui::BeginTable("ContentTable", 1, OutlinerTableFlags))
		{
			ImGuiListClipper ListClipper{};
			ListClipper.Begin(DisplayObjects.Num());
			while (ListClipper.Step())
			{
				for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
				{
					ImGui::TableNextColumn();
					UObject* Object = DisplayObjects[Idx];
					if (ImGui::Selectable(TCHAR_TO_UTF8(*Object->GetName())))
					{
						SelectedObject = Object;
						FilterString.Empty();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("Class: %s"), *Object->GetClass()->GetName())));
						ImGui::Text("InPackage: %s", Object->HasAnyFlags(RF_Load) ? "true" : "false");
						ImGui::EndTooltip();
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}

	ImGui::SetNextWindowClass(WindowClass);
	if (ImGui::Begin("ObjectBrowserDetails", nullptr, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Detail Settings"))
			{
				{
					bool Value = bDisplayAllProperties;
					if (ImGui::Checkbox("Display All Properties", &Value))
					{
						bDisplayAllProperties = Value;
						Owner->SaveConfig();
					}
				}
				{
					bool Value = bEnableEditVisibleProperty;
					if (ImGui::Checkbox("Enable Edit Visible Property", &Value))
					{
						bEnableEditVisibleProperty = Value;
						Owner->SaveConfig();
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		static UnrealImGui::FDetailsFilter DetailsFilter;
		DetailsFilter.Draw();
		if (SelectedObject)
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*SelectedObject->GetName()));
			TGuardValue<bool> GDisplayAllPropertiesGuard(UnrealImGui::GlobalValue::GDisplayAllProperties, bDisplayAllProperties);
			TGuardValue<bool> GEnableEditVisiblePropertyGuard(UnrealImGui::GlobalValue::GEnableEditVisibleProperty, bEnableEditVisibleProperty);
			UnrealImGui::DrawDetailTable("Details", SelectedObject->GetClass(), { SelectedObject }, &DetailsFilter);
		}
		else
		{
			ImGui::Text("Not select object");
		}

		ImGui::End();
	}
}

#undef LOCTEXT_NAMESPACE
