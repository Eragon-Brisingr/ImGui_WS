// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiViewportExtent.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiViewportBase.h"

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

void FUnrealImGuiViewportContext::DrawDashedLine(const FVector2D& Start, const FVector2D& End, const FColor& Color, float DashSize, float Thickness) const
{
	if (!ViewBounds.Intersect(FBox2D(TArray<FVector2D>{Start, End})))
	{
		return;
	}

	FVector2D LineDir = End - Start;
	double LineLeft = (End - Start).Size();
	if (LineLeft)
	{
		LineDir /= LineLeft;
	}

	const FVector2D Dash = (DashSize * LineDir);
	FVector2D DrawStart = Start;
	while (LineLeft > DashSize)
	{
		const FVector2D DrawEnd = DrawStart + Dash;

		DrawList->AddLine(ImVec2{ WorldToScreenLocation(Start) }, ImVec2{ WorldToScreenLocation(End) }, FColorToU32(Color), Thickness);

		LineLeft -= 2*DashSize;
		DrawStart = DrawEnd + Dash;
	}
	if (LineLeft > 0.0f)
	{
		DrawList->AddLine(ImVec2{ WorldToScreenLocation(Start) }, ImVec2{ WorldToScreenLocation(End) }, FColorToU32(Color), Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawArc(const FVector2D& Center, float MinAngle, float MaxAngle, double Radius, const FColor& Color, int NumSegments, float Thickness) const
{
	const FBox2D CircleBounds{ Center - Radius, Center + Radius };
	if (ViewBounds.Intersect(CircleBounds))
	{
		MinAngle = FMath::DegreesToRadians(MinAngle);
		MaxAngle = FMath::DegreesToRadians(MaxAngle);
		DrawList->PathArcTo(ImVec2{ WorldToScreenLocation(Center) }, Radius, MinAngle, MaxAngle, NumSegments);
		DrawList->PathStroke(FColorToU32(Color), ImDrawFlags_None, Thickness);
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
	const FBox2D CircleBounds{ Center - Radius, Center + Radius };
	if (ViewBounds.Intersect(CircleBounds))
	{
		const float InnerRectRadius = Radius * 0.7071f;
		const FBox2D InnerRectBounds = FBox2D{ Center - InnerRectRadius, Center + InnerRectRadius };
		if (InnerRectBounds.IsInside(ViewBounds) == false)
		{
			DrawList->AddCircle(ImVec2{ WorldToScreenLocation(Center) }, Radius * Zoom, FColorToU32(Color), NumSegments, Thickness);
		}
	}
}

void FUnrealImGuiViewportContext::DrawCircleFilled(const FVector2D& Center, float Radius, const FColor& Color, int NumSegments) const
{
	const FBox2D CircleBounds{ Center - Radius, Center + Radius };
	if (ViewBounds.Intersect(CircleBounds))
	{
		DrawList->AddCircleFilled(ImVec2{ WorldToScreenLocation(Center) }, Radius * Zoom, FColorToU32(Color), NumSegments);
	}
}

namespace ImGuiExpand
{
	void AddTaperedCapsule(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius_a, float radius_b, ImU32 color, int num_segments = 0, float thickness = 1.0f)
	{
		const ImVec2 diff{ b.x - a.x, b.y - a.y };
		const float angle = atan2(diff.y, diff.x) + IM_PI * 0.5f;
		draw_list->PathArcTo(a, radius_a, angle, angle + IM_PI, num_segments);
		draw_list->PathArcTo(b, radius_b, angle + IM_PI, angle + IM_PI * 2.f, num_segments);
		draw_list->PathStroke(color, ImDrawFlags_Closed, thickness);
	}

	void AddCapsule(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius, ImU32 color, int num_segments = 0, float thickness = 1.0f)
	{
		AddTaperedCapsule(draw_list, a, b, radius, radius, color, num_segments, thickness);
	}

	void AddTaperedCapsuleFilled(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius_a, float radius_b, ImU32 color, int num_segments = 0)
	{
		const ImVec2 diff{ b.x - a.x, b.y - a.y };
		const float angle = atan2(diff.y, diff.x) + IM_PI * 0.5f;
		draw_list->PathArcTo(a, radius_a, angle, angle + IM_PI, num_segments);
		draw_list->PathArcTo(b, radius_b, angle + IM_PI, angle + IM_PI * 2.f, num_segments);
		draw_list->PathFillConvex(color);
	}

	void AddCapsuleFilled(ImDrawList* draw_list, ImVec2 a, ImVec2 b, float radius, ImU32 color, int num_segments = 0)
	{
		AddTaperedCapsuleFilled(draw_list, a, b, radius, radius, color, num_segments);
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

void FUnrealImGuiViewportContext::DrawTaperedCapsule(const FVector2D& A, const FVector2D& B, float RadiusA, float RadiusB, const FColor& Color, int NumSegments, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B } }.ExpandBy(FMath::Max(RadiusA, RadiusB))))
	{
		ImGuiExpand::AddTaperedCapsule(DrawList, ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, RadiusA * Zoom, RadiusB * Zoom, FColorToU32(Color), NumSegments, Thickness);
	}
}

void FUnrealImGuiViewportContext::DrawTaperedCapsuleFilled(const FVector2D& A, const FVector2D& B, float RadiusA, float RadiusB, const FColor& Color, int NumSegments) const
{
	if (ViewBounds.Intersect(FBox2D{ { A, B } }.ExpandBy(FMath::Max(RadiusA, RadiusB))))
	{
		ImGuiExpand::AddTaperedCapsuleFilled(DrawList, ImVec2{ WorldToScreenLocation(A) }, ImVec2{ WorldToScreenLocation(B) }, RadiusA * Zoom, RadiusB * Zoom, FColorToU32(Color), NumSegments);
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

void FUnrealImGuiViewportContext::DrawCoordinateSystem(const FTransform& Transform, float Scale, float Thickness) const
{
	const FVector2D X{ Transform.GetScaledAxis(EAxis::X) * Scale };
	const FVector2D Y{ Transform.GetScaledAxis(EAxis::Y) * Scale };
	const FVector2D Z{ Transform.GetScaledAxis(EAxis::Z) * Scale };

	FBox2D Bounds;
	const FVector2D Location{ Transform.GetLocation() };
	Bounds += Location;
	Bounds += X;
	Bounds += Y;
	Bounds += Z;
	if (ViewBounds.Intersect(Bounds))
	{
		if (!X.IsNearlyZero())
		{
			DrawList->AddLine(ImVec2{ WorldToScreenLocation(Location) }, ImVec2{ WorldToScreenLocation(Location + X) }, FColorToU32(FColor::Red), Thickness);
		}
		if (!Y.IsNearlyZero())
		{
			DrawList->AddLine(ImVec2{ WorldToScreenLocation(Location) }, ImVec2{ WorldToScreenLocation(Location + Y) }, FColorToU32(FColor::Green), Thickness);
		}
		if (!Z.IsNearlyZero())
		{
			DrawList->AddLine(ImVec2{ WorldToScreenLocation(Location) }, ImVec2{ WorldToScreenLocation(Location + Z) }, FColorToU32(FColor::Blue), Thickness);
		}
	}
}

void FUnrealImGuiViewportContext::DrawText(const FVector2D& Position, const FString& Text, const FColor& Color) const
{
	if (ViewBounds.IsInside(Position))
	{
		DrawList->AddText(ImVec2{ WorldToScreenLocation(Position) }, FColorToU32(Color), TCHAR_TO_UTF8(*Text));
	}
}
