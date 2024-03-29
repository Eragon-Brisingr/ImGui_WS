// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiHeatMap.h"


#include "imgui.h"
#include "UnrealImGuiStat.h"

namespace UnrealImGui
{
DECLARE_MEMORY_STAT(TEXT("UnrealImGuiHeatMap_UnitSize"), Stat_UnrealImGuiHeatMap_UnitSize, STATGROUP_ImGui);

ImU32 GetHeatColor(float T)
{
	static TArray64<ImU32> HeatColor = []
	{
		TArray64<ImU32> ReturnColors;
		ReturnColors.SetNum(30);
		for (int32 Idx = 0; Idx < ReturnColors.Num(); ++Idx)
		{
			const float Weight = Idx / (ReturnColors.Num() - 1.f);
			const FLinearColor Color = FLinearColor{ (1.f - Weight) * 270.f, 1.f, 1.f, FMath::Pow(Weight, 0.5f) }.HSVToLinearRGB();
			ReturnColors[Idx] = ImGui::ColorConvertFloat4ToU32((ImVec4&)Color);
		}
		return ReturnColors;
	}();
	return HeatColor[static_cast<int32>((HeatColor.Num() - 1) * T + 0.5f)];
}

FHeatMapBase::~FHeatMapBase()
{
#if STATS
	const int64 ReleaseUnitSize = Grids.Num() * PreGridUnitCount * PreGridUnitCount * UnitTypeSize;
	DEC_MEMORY_STAT_BY(Stat_UnrealImGuiHeatMap_UnitSize, ReleaseUnitSize);
#endif
}

void FHeatMapBase::DrawHeatMap(const FBox2D& ViewBounds, const FBox2D& CullRect, const float Zoom, const FTransform2D& WorldToMapTransform) const
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiHeatMap_Draw"), STAT_UnrealImGuiHeatMap_Draw, STATGROUP_ImGui);

	ImDrawList& DrawList = *ImGui::GetWindowDrawList();

	const FGridLocation GridMin = ToGridLocation(ViewBounds.Min);
	const FGridLocation GridMax = ToGridLocation(ViewBounds.Max);
	const FVector2D OffsetPixel = WorldToMapTransform.TransformPoint(ViewBounds.GetCenter()) - ViewBounds.GetCenter() * Zoom;

	const float GridSizeZoomed = GridSize * Zoom;
	const float UnitSizeZoomed = UnitSize * Zoom;
	for (int32 GridX = GridMin.X; GridX <= GridMax.X; ++GridX)
	{
		for (int32 GridY = GridMin.Y; GridY <= GridMax.Y; ++GridY)
		{
			if (const FGridBase* Grid = Grids.Find({ GridX, GridY }))
			{
				for (int32 UnitX = 0; UnitX < PreGridUnitCount; ++UnitX)
				{
					for (int32 UnitY = 0; UnitY < PreGridUnitCount; ++UnitY)
					{
						// TODO：处理跨Grid的Lerp
						auto GetUnitT = [this, Grid](uint32 UnitX, uint32 UnitY)
						{
							const int32 Index = UnitY * PreGridUnitCount + UnitX;
							return GetUnitValueT_Impl((FUnit&)reinterpret_cast<const uint8*>(Grid->Units)[Index * UnitTypeSize]);
						};
						const float T = GetUnitT(UnitX, UnitY);
						const float RightT = UnitX + 1 < PreGridUnitCount ? GetUnitT(UnitX + 1, UnitY) : T;
						const float TopT = UnitY >= 1 ? GetUnitT(UnitX, UnitY - 1) : T;
						const float TopRightT = UnitX + 1 < PreGridUnitCount && UnitY >= 1 ? GetUnitT(UnitX + 1, UnitY - 1) : T;

						const ImVec2 Min{ float(OffsetPixel.X + GridX * GridSizeZoomed + UnitX * UnitSizeZoomed), float(OffsetPixel.Y + GridY * GridSizeZoomed + UnitY * UnitSizeZoomed) };
						const ImVec2 Max{ Min.x + UnitSizeZoomed, Min.y + UnitSizeZoomed };
						DrawList.AddRectFilledMultiColor(Min, Max, GetHeatColor(TopT), GetHeatColor(TopRightT), GetHeatColor(RightT), GetHeatColor(T));
					}
				}
			}
			else
			{
				DrawList.AddRectFilled({ (float)OffsetPixel.X + GridX * GridSizeZoomed, (float)OffsetPixel.Y + GridY * GridSizeZoomed },
						{(float)OffsetPixel.X + (GridX + 1) * GridSizeZoomed, (float)OffsetPixel.Y + (GridY + 1) * GridSizeZoomed }, GetHeatColor(0.f));
			}
		}
	}
}

void FHeatMapBase::StatMemoryInc(int64 Value)
{
	INC_MEMORY_STAT_BY(Stat_UnrealImGuiHeatMap_UnitSize, Value);
}
}

