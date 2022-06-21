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
		FHeatMapBase(const float UnitSize, const int32 PreCellUnitCount, const uint32 UnitTypeSize)
			: UnitSize(UnitSize)
			, PreCellUnitCount(PreCellUnitCount)
			, CellSize(UnitSize * PreCellUnitCount)
			, UnitTypeSize(UnitTypeSize)
		{}
		virtual ~FHeatMapBase();

		const float UnitSize;
		const int32 PreCellUnitCount;
		const float CellSize;
		const uint32 UnitTypeSize;
		using FUnit = FHeatMapUnit;
		struct FCell
		{
			~FCell()
			{
				delete[] Units;
			}
			FUnit* Units = nullptr;
		};
		FIntVector2 ToCellLocation(const FVector2D& WorldLocation) const { return { FMath::FloorToInt(WorldLocation.X / CellSize), FMath::FloorToInt(WorldLocation.Y / CellSize) }; }

		void DrawHeatMap(const FBox2D& ViewBounds, const FBox2D& CullRect, const float Zoom, const FTransform2D& WorldToMapTransform) const;
	protected:
		using KeyType = FIntVector2;
		using ValueType = FCell;
		struct FGridKeyFunc : BaseKeyFuncs<TPair<KeyType, ValueType>, KeyType, false>
		{
			typedef typename TTypeTraits<KeyType>::ConstPointerType KeyInitType;
			typedef const TPairInitializer<typename TTypeTraits<KeyType>::ConstInitType, typename TTypeTraits<ValueType>::ConstInitType>& ElementInitType;

			static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
			{
				return Element.Key;
			}

			static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
			{
				return A == B;
			}

			template<typename ComparableKey>
			static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
			{
				return A == B;
			}

			static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
			{
				return HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y));
			}

			template<typename ComparableKey>
			static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
			{
				return GetTypeHash(Key);
			}
		};
		TMap<KeyType, ValueType, FDefaultSetAllocator, FGridKeyFunc> Cells;

		virtual float GetUnitValueT_Impl(const FUnit& Unit) const = 0;

		static void StatMemoryInc(int64 Value);
	};

	template<typename TUnit, typename TVisitor>
	class THeatMap : public FHeatMapBase
	{
		static_assert(TIsDerivedFrom<TUnit, FUnit>::Value);
	public:
		THeatMap(const float UnitSize, const int32 PreCellUnitCount)
			: FHeatMapBase(UnitSize, PreCellUnitCount, sizeof(TUnit))
		{}
		TUnit& AddOrFindValue(const FVector2D& WorldLocation)
		{
			FCell& Cell = Cells.FindOrAdd(ToCellLocation(WorldLocation));
			if (Cell.Units == nullptr)
			{
				StatMemoryInc(PreCellUnitCount * PreCellUnitCount * sizeof(TUnit));
				Cell.Units = new TUnit[PreCellUnitCount * PreCellUnitCount]{};
			}
			static auto FMod = [](double X, float Y)
			{
				if (X > 0)
				{
					return FMath::Fmod(X, Y);
				}
				return FMath::Fmod(X, Y) + Y;
			};
			const int32 UnitX = FMath::FloorToInt(FMod(WorldLocation.X, CellSize) / UnitSize);
			const int32 UnitY = FMath::FloorToInt(FMod(WorldLocation.Y, CellSize) / UnitSize);
			const int32 Index = UnitY * PreCellUnitCount + UnitX;
			return static_cast<TUnit*>(Cell.Units)[Index];
		}

		template<typename TFunc>
		void ForEachUnit(TFunc Func)
		{
			for (auto& [_, Cell] : Cells)
			{
				for (int32 Idx = 0; Idx < PreCellUnitCount * PreCellUnitCount; ++Idx)
				{
					Func(static_cast<TUnit*>(Cell.Units)[Idx]);
				}
			}
		}
	private:
		float GetUnitValueT_Impl(const FUnit& Unit) const override final
		{
			return TVisitor::GetUnitValueT((const TUnit&)Unit);
		}
	};
}
