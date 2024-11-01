// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiViewportBase.h"

#include "ImGuiEx.h"
#include "imgui_internal.h"
#include "UnrealImGuiViewportExtent.h"
#include "Engine/World.h"

namespace UnrealImGui::Viewport::EFilterType
{
	const FString TypeFilter_FilterType = TEXT("type");
}

UUnrealImGuiViewportBase::UUnrealImGuiViewportBase()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar;
}

void UUnrealImGuiViewportBase::Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	PanelBuilder = Builder;
	{
		// Only last left child class
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

		TArray<UClass*> ExtentClasses;
		GetDerivedClasses(UUnrealImGuiViewportExtentBase::StaticClass(), ExtentClasses);
		RemoveNotLeafClass(ExtentClasses);
		TSet<const UClass*> VisitedExtentClasses;
		for (UClass* Class : ExtentClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			bool bIsAlreadyInSet;
			VisitedExtentClasses.Add(Class, &bIsAlreadyInSet);
			if (bIsAlreadyInSet)
			{
				continue;
			}

			if (ShouldCreateExtent(Owner, Class) == false)
			{
				continue;
			}
			const UUnrealImGuiViewportExtentBase* CDO = Class->GetDefaultObject<UUnrealImGuiViewportExtentBase>();
			if (CDO->ShouldCreateExtent(Owner, this) == false)
			{
				continue;
			}

			UUnrealImGuiViewportExtentBase* Extent = NewObject<UUnrealImGuiViewportExtentBase>(this, Class, Class->GetFName(), RF_Transient);
			if (Extent->ExtentName == NAME_None)
			{
				Extent->ExtentName = Class->GetFName();
			}
			Extents.Add(Extent);
		}
		Extents.Sort([](const UUnrealImGuiViewportExtentBase& LHS, const UUnrealImGuiViewportExtentBase& RHS)
		{
			return LHS.Priority < RHS.Priority;
		});
	}
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		Extent->Register(Owner, this);
		if (Extent->bEnable)
		{
			Extent->WhenEnable(Owner, this);
		}
	}
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		PassDrawers.Add({ Extent, Extent->Priority, [Extent](UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
		{
			Extent->DrawViewportContent(Owner, ViewportContext);
		}});
		for (auto& PassDrawer : Extent->GetPassDrawers(Owner, this))
		{
			PassDrawers.Add({ Extent, PassDrawer.Priority, MoveTemp(PassDrawer.Drawer) });
		}
	}
	PassDrawers.Sort([](const FPassDrawer& LHS, const FPassDrawer& RHS)
	{
		return LHS.Priority < RHS.Priority;
	});

	CurrentViewLocation = ViewLocation;
	CurrentZoom = FMath::Pow(2.f, -ZoomFactor);
}

void UUnrealImGuiViewportBase::Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		if (Extent->bEnable)
		{
			Extent->WhenDisable(Owner, this);
		}
		Extent->Unregister(Owner, this);
	}
	PanelBuilder = nullptr;
}

void UUnrealImGuiViewportBase::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	const UWorld* World = Owner->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	auto Config = GetConfigObject<ThisClass>();

	ImGuiIO& IO = ImGui::GetIO();
	// Avoid left mouse drag window
	TGuardValue<bool> ConfigWindowsMoveFromTitleBarOnlyGuard{ IO.ConfigWindowsMoveFromTitleBarOnly, true };

	{
		float ViewMoveScale = 0.f;
		if (CurrentViewLocation != ViewLocation)
		{
			const FVector2D PrevViewLocation{ CurrentViewLocation };
			const float PrevDist = FVector2D::Distance(CurrentViewLocation, ViewLocation);
			CurrentViewLocation = FMath::Vector2DInterpTo(CurrentViewLocation, ViewLocation, DeltaSeconds, 10.f);
			const float MoveDist = FVector2D::Distance(PrevViewLocation, CurrentViewLocation);
			ViewMoveScale = MoveDist / PrevDist;
		}
		else
		{
			ViewMoveScale = 0.f;
		}
		if (ViewMoveScale > 0.f)
		{
			CurrentZoom = FMath::Pow(2.f, FMath::Lerp(FMath::LogX(2.f, CurrentZoom), -ZoomFactor, ViewMoveScale));
		}
		else
		{
			CurrentZoom = FMath::Pow(2.f, -ZoomFactor);
		}
	}

	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	const FVector2D ContentMin{ ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin() };
	const FVector2D ContentSize{ ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin() };
	const FVector2D ContentMax{ ContentMin + ContentSize };

	bool bIsConfigDirty = false;
	bool bIsMenuPopupState = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Viewport"))
		{
			DrawViewportMenu(Owner, bIsConfigDirty);
			for (int32 Idx = Extents.Num() - 1; Idx >= 0; --Idx)
			{
				UUnrealImGuiViewportExtentBase* Extent = Extents[Idx];
				if (Extent->bEnable == false)
				{
					continue;
				}
				bool bIsExtentConfigDirty = false;
				{
					ImGui::FIdScope IdScope{ Extent };
					Extent->DrawViewportMenu(Owner, bIsExtentConfigDirty);
				}
				if (bIsExtentConfigDirty)
				{
					Extent->SaveConfig();
				}
			}
			ImGui::Separator();
			ImGui::TextUnformatted("Zoom Speed: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4.f);
			if (ImGui::DragFloat("##ZoomFactorSpeed", &Config->ZoomFactorSpeed, 0.1f, 0.1f, 2.f))
			{
				SaveImGuiConfig();
			}
			if (ImGui::BeginMenu("View Frustum"))
			{
				if (ImGui::Checkbox("Draw View Frustum", &Config->bDrawViewFrustum))
				{
					SaveImGuiConfig();
				}
				ImGui::TextUnformatted("Near Plane Distance");
				if (ImGui::DragFloat("##NearPlaneDistance", &Config->NearPlaneDistance, 1, 0, Config->FarPlaneDistance))
				{
					Config->NearPlaneDistance = FMath::Min(Config->NearPlaneDistance, Config->FarPlaneDistance);
					SaveImGuiConfig();
				}
				ImGui::TextUnformatted("Far Plane Distance");
				if (ImGui::DragFloat("##FarPlaneDistance", &Config->FarPlaneDistance, 1, Config->NearPlaneDistance, FLT_MAX))
				{
					Config->FarPlaneDistance = FMath::Max(Config->FarPlaneDistance, Config->NearPlaneDistance);
					SaveImGuiConfig();
				}
				ImGui::EndMenu();
			}
			{
				ImGui::FDisabled Disabled{ World->ViewLocationsRenderedLastFrame.Num() == 0 };
				if (ImGui::Button("To View Location"))
				{
					if (World->ViewLocationsRenderedLastFrame.Num() > 0)
					{
						const FVector LastViewLocation{ World->ViewLocationsRenderedLastFrame[0] };
						ViewLocation = FVector2D{ LastViewLocation.X, LastViewLocation.Y };
					}
				}
			}
			if (ImGui::Button("To Origin Location"))
			{
				ViewLocation = FVector2D::ZeroVector;
			}
			ImGui::EndMenu();
		}
		DrawMenu(Owner, bIsConfigDirty);
		for (int32 Idx = Extents.Num() - 1; Idx >= 0; --Idx)
		{
			UUnrealImGuiViewportExtentBase* Extent = Extents[Idx];
			if (Extent->bEnable == false)
			{
				continue;
			}
			bool bIsExtentConfigDirty = false;
			{
				ImGui::FIdScope IdScope{ Extent };
				Extent->DrawMenu(Owner, bIsExtentConfigDirty);
			}
			if (bIsExtentConfigDirty)
			{
				Extent->SaveConfig();
			}
		}
		if (Extents.Num() > 0)
		{
			if (ImGui::BeginMenu("Extents"))
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					bool bEnable = Extent->bEnable;
					if (ImGui::Checkbox(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s##%s"), *Extent->ExtentName.ToString(), *Extent->GetClass()->GetName())), &bEnable))
					{
						Extent->bEnable = bEnable;
						if (bEnable)
						{
							Extent->WhenEnable(Owner, this);
						}
						else
						{
							Extent->WhenDisable(Owner, this);
						}
						Extent->SaveConfig();
					}
					if (ImGui::BeginItemTooltip())
					{
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Extent->GetClass()->GetName()));
						ImGui::EndTooltip();
					}
				}
				ImGui::EndMenu();
			}
		}

		{
			ImGui::Indent(ContentSize.X - ImGui::GetFontSize() * 11);
			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10);
			TArray<ANSICHAR, TInlineAllocator<256>> FilterArray;
			{
				const auto StringPoint = FTCHARToUTF8(*FilterString);
				FilterArray.SetNumZeroed(FMath::Max(256, StringPoint.Length() + 128));
				FMemory::Memcpy(FilterArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
			}
			if (ImGui::InputTextWithHint("##Filter", "Filter", FilterArray.GetData(), FilterArray.Num()))
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					if (Extent->bEnable == false)
					{
						continue;
					}
					FilterString = UTF8_TO_TCHAR(FilterArray.GetData());
					Extent->WhenFilterStringChanged(this, FilterString);
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
			{
				ImGui::BeginTooltip();
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					if (Extent->bEnable == false)
					{
						continue;
					}
					Extent->DrawFilterTooltip(this);
				}
				ImGui::EndTooltip();
			}

			static bool PreInputTextIsActive = false;
			const bool InputTextIsActive = ImGui::IsItemActive();
			const ImVec2 FilterBarPos = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					if (Extent->bEnable == false)
					{
						continue;
					}
					Extent->FocusEntitiesByFilter(this);
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("x##ClearFilter"))
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					if (Extent->bEnable == false)
					{
						continue;
					}
					FilterString.Reset();
					Extent->WhenFilterStringChanged(this, FilterString);
				}
			}
			if (PreInputTextIsActive || InputTextIsActive)
			{
				bIsMenuPopupState |= true;
				ImGui::SetNextWindowPos(FilterBarPos, ImGuiCond_Always);
				ImGui::SetNextWindowSize({200.f, 200.f});
				ImGui::OpenPopup("##FilterBar");
				if (ImGui::BeginPopup("##FilterBar", ImGuiWindowFlags_ChildWindow))
				{
					ImGui::PushTabStop(false);
					for (UUnrealImGuiViewportExtentBase* Extent : Extents)
					{
						if (Extent->bEnable == false)
						{
							continue;
						}
						Extent->DrawFilterPopup(this);
					}
					ImGui::PopTabStop();
					ImGui::EndPopup();
				}
			}
			PreInputTextIsActive = InputTextIsActive;
		}
		ImGui::EndMenuBar();
	}

	if (ContentSize.GetMin() > 0.f)
	{
		ImGui::InvisibleButton("Content", ImVec2{ ContentSize }, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	}
	ImGui::PushClipRect(ImVec2{ ContentMin }, ImVec2{ ContentMax }, true);
	const bool bIsContentHovered = ImGui::IsItemHovered();
	const bool bIsContentActive = ImGui::IsItemActive();
	const bool bIsMultiSelect = IO.KeyCtrl;
	const bool bIsSelectDragging = bIsContentActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
	bool bIsViewDragEnd = false;
	if (bIsContentHovered)
	{
		if (IO.MouseWheel != 0.f && bIsMenuPopupState == false)
		{
			const FVector2D ScreenMousePos{ ImGui::GetMousePos() };
			const FTransform2D PreZoomScreenToWorldTransform{ FTransform2D{ FScale2D{CurrentZoom}, -ViewLocation * CurrentZoom + ContentSize / 2.f + ContentMin }.Inverse() };
			const FVector2D MouseWorldPos{ PreZoomScreenToWorldTransform.TransformPoint(ScreenMousePos) };
			ZoomFactor = FMath::Clamp(ZoomFactor - IO.MouseWheel * Config->ZoomFactorSpeed, MinZoomFactor, MaxZoomFactor);
			const float Zoom = FMath::Pow(2.f, -ZoomFactor);
			const FTransform2D PostZoomWorldToScreenTransform{ FTransform2D{ FScale2D{Zoom}, -ViewLocation * Zoom + ContentSize / 2.f + ContentMin }.Inverse() };
			const FVector2D NextWorldPos{ PostZoomWorldToScreenTransform.TransformPoint(ScreenMousePos) };
			ViewLocation -= NextWorldPos - MouseWorldPos;
			CurrentZoom = Zoom;
			CurrentViewLocation = ViewLocation;
			bIsConfigDirty |= true;
		}
		static bool PreIsMouseDragging = false;
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			PreIsMouseDragging = true;
			ViewLocation -= FVector2D(IO.MouseDelta.x, IO.MouseDelta.y) / CurrentZoom;
			CurrentViewLocation = ViewLocation;
		}
		else if (PreIsMouseDragging)
		{
			bIsConfigDirty |= true;
			PreIsMouseDragging = false;
			bIsViewDragEnd = true;
		}
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			if (bIsMultiSelect == false)
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					if (Extent->bEnable == false)
					{
						continue;
					}
					Extent->ResetSelection();
				}
			}
		}
	}
	FUnrealImGuiViewportContext::FData ContextData;
	const FUnrealImGuiViewportContext Context
	{
		ContextData,
		this,
		DrawList,
		ContentMin,
		ContentSize,
		CurrentViewLocation,
		CurrentZoom,
		FVector2D{ ImGui::GetMousePos() },
		bIsContentHovered,
		bIsContentActive,
		bIsViewDragEnd,
		bIsSelectDragging,
		{ IO.MouseClickedPos[0].x, IO.MouseClickedPos[0].y },
		DeltaSeconds
	};

	constexpr int32 FloatContextFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	{
		ImGui::SetNextWindowPos(ImVec2{ ContentMin + 10.f }, ImGuiCond_Always);
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::SetNextWindowBgAlpha(0.5f);
		if (ImGui::Begin(Context.FloatingContextName, nullptr, FloatContextFlags))
		{
			// Construct
		}
		ImGui::End();
	}

	// Right click menu
	if (bIsViewDragEnd == false && ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Focus View To Here"))
		{
			ViewLocation = Context.ScreenToWorldLocation(FVector2D{ ImGui::GetWindowPos() });
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	DrawViewportContent(Owner, Context);
	for (const auto& Drawer : PassDrawers)
	{
		if (Drawer.Extent->bEnable == false)
		{
			continue;
		}
		FGuardValue_Bitfield(Context.Data->bIsConfigDirty, false);
		Drawer.Drawer(Owner, Context);
		if (Context.Data->bIsConfigDirty)
		{
			Drawer.Extent->SaveConfig();
		}
	}
	if (Config->bDrawViewFrustum)
	{
		DrawCurrentViewFrustum(Owner, Context);
	}

	// Draw drag area
	if (bIsSelectDragging)
	{
		constexpr ImU32 DragColor = IM_COL32(255, 255, 255, 51);
		DrawList->AddRectFilled(IO.MouseClickedPos[0], IO.MousePos, DragColor);
		DrawList->AddRect(IO.MouseClickedPos[0], IO.MousePos, DragColor);
	}

	// Draw scale
	{
		const ImU32 RatioColor = FUnrealImGuiViewportContext::FColorToU32(FColor::White);
		constexpr float Padding = 60.f;
		const float RealLength = Padding / 100.f / CurrentZoom;
		const ImVec2 Start{ (float)ContentMin.X + Padding, (float)ContentMax.Y - 30.f };
		const ImVec2 End{ Start.x + Padding, Start.y };

		DrawList->AddLine(Start, End, RatioColor);
		DrawList->AddLine({ Start.x, Start.y - 4.f }, Start, RatioColor);
		DrawList->AddLine({ End.x, End.y - 4.f }, End, RatioColor);
		DrawList->AddText({ Start.x + 4.f, Start.y + 2.f }, RatioColor, TCHAR_TO_UTF8(*FString::Printf(TEXT("%.1f m"), RealLength)));
	}

	// Draw axis
	{
		constexpr float Padding = 30.f;
		const ImVec2 Start{ (float)ContentMin.X + Padding, (float)ContentMax.Y - 50.f };
		DrawList->AddLine({ Start }, { Start.x + 20.f, Start.y }, FUnrealImGuiViewportContext::FColorToU32(FColor::Red));
		DrawList->AddLine({ Start }, { Start.x, Start.y + 20.f }, FUnrealImGuiViewportContext::FColorToU32(FColor::Green));
	}

	{
		DrawList->AddRect(ImVec2{ ContentMin }, ImVec2{ ContentMax }, IM_COL32_WHITE);
	}
	ImGui::PopClipRect();

	auto ViewportWindow = ImGui::GetCurrentWindow();
	if (ImGui::Begin(Context.FloatingContextName, nullptr, FloatContextFlags))
	{
		ImGui::BringWindowToDisplayBehind(ViewportWindow, ImGui::GetCurrentWindow());
		for (const auto& Message : Context.Data->Messages)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, Context.FColorToU32(Message.Color));
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*Message.Text));
			ImGui::PopStyleColor();
		}
	}
	ImGui::End();

	bIsConfigDirty |= Context.Data->bIsConfigDirty;
	if (bIsConfigDirty)
	{
		SaveConfig();
	}
}

void UUnrealImGuiViewportBase::DrawCurrentViewFrustum(UObject* Owner, const FUnrealImGuiViewportContext& Context)
{
	UWorld* World = GetWorld();
	if (World->CachedViewInfoRenderedLastFrame.Num())
	{
		const FWorldCachedViewInfo& ViewInfo = World->CachedViewInfoRenderedLastFrame[0];
		const auto InvTanHalfFov = ViewInfo.ProjectionMatrix.M[0][0];

		FTransform ViewTransform = FTransform{ ViewInfo.ViewToWorld };
		ViewTransform.ConcatenateRotation(FRotator{ 90.f, -90.f, 0.f }.Quaternion());

		const auto TanHalfFov = 1.0 / InvTanHalfFov;

		auto Config = GetConfigObject<ThisClass>();
		Context.DrawViewFrustum(ViewTransform, TanHalfFov, Config->NearPlaneDistance, Config->FarPlaneDistance, FColor{ 0, 127, 255, 127 });

		const FVector2D ViewLocation2D{ ViewTransform.GetLocation() };
		Context.DrawCircleFilled(ViewLocation2D, 3.f / CurrentZoom, FColor::White);
		Context.DrawText(ViewLocation2D + FVector2D{ -20.f / CurrentZoom, 6.f / CurrentZoom }, TEXT("World View"), FColor::White);
	}
}

UUnrealImGuiViewportExtentBase* UUnrealImGuiViewportBase::FindExtent(const TSubclassOf<UUnrealImGuiViewportExtentBase>& ExtentType) const
{
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		if (Extent->bEnable && Extent->IsA(ExtentType))
		{
			return Extent;
		}
	}
	return nullptr;
}

void UUnrealImGuiViewportBase::ResetSelection()
{
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		Extent->ResetSelection();
	}
}
