// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiHeatMap.h"


#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiStat.h"

namespace UnrealImGui
{
DECLARE_MEMORY_STAT(TEXT("UnrealImGuiHeatMap_UnitSize"), Stat_UnrealImGuiHeatMap_UnitSize, STATGROUP_ImGui);

template <typename T>
struct MaxIdx { static const unsigned int Value; };
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value   = 4294967295;

struct FRectInfo
{
	ImVec2 Min, Max;
	ImU32 Color[4];
};

template <typename TGetter>
struct RectRenderer
{
    RectRenderer(const TGetter& getter) :
        Getter(getter),
        Prims(Getter.Count)
    {}
    bool operator()(ImDrawList& DrawList, const ImRect& cull_rect, const ImVec2& uv, unsigned int prim) const
	{
        const FRectInfo rect = Getter(prim);
        const ImVec2 P1 = rect.Min;
        const ImVec2 P2 = rect.Max;

    	if (((rect.Color[0] | rect.Color[1] | rect.Color[2] | rect.Color[3]) & IM_COL32_A_MASK) == 0)
    	{
    		return false;
    	}

		if ((!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))))
            return false;

        DrawList._VtxWritePtr[0].pos   = P1;
        DrawList._VtxWritePtr[0].uv    = uv;
        DrawList._VtxWritePtr[0].col   = rect.Color[0];
        DrawList._VtxWritePtr[1].pos.x = P1.x;
        DrawList._VtxWritePtr[1].pos.y = P2.y;
        DrawList._VtxWritePtr[1].uv    = uv;
        DrawList._VtxWritePtr[1].col   = rect.Color[1];
        DrawList._VtxWritePtr[2].pos   = P2;
        DrawList._VtxWritePtr[2].uv    = uv;
        DrawList._VtxWritePtr[2].col   = rect.Color[2];
        DrawList._VtxWritePtr[3].pos.x = P2.x;
        DrawList._VtxWritePtr[3].pos.y = P1.y;
        DrawList._VtxWritePtr[3].uv    = uv;
        DrawList._VtxWritePtr[3].col   = rect.Color[3];
        DrawList._VtxWritePtr += 4;
        DrawList._IdxWritePtr[0] = DrawList._VtxCurrentIdx;
        DrawList._IdxWritePtr[1] = DrawList._VtxCurrentIdx + 1;
        DrawList._IdxWritePtr[2] = DrawList._VtxCurrentIdx + 3;
        DrawList._IdxWritePtr[3] = DrawList._VtxCurrentIdx + 1;
        DrawList._IdxWritePtr[4] = DrawList._VtxCurrentIdx + 2;
        DrawList._IdxWritePtr[5] = DrawList._VtxCurrentIdx + 3;
        DrawList._IdxWritePtr   += 6;
        DrawList._VtxCurrentIdx += 4;
        return true;
    }
    const TGetter& Getter;
    const unsigned int Prims;
    static constexpr int IdxConsumed = 6;
    static constexpr int VtxConsumed = 4;
};

template <typename Renderer>
void RenderPrimitives(const Renderer& renderer, ImDrawList& DrawList, const ImRect& cull_rect) {
	unsigned int prims        = renderer.Prims;
	unsigned int prims_culled = 0;
	unsigned int idx          = 0;
	const ImVec2 uv = DrawList._Data->TexUvWhitePixel;
	while (prims) {
		// find how many can be reserved up to end of current draw command's limit
		unsigned int cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - DrawList._VtxCurrentIdx) / Renderer::VtxConsumed);
		// make sure at least this many elements can be rendered to avoid situations where at the end of buffer this slow path is not taken all the time
		if (cnt >= ImMin(64u, prims)) {
			if (prims_culled >= cnt)
				prims_culled -= cnt; // reuse previous reservation
			else {
				DrawList.PrimReserve((cnt - prims_culled) * Renderer::IdxConsumed, (cnt - prims_culled) * Renderer::VtxConsumed); // add more elements to previous reservation
				prims_culled = 0;
			}
		}
		else
		{
			if (prims_culled > 0) {
				DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
				prims_culled = 0;
			}
			cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - 0/*DrawList._VtxCurrentIdx*/) / Renderer::VtxConsumed);
			DrawList.PrimReserve(cnt * Renderer::IdxConsumed, cnt * Renderer::VtxConsumed); // reserve new draw command
		}
		prims -= cnt;
		for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
			if (!renderer(DrawList, cull_rect, uv, idx))
				prims_culled++;
		}
	}
	if (prims_culled > 0)
		DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
}

constexpr ImU32 GetHeatColor(float T)
{
	// constexpr ImU32 Jet[] = {0xffaa0000, 0xffff0000, 0xffff5500, 0xffffaa00, 0xffffff00, 0xffaaff55, 0xff55ffaa, 0xff00ffff, 0xff00aaff, 0xff0055ff, 0xff0000ff };
	constexpr ImU32 JetWithAlpha[] = {0x00aa0000, 0x1aff0000, 0x33ff5500, 0x4dffaa00, 0x66ffff00, 0x80aaff55, 0x9955ffaa, 0xb300ffff, 0xcc00aaff, 0xff0055ff, 0xe60000ff };
	return JetWithAlpha[(int)((UE_ARRAY_COUNT(JetWithAlpha) - 1) * T + 0.5f)];
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

	struct FToDrawGrid
	{
		FGridLocation Location;
		const FGridBase* Grid;
	};
	TArray64<FToDrawGrid> ToDrawGrids;

	ImDrawList& DrawList = *ImGui::GetWindowDrawList();

	const FGridLocation GridMin = ToGridLocation(ViewBounds.Min);
	const FGridLocation GridMax = ToGridLocation(ViewBounds.Max);
	const FVector2D OffsetPixel = WorldToMapTransform.TransformPoint(ViewBounds.GetCenter()) - ViewBounds.GetCenter() * Zoom;

	for (int32 GridX = GridMin.X; GridX <= GridMax.X; ++GridX)
	{
		for (int32 GridY = GridMin.Y; GridY <= GridMax.Y; ++GridY)
		{
			if (const FGridBase* Grid = Grids.Find({ GridX, GridY }))
			{
				ToDrawGrids.Add({ FGridLocation{ GridX, GridY }, Grid });
			}
			else
			{
				DrawList.AddRectFilled({ (float)OffsetPixel.X + GridX * GridSize * Zoom, (float)OffsetPixel.Y + GridY * GridSize * Zoom },
						{(float)OffsetPixel.X + (GridX + 1) * GridSize * Zoom, (float)OffsetPixel.Y + (GridY + 1) * GridSize * Zoom }, GetHeatColor(0.f));
			}
		}
	}
	struct FUnitGetter
	{
		FUnitGetter(const FHeatMapBase& HeatMap, const TArray64<FToDrawGrid>& ToDrawGrids, int32 PreGridUnitCount, const float GridSize, const float UnitSize, const FVector2D& OffsetPixel)
			: HeatMap(HeatMap)
			, ToDrawGrids(ToDrawGrids)
			, PreGridUnitCount(PreGridUnitCount)
			, PreGridUnitCountSquared(PreGridUnitCount * PreGridUnitCount)
			, Count(ToDrawGrids.Num() * PreGridUnitCountSquared)
			, GridSize(GridSize)
			, UnitSize(UnitSize)
			, OffsetPixel(OffsetPixel.X, OffsetPixel.Y)
		{}

		const FHeatMapBase& HeatMap;
		const TArray64<FToDrawGrid>& ToDrawGrids;
		const uint32 PreGridUnitCount;
		const uint32 PreGridUnitCountSquared;
		const uint32 Count;
		const float GridSize;
		const float UnitSize;
		const FVector2f OffsetPixel;
		FRectInfo operator()(uint32 TotalUnitIndex) const
		{
			const uint32 GridIndex = TotalUnitIndex / PreGridUnitCountSquared;
			const uint32 UnitIndex = TotalUnitIndex % PreGridUnitCountSquared;
			const uint32 UnitX = UnitIndex % PreGridUnitCount;
			const uint32 UnitY = UnitIndex / PreGridUnitCount;
			const FToDrawGrid& ToDrawGrid = ToDrawGrids[GridIndex];
			const float T = HeatMap.GetUnitValueT_Impl((FUnit&)reinterpret_cast<const uint8*>(ToDrawGrid.Grid->Units)[UnitIndex * HeatMap.UnitTypeSize]);

			// TODO：处理跨Grid的Lerp
			auto GetUnitT = [this, &ToDrawGrid](uint32 UnitX, uint32 UnitY)
			{
				const int32 Index = UnitY * PreGridUnitCount + UnitX;
				return HeatMap.GetUnitValueT_Impl((FUnit&)reinterpret_cast<const uint8*>(ToDrawGrid.Grid->Units)[Index * HeatMap.UnitTypeSize]);
			};
			const float RightT = UnitX + 1 < PreGridUnitCount ? GetUnitT(UnitX + 1, UnitY) : T;
			const float TopT = UnitY >= 1 ? GetUnitT(UnitX, UnitY - 1) : T;
			const float TopRightT = UnitX + 1 < PreGridUnitCount && UnitY >= 1 ? GetUnitT(UnitX + 1, UnitY - 1) : T;

			const ImVec2 Min{ OffsetPixel.X + ToDrawGrid.Location.X * GridSize + UnitX * UnitSize, OffsetPixel.Y + ToDrawGrid.Location.Y * GridSize + UnitY * UnitSize };
			const ImVec2 Max{ Min.x + UnitSize, Min.y + UnitSize };
			return FRectInfo{ Min, Max, { GetHeatColor(TopT), GetHeatColor(T), GetHeatColor(RightT), GetHeatColor(TopRightT) } };
		}
	};
	const FUnitGetter UnitGetter{ *this, ToDrawGrids, PreGridUnitCount, GridSize * Zoom, UnitSize * Zoom, OffsetPixel };
	RenderPrimitives(RectRenderer{ UnitGetter }, DrawList, ImRect{ { (float)CullRect.Min.X, (float)CullRect.Min.Y }, { (float)CullRect.Max.X, (float)CullRect.Max.Y } });
}

void FHeatMapBase::StatMemoryInc(int64 Value)
{
	INC_MEMORY_STAT_BY(Stat_UnrealImGuiHeatMap_UnitSize, Value);
}
}

