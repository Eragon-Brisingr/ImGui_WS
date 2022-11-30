// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	struct FHeatMapUnit
	{
		virtual ~FHeatMapUnit() {}
	};
	class IMGUI_API FHeatMapBase
	{
	public:
		FHeatMapBase(const float UnitSize, const int32 PreGridUnitCount, const uint32 UnitTypeSize)
			: UnitSize(UnitSize)
			, PreGridUnitCount(PreGridUnitCount)
			, GridSize(UnitSize * PreGridUnitCount)
			, UnitTypeSize(UnitTypeSize)
		{}
		virtual ~FHeatMapBase();

		using FGridLocation = FIntPoint;
		using FUnitLocation = FIntVector2;

		const float UnitSize;
		const int32 PreGridUnitCount;
		const float GridSize;
		const uint32 UnitTypeSize;
		using FUnit = FHeatMapUnit;
		FGridLocation ToGridLocation(const FVector2D& WorldLocation) const { return { FMath::FloorToInt((float)WorldLocation.X / GridSize), FMath::FloorToInt((float)WorldLocation.Y / GridSize) }; }
		FUnitLocation ToUnitLocation(const FGridLocation& GridLocation, const FVector2D& WorldLocation) const
		{
			const int32 UnitX = FMath::FloorToInt((WorldLocation.X - GridLocation.X * GridSize) / UnitSize);
			const int32 UnitY = FMath::FloorToInt((WorldLocation.Y - GridLocation.Y * GridSize) / UnitSize);
			return { UnitX, UnitY };
		}
		FVector2D ToWorldLocation(const FGridLocation& GridLocation, const FUnitLocation& UnitLocation) const { return { GridLocation.X * GridSize + UnitLocation.X * UnitSize, GridLocation.Y * GridSize + UnitLocation.Y * UnitSize }; }

		void DrawHeatMap(const FBox2D& ViewBounds, const FBox2D& CullRect, const float Zoom, const FTransform2D& WorldToMapTransform) const;
	protected:
		struct FGridBase
		{
			~FGridBase()
			{
				delete[] Units;
			}
			FUnit* Units = nullptr;
		};
		TMap<FGridLocation, FGridBase> Grids;

		virtual float GetUnitValueT_Impl(const FUnit& Unit) const = 0;

		static void StatMemoryInc(int64 Value);
	};

	template<typename TUnit, typename THeatMapFinalType>
	class THeatMap : public FHeatMapBase
	{
		static_assert(TIsDerivedFrom<TUnit, FUnit>::Value);
	public:
		THeatMap(const float UnitSize, const int32 PreGridUnitCount)
			: FHeatMapBase(UnitSize, PreGridUnitCount, sizeof(TUnit))
		{}
		struct FGrid : FGridBase
		{
			TUnit& GetUnit(int32 Index) { return static_cast<TUnit*>(Units)[Index]; }
		};

		FGrid& AddOrFindGrid(const FGridLocation& GridLocation)
		{
			FGridBase& Grid = Grids.FindOrAdd(GridLocation);
			if (Grid.Units == nullptr)
			{
				StatMemoryInc(PreGridUnitCount * PreGridUnitCount * sizeof(TUnit));
				Grid.Units = new TUnit[PreGridUnitCount * PreGridUnitCount]{};
			}
			return static_cast<FGrid&>(Grid);
		}
		TUnit& GetUnit(FGrid& Grid, const FGridLocation GridLocation, const FVector2D& WorldLocation) const
		{
			const FUnitLocation UnitLocation = ToUnitLocation(GridLocation, WorldLocation);
			const int32 Index = UnitLocation.Y * PreGridUnitCount + UnitLocation.X;
			check(Index >= 0 && Index < PreGridUnitCount * PreGridUnitCount);
			return Grid.GetUnit(Index);
		}
		TUnit& AddOrFindUnit(const FVector2D& WorldLocation)
		{
			const FGridLocation GridLocation = ToGridLocation(WorldLocation);
			FGrid& Grid = AddOrFindGrid(GridLocation);
			return GetUnit(Grid, GridLocation, WorldLocation);
		}
		template<bool bAddGridWhenNotFind = true, typename TFunc>
		void ForEachUnit(const FBox2D& Bounds, TFunc Func)
		{
			const FGridLocation GridMin = ToGridLocation(Bounds.Min);
			const FGridLocation GridMax = ToGridLocation(Bounds.Max);
			for (int32 GridX = GridMin.X; GridX <= GridMax.X; ++GridX)
			{
				for (int32 GridY = GridMin.Y; GridY <= GridMax.Y; ++GridY)
				{
					FGrid* Grid;
					if constexpr (bAddGridWhenNotFind)
					{
						Grid = &AddOrFindGrid(FGridLocation{GridX, GridY});
					}
					else
					{
						Grid = static_cast<FGrid*>(Grids.Find({GridX, GridY}));
						if (Grid == nullptr)
						{
							continue;
						}
					}

					const FVector2D Min{ FMath::Max(GridX * GridSize, Bounds.Min.X), FMath::Max(GridY * GridSize, Bounds.Min.Y) };
					const FVector2D Max{ FMath::Min((GridX + 1) * GridSize, Bounds.Max.X), FMath::Min((GridY + 1) * GridSize, Bounds.Max.Y) };
					const FUnitLocation UnitMin = ToUnitLocation({ GridX, GridY }, Min);
					const FUnitLocation UnitMax = ToUnitLocation({ GridX, GridY }, Max);
					for (int32 UnitX = UnitMin.X; UnitX < UnitMax.X; ++UnitX)
					{
						for (int32 UnitY = UnitMin.Y; UnitY < UnitMax.Y; ++UnitY)
						{
							const int32 Index = UnitY * PreGridUnitCount + UnitX;
							check(Index >= 0 && Index < PreGridUnitCount * PreGridUnitCount);
							Func(Grid->GetUnit(Index), FGridLocation{ GridX, GridY }, FUnitLocation{ UnitX, UnitY });
						}
					}
				}
			}
		}
		template<bool bAddGridWhenNotFind = true, typename TFunc>
		void ForEachUnit(const FSphere& Bounds, TFunc Func)
		{
			const FVector2D BoundsMin{ Bounds.Center.X - Bounds.W, Bounds.Center.Y - Bounds.W };
			const FVector2D BoundsMax{ Bounds.Center.X + Bounds.W, Bounds.Center.Y + Bounds.W };
			ForEachUnit<bAddGridWhenNotFind>(FBox2D{ BoundsMin, BoundsMax }, [&](TUnit& Unit, const FGridLocation& GridLocation, const FUnitLocation& UnitLocation)
			{
				const FVector2D UnitWorldLocation = ToWorldLocation(GridLocation, UnitLocation);
				if (Bounds.IsInside(FVector{ UnitWorldLocation.X, UnitWorldLocation.Y, Bounds.Center.Z }))
				{
					Func(Unit, GridLocation, UnitLocation);
				}
			});
		}

		template<typename TFunc>
		void ForEachUnit(TFunc Func)
		{
			for (auto& [_, Grid] : Grids)
			{
				for (int32 Idx = 0; Idx < PreGridUnitCount * PreGridUnitCount; ++Idx)
				{
					Func(static_cast<FGrid&>(Grid).GetUnit(Idx));
				}
			}
		}
	private:
		float GetUnitValueT_Impl(const FUnit& Unit) const override final
		{
			return static_cast<const THeatMapFinalType*>(this)->GetUnitValueT(static_cast<const TUnit&>(Unit));
		}
	};
}
