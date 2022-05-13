// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiObjectBrowser.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiPropertyDetails.h"
#include "UnrealImGuiUtils.h"
#include "UnrealImGuiWrapper.h"

void FUnrealImGuiObjectBrowser::Draw(UObject* Owner)
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
	}
	ImGui::EndChild();

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
		const bool bInvokeSearch = UnrealImGui::InputTextWithHint("##Filter", "Filer", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
		if (bInvokeSearch)
		{
			SelectedObject = FindObject<UObject>(nullptr, *FilterString.ToString());
			FilterString.Empty();
		}
		ImGui::Separator();
		
		const FString Filter = FilterString.ToString();
		if (ImGui::BeginChild("Content"))
		{
			if (SelectedObject == nullptr)
			{
				TArray<UObject*> Objects;
				GetObjectsOfClass(UPackage::StaticClass(), Objects);
				if (Filter.Len() > 0)
				{
					Objects.RemoveAllSwap([&Filter](const UObject* E){ return E->GetName().Contains(Filter) == false; });
				}

				ImGuiListClipper ListClipper{ Objects.Num() };
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						UObject* Object = Objects[Idx];
						if (ImGui::Button(TCHAR_TO_UTF8(*Object->GetName())))
						{
							SelectedObject = Object;
						}
					}
				}
			}
			else
			{
				TArray<UObject*, TInlineAllocator<256>> SubObjects;
				ForEachObjectWithOuter(SelectedObject, [&SubObjects, &Filter](UObject* SubObject)
				{
					if (Filter.Len() > 0 && SubObject->GetName().Contains(Filter) == false)
					{
						return;
					}
					SubObjects.Add(SubObject);
				}, false);
	
				ImGuiListClipper ListClipper{ SubObjects.Num() };
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						UObject* Object = SubObjects[Idx];
						if (ImGui::Button(TCHAR_TO_UTF8(*Object->GetName())))
						{
							SelectedObject = Object;
						}
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	ImGui::SetNextWindowClass(WindowClass);
	if (ImGui::Begin("ObjectBrowserDetails"))
	{
		if (SelectedObject)
		{
			ImGui::Text(TCHAR_TO_UTF8(*SelectedObject->GetName()));
			UnrealImGui::DrawDetailTable("Details", SelectedObject->GetClass(), { SelectedObject });
		}
		else
		{
			ImGui::Text("Not select object");
		}

		ImGui::End();
	}
}
