// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiViewportExtent.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiViewportBase.h"

ImU32 FUnrealImGuiViewportContext::FColorToU32(const FColor& Color)
{
	return IM_COL32(Color.R, Color.G, Color.B, Color.A);
}

UUnrealImGuiViewportBase* UUnrealImGuiViewportExtentBase::GetViewport() const
{
	return CastChecked<UUnrealImGuiViewportBase>(GetOuter());
}

void FUnrealImGuiViewportContext::DrawLine(const FVector2D& Start, const FVector2D& End, const FColor& Color, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D(TArray<FVector2D>{Start, End})))
	{
		DrawList->AddLine(ImVec2{ WorldToScreenLocation(Start) }, ImVec2{ WorldToScreenLocation(End) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawArrow(const FVector2D& Start, const FVector2D& End, const FColor& Color, float ArrowSize, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D(TArray<FVector2D>{Start, End})))
	{
		const FVector2D ScreenEnd{ WorldToScreenLocation(End) };
		DrawList->AddLine(ImVec2{ WorldToScreenLocation(Start) }, ImVec2{ ScreenEnd }, FColorToU32(Color), Thickness);
		const FVector2D ArrowVec{ (Start - End).GetSafeNormal() * ArrowSize };
		DrawList->AddLine(ImVec2{ ScreenEnd }, ImVec2{ ScreenEnd + ArrowVec.GetRotated(-22.5f) }, FColorToU32(Color), Thickness);
		DrawList->AddLine(ImVec2{ ScreenEnd }, ImVec2{ ScreenEnd + ArrowVec.GetRotated(22.5f) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawTriangle(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FColor& Color, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B, C } }))
	{
		DrawList->AddTriangle(ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, ImVec2{ WorldToScreenLocation(C) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawTriangleFilled(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FColor& Color) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B, C } }))
	{
		DrawList->AddTriangleFilled(ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, ImVec2{ WorldToScreenLocation(C) }, FColorToU32(Color));
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

namespace ImGuiExpand
{
	void AddCapsule(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius, ImU32 color, int num_segments = 0, float thickness = 1.0f)
	{
		const ImVec2 diff{ (b - a) };
		const float angle = atan2(diff.y, diff.x) + IM_PI * 0.5f;
		draw_list->PathArcTo(a, radius, angle, angle + IM_PI, num_segments);
		draw_list->PathArcTo(b, radius, angle + IM_PI, angle + IM_PI * 2.f, num_segments);
		draw_list->PathStroke(color, ImDrawFlags_Closed, thickness);
	}

	void AddCapsuleFilled(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius, ImU32 color, int num_segments = 0)
	{
		const ImVec2 diff{ (b - a) };
		const float angle = atan2(diff.y, diff.x) + IM_PI * 0.5f;
		draw_list->PathArcTo(a, radius, angle, angle + IM_PI, num_segments);
		draw_list->PathArcTo(b, radius, angle + IM_PI, angle + IM_PI * 2.f, num_segments);
		draw_list->PathFillConvex(color);
	}
}

void FUnrealImGuiViewportContext::DrawCapsule(const FVector2D& A, const FVector2D& B, float Radius, const FColor& Color, int NumSegments, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B } }.ExpandBy(Radius)))
	{
		ImGuiExpand::AddCapsule(DrawList, ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, Radius * Zoom, FColorToU32(Color), NumSegments, Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawCapsuleFilled(const FVector2D& A, const FVector2D& B, float Radius, const FColor& Color, int NumSegments) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B } }.ExpandBy(Radius)))
	{
		ImGuiExpand::AddCapsuleFilled(DrawList, ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, Radius * Zoom, FColorToU32(Color), NumSegments);
	}
}

void FUnrealImGuiViewportContext::DrawRect(const FBox2D& Box, const FColor& Color, float Rounding, float Thickness) const
{
	if (ViewBounds.Intersect(Box) && Box.ExpandBy(Box.GetSize() * -Rounding).IsInside(ViewBounds) == false)
	{
		DrawList->AddRect(ImVec2{ WorldToScreenLocation(Box.Min) }, ImVec2{ WorldToScreenLocation(Box.Max) }, FColorToU32(Color), Rounding, ImDrawFlags_RoundCornersAll, Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawRectFilled(const FBox2D& Box, const FColor& Color, float Rounding) const
{
	if (ViewBounds.Intersect(Box))
	{
		DrawList->AddRectFilled(ImVec2{ WorldToScreenLocation(Box.Min) }, ImVec2{ WorldToScreenLocation(Box.Max) }, FColorToU32(Color), Rounding, ImDrawFlags_RoundCornersAll);
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
