// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiViewportBase.h"

#include "imgui.h"

UUnrealImGuiViewportBase::UUnrealImGuiViewportBase()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
}

void UUnrealImGuiViewportBase::Register(UObject* Owner)
{
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
		for (const UClass* Class : ExtentClasses)
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

			const UUnrealImGuiViewportExtentBase* CDO = Class->GetDefaultObject<UUnrealImGuiViewportExtentBase>();
			if (CDO->ShouldCreateExtent(Owner, this) == false)
			{
				continue;
			}

			UUnrealImGuiViewportExtentBase* Extent = NewObject<UUnrealImGuiViewportExtentBase>(this, Class, Class->GetFName(), RF_Transient);
			Extents.Add(Extent);
		}
		Extents.Sort([](const UUnrealImGuiViewportExtentBase& LHS, const UUnrealImGuiViewportExtentBase& RHS)
		{
			return LHS.Priority > RHS.Priority;
		});
	}
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		Extent->Register(Owner, this);
	}
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		PassDrawers.Add({ Extent->Priority, [Extent](UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
		{
			Extent->DrawViewportContent(Owner, ViewportContext);
		}});
		PassDrawers.Append(Extent->GetPassDrawers(Owner, this));
	}
	PassDrawers.Sort([](const FPassDrawer& LHS, const FPassDrawer& RHS)
	{
		return LHS.Priority > RHS.Priority;
	});

	CurrentViewLocation = ViewLocation;
	CurrentZoom = FMath::Pow(2.f, -ZoomFactor);
}

void UUnrealImGuiViewportBase::Unregister(UObject* Owner)
{
	for (UUnrealImGuiViewportExtentBase* Extent : Extents)
	{
		Extent->Unregister(Owner, this);
	}
}

void UUnrealImGuiViewportBase::Draw(UObject* Owner, float DeltaSeconds)
{
	ImGuiIO& IO = ImGui::GetIO();
	// Avoid left mouse drag window
	TGuardValue<bool> ConfigWindowsMoveFromTitleBarOnlyGuard{ IO.ConfigWindowsMoveFromTitleBarOnly, true };

	CurrentViewLocation = FMath::Vector2DInterpTo(CurrentViewLocation, ViewLocation, DeltaSeconds, 10.f);
	CurrentZoom = FMath::FInterpTo(CurrentZoom, FMath::Pow(2.f, -ZoomFactor), DeltaSeconds, 10.f);

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
			ImGui::Separator();
			if (ImGui::Button("To View Location"))
			{
				if (const UWorld* World = Owner->GetWorld())
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
		for (UUnrealImGuiViewportExtentBase* Extent : Extents)
		{
			Extent->DrawViewportMenu(Owner, bIsConfigDirty);
		}

		{
			ImGui::Indent(ContentSize.X - 240.f);
			ImGui::SetNextItemWidth(200.f);
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
					Extent->WhenFilterStringChanged(UTF8_TO_TCHAR(FilterArray.GetData()));
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
			{
				ImGui::BeginTooltip();
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					Extent->DrawFilterTooltip();
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
					Extent->FocusEntitiesByFilter();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("x##ClearFilter"))
			{
				for (UUnrealImGuiViewportExtentBase* Extent : Extents)
				{
					Extent->WhenFilterStringChanged(TEXT(""));
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
					ImGui::PushAllowKeyboardFocus(false);
					for (UUnrealImGuiViewportExtentBase* Extent : Extents)
					{
						Extent->DrawFilterPopup();
					}
					ImGui::PopAllowKeyboardFocus();
					ImGui::EndPopup();
				}
			}
			PreInputTextIsActive = InputTextIsActive;
		}
		ImGui::EndMenuBar();
	}

	ImGui::InvisibleButton("Content", ImVec2{ ContentSize }, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
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
			ZoomFactor = FMath::Clamp(int32(ZoomFactor - IO.MouseWheel), 0, 10);
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
					Extent->ResetSelection();
				}
			}
		}
	}
	const FUnrealImGuiViewportContext Context
	{
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

	for (const auto& Drawer : PassDrawers)
	{
		Drawer.Drawer(Owner, Context);
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
		DrawList->AddRect(ImVec2{ ContentMin - 10.f }, ImVec2{ ContentMax + 10.f }, IM_COL32_BLACK, 0.f, ImDrawFlags_None, 20.f);
		DrawList->AddRect(ImVec2{ ContentMin }, ImVec2{ ContentMax }, IM_COL32_WHITE);
	}

	{
		ImGui::SetNextWindowPos(ImVec2{ ContentMin + 10.f }, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2{ MessageBoxWidth, -1.f });
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::SetNextWindowBgAlpha(0.5f);
		if (ImGui::Begin("MessageContent", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			for (const auto& Message : Context.Messages)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, Context.FColorToU32(Message.Color));
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*Message.Text));
				ImGui::PopStyleColor();
			}
		}
		ImGui::End();
	}

	bIsConfigDirty |= Context.bIsConfigDirty;
	if (bIsConfigDirty)
	{
		SaveConfig();
	}
}

ImU32 FUnrealImGuiViewportContext::FColorToU32(const FColor& Color)
{
	return IM_COL32(Color.R, Color.G, Color.B, Color.A);
}

void FUnrealImGuiViewportContext::DrawLine(const FVector2D& Start, const FVector2D& End, const FColor& Color, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D(TArray<FVector2D>{Start, End})))
	{
		DrawList->AddLine(ImVec2{ WorldToScreenLocation(Start) }, ImVec2{ WorldToScreenLocation(End) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawTriangle(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FColor& Color, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D{ { P1, P2, P3 } }))
	{
		DrawList->AddTriangle(ImVec2{ WorldToScreenLocation(P1) }, ImVec2{ WorldToScreenLocation(P2) }, ImVec2{ WorldToScreenLocation(P3) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawTriangleFilled(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FColor& Color) const
{
	if (ViewBounds.Intersect(FBox2D{ { P1, P2, P3 } }))
	{
		DrawList->AddTriangleFilled(ImVec2{ WorldToScreenLocation(P1) }, ImVec2{ WorldToScreenLocation(P2) }, ImVec2{ WorldToScreenLocation(P3) }, FColorToU32(Color));
	}
}

void FUnrealImGuiViewportContext::DrawRect(const FBox2D& Box, const FColor& Color, float Rounding, float Thickness) const
{
	if (ViewBounds.Intersect(Box) && Box.ExpandBy(Rounding).IsInside(ViewBounds) == false)
	{
		DrawList->AddRect(ImVec2{ WorldToScreenLocation(Box.Min) }, ImVec2{ WorldToScreenLocation(Box.Max) }, FColorToU32(Color), Rounding, ImDrawCornerFlags_All, Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawRectFilled(const FBox2D& Box, const FColor& Color, float Rounding) const
{
	if (ViewBounds.Intersect(Box))
	{
		DrawList->AddRectFilled(ImVec2{ WorldToScreenLocation(Box.Min) }, ImVec2{ WorldToScreenLocation(Box.Max) }, FColorToU32(Color), Rounding, ImDrawCornerFlags_All);
	}
}

void FUnrealImGuiViewportContext::DrawCircle(const FVector2D& Center, float Radius, const FColor& Color, int NumSegments, float Thickness) const
{
	const FBox2D CircleBounds{ FVector2D{Center} - FVector2D(Radius), FVector2D{Center} + FVector2D(Radius) };
	if (ViewBounds.Intersect(CircleBounds))
	{
		const float InnerRectRadius = Radius * 0.7071f;
		const FBox2D InnerRectBounds = FBox2D{ FVector2D{Center} - FVector2D(InnerRectRadius), FVector2D{Center} + FVector2D(InnerRectRadius) };

		if (InnerRectBounds.IsInside(ViewBounds) == false)
		{
			DrawList->AddCircle(ImVec2{ WorldToScreenLocation(Center) }, Radius * Zoom, FColorToU32(Color), NumSegments, Thickness);
		}
	}
}

void FUnrealImGuiViewportContext::DrawCircleFilled(const FVector2D& Center, float Radius, const FColor& Color, int NumSegments) const
{
	const FBox2D CircleBounds{ FVector2D{Center} - FVector2D(Radius), FVector2D{Center} + FVector2D(Radius) };
	if (ViewBounds.Intersect(CircleBounds))
	{
		DrawList->AddCircleFilled(ImVec2{ WorldToScreenLocation(Center) }, Radius * Zoom, FColorToU32(Color), NumSegments);
	}
}

void FUnrealImGuiViewportContext::DrawQuad(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, const FColor& Color, float Thickness) const
{
	const FBox2D QuadBounds{ { P1, P2, P3, P4 } };
	if (ViewBounds.Intersect(QuadBounds))
	{
		DrawList->AddQuad(ImVec2{ WorldToScreenLocation(P1) }, ImVec2{ WorldToScreenLocation(P2) }, ImVec2{ WorldToScreenLocation(P3) }, ImVec2{ WorldToScreenLocation(P4) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawQuadFilled(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, const FColor& Color) const
{
	const FBox2D QuadBounds{ { P1, P2, P3, P4 } };
	if (ViewBounds.Intersect(QuadBounds))
	{
		DrawList->AddQuadFilled(ImVec2{ WorldToScreenLocation(P1) }, ImVec2{ WorldToScreenLocation(P2) }, ImVec2{ WorldToScreenLocation(P3) }, ImVec2{ WorldToScreenLocation(P4) }, FColorToU32(Color));
	}
}

void FUnrealImGuiViewportContext::DrawText(const FVector2D& Position, const FString& Text, const FColor& Color) const
{
	if (ViewBounds.IsInside(Position))
	{
		DrawList->AddText(ImVec2{ WorldToScreenLocation(Position) }, FColorToU32(Color), TCHAR_TO_UTF8(*Text));
	}
}
