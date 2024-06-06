// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiViewportExtentImpl.h"

#include "imgui.h"
#include "NavigationSystem.h"
#include "UnrealImGuiViewportBase.h"
#include "Detour/DetourNavMesh.h"
#include "NavMesh/RecastHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "WorldPartition/WorldPartitionRuntimeSpatialHash.h"

UUnrealImGuiViewportWorldPartitionExtent::UUnrealImGuiViewportWorldPartitionExtent()
{
	ExtentName = TEXT("World Partition");
	Priority = -90;
}

void UUnrealImGuiViewportWorldPartitionExtent::DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty)
{
	const UWorld* World = Owner->GetWorld();
	const TArray<FSpatialHashStreamingGrid>* StreamingGridsPtr = nullptr;
	if (const UWorldPartition* WorldPartition = World->GetWorldPartition())
	{
		const UWorldPartitionRuntimeSpatialHash* RuntimeHash = Cast<UWorldPartitionRuntimeSpatialHash>(WorldPartition->RuntimeHash);
		if (RuntimeHash)
		{
			static FProperty* StreamingGridsProperty = FindFProperty<FProperty>(RuntimeHash->GetClass(), TEXT("StreamingGrids"));
			StreamingGridsPtr = StreamingGridsProperty->ContainerPtrToValuePtr<TArray<FSpatialHashStreamingGrid>>(RuntimeHash);
		}
	}
	if (StreamingGridsPtr)
	{
		if (ImGui::BeginMenu("World Partition"))
		{
			if (ImGui::RadioButton("None", WorldPartitionGridIndex == INDEX_NONE))
			{
				WorldPartitionGridIndex = INDEX_NONE;
				bIsConfigDirty |= true;
			}
			const TArray<FSpatialHashStreamingGrid>& StreamingGrids = *StreamingGridsPtr;
			for (int32 Idx = 0; Idx < StreamingGrids.Num(); ++Idx)
			{
				const FSpatialHashStreamingGrid& Grid = StreamingGrids[Idx];
				if (ImGui::RadioButton(TCHAR_TO_UTF8(*Grid.GridName.ToString()), WorldPartitionGridIndex == Idx))
				{
					WorldPartitionGridIndex = Idx;
					bIsConfigDirty |= true;
				}
			}
			if (StreamingGrids.IsValidIndex(WorldPartitionGridIndex))
			{
				const TArray<FSpatialHashStreamingGridLevel>& Levels = StreamingGrids[WorldPartitionGridIndex].GridLevels;
				ImGui::Text("Level Range");
				if (ImGui::DragIntRange2("##Level Range", &WorldPartitionGridLevelRange[0], &WorldPartitionGridLevelRange[1], 1, 0, Levels.Num() - 1))
				{
					bIsConfigDirty |= true;
				}
			}
			ImGui::Text("Alpha Color Multiplier");
			ImGui::SliderFloat("##AlphaColorMultiplier", &AlphaColorMultiplier, 0.f, 1.f);
			ImGui::EndMenu();
		}
	}
}

void UUnrealImGuiViewportWorldPartitionExtent::DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
{
	const UWorld* World = Owner->GetWorld();
	const TArray<FSpatialHashStreamingGrid>* StreamingGridsPtr = nullptr;
	if (const UWorldPartition* WorldPartition = World->GetWorldPartition())
	{
		const UWorldPartitionRuntimeSpatialHash* RuntimeHash = Cast<UWorldPartitionRuntimeSpatialHash>(WorldPartition->RuntimeHash);
		if (RuntimeHash)
		{
			static FProperty* StreamingGridsProperty = FindFProperty<FProperty>(RuntimeHash->GetClass(), TEXT("StreamingGrids"));
			StreamingGridsPtr = StreamingGridsProperty->ContainerPtrToValuePtr<TArray<FSpatialHashStreamingGrid>>(RuntimeHash);
		}
	}
	if (StreamingGridsPtr && WorldPartitionGridIndex != INDEX_NONE)
	{
		const TArray<FSpatialHashStreamingGrid>& StreamingGrids = *StreamingGridsPtr;
		if (StreamingGrids.IsValidIndex(WorldPartitionGridIndex))
		{
			const FSpatialHashStreamingGrid& Grid = StreamingGrids[WorldPartitionGridIndex];
			for (int32 LevelIdx = WorldPartitionGridLevelRange[0]; LevelIdx < FMath::Min(Grid.GridLevels.Num(), WorldPartitionGridLevelRange[1]); ++LevelIdx)
			{
				const FSpatialHashStreamingGridLevel& GridLevel = Grid.GridLevels[LevelIdx];
				for (const FSpatialHashStreamingGridLayerCell& LayerCell : GridLevel.LayerCells)
				{
					for (const UWorldPartitionRuntimeCell* Cell : LayerCell.GridCells)
					{
						const FVector Location = Cell->GetCellBounds().GetCenter();
						const int32 CellExtent = Grid.CellSize * (1 << LevelIdx) / 2;
						const FBox2D CellBox{ FVector2D(Location) - CellExtent, FVector2D(Location) + CellExtent };
						FLinearColor DebugColor = Cell->GetDebugColor(EWorldPartitionRuntimeCellVisualizeMode::StreamingStatus);
						DebugColor.A *= AlphaColorMultiplier;
						ViewportContext.DrawRectFilled(CellBox, DebugColor.ToFColor(true));
					}
				}
			}
			ViewportContext.DrawRect(FBox2D{ FVector2D(Grid.WorldBounds.Min), FVector2D(Grid.WorldBounds.Max) }, FColor::Yellow);
		}
	}
}

UUnrealImGuiViewportNavMeshExtent::UUnrealImGuiViewportNavMeshExtent()
{
	ExtentName = TEXT("Nav Mesh");
	Priority = -100;
}

void UUnrealImGuiViewportNavMeshExtent::DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty)
{
	const UWorld* World = Owner->GetWorld();
	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavigationSystem)
	{
		if (ImGui::BeginMenu("Nav Mesh"))
		{
			if (ImGui::RadioButton("None", NavMeshAgentIndex == INDEX_NONE))
			{
				NavMeshAgentIndex = INDEX_NONE;
				bIsConfigDirty |= true;
			}

			const TArray<FNavDataConfig>& Agents = NavigationSystem->GetSupportedAgents();
			for (int32 Idx = 0; Idx < Agents.Num(); ++Idx)
			{
				const FNavDataConfig& Agent = Agents[Idx];
				if (ImGui::RadioButton(TCHAR_TO_UTF8(*Agent.Name.ToString()), NavMeshAgentIndex == Idx))
				{
					NavMeshAgentIndex = Idx;
					bIsConfigDirty |= true;
				}
			}

			ImGui::Text("Max Show Zoom Factor");
			ImGui::SliderInt("##MaxShowZoomFactor", &MaxShowZoomFactor, 1, 10);
			ImGui::EndMenu();
		}
	}
}

void UUnrealImGuiViewportNavMeshExtent::DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
{
	if (ViewportContext.Viewport->ZoomFactor > MaxShowZoomFactor)
	{
		return;
	}
	
	const UWorld* World = Owner->GetWorld();
	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavigationSystem && NavigationSystem->GetSupportedAgents().IsValidIndex(NavMeshAgentIndex))
	{
		const FName& AgentName = NavigationSystem->GetSupportedAgents()[NavMeshAgentIndex].Name;
		const int32 RecastNavMeshIndex = NavigationSystem->NavDataSet.IndexOfByPredicate([&](const ANavigationData* E) { return E && E->GetConfig().Name == AgentName; });
		if (const ARecastNavMesh* RecastNavMesh = RecastNavMeshIndex != INDEX_NONE ? Cast<ARecastNavMesh>(NavigationSystem->NavDataSet[RecastNavMeshIndex]) : nullptr)
		{
			if (const dtNavMesh* DetourNavMesh = RecastNavMesh->GetRecastMesh())
			{
				for (int32 TileIdx = 0; TileIdx < DetourNavMesh->getMaxTiles(); ++TileIdx)
				{
					const FBox TileBounds = RecastNavMesh->GetNavMeshTileBounds(TileIdx);

					dtMeshTile const* const Tile = DetourNavMesh->getTile(TileIdx);
					if (Tile != nullptr && Tile->header != nullptr)
					{
						dtMeshHeader const* const Header = Tile->header;

						// NavMesh
						if (ViewportContext.ViewBounds.Intersect(FBox2D(FVector2D(TileBounds.Min), FVector2D(TileBounds.Max))))
						{
							constexpr ImU32 LayerColor = IM_COL32(0, 120, 0, 127);
							for (int32 PolyIdx = 0; PolyIdx < Header->polyCount; ++PolyIdx)
							{
								dtPoly const* const Poly = &Tile->polys[PolyIdx];
								if (Poly->getType() == DT_POLYTYPE_GROUND)
								{
									dtPolyDetail const* const DetailPoly = &Tile->detailMeshes[PolyIdx];
									for (int32 TriIdx = 0; TriIdx < DetailPoly->triCount; ++TriIdx)
									{
										const int32 DetailTriIdx = (DetailPoly->triBase + TriIdx) * 4;
										const unsigned char* DetailTri = &Tile->detailTris[DetailTriIdx];

										ImVec2 TriVertexes[3];
										for (int32 TriVertIdx = 0; TriVertIdx < UE_ARRAY_COUNT(TriVertexes); ++TriVertIdx)
										{
											if (DetailTri[TriVertIdx] < Poly->vertCount)
											{
												const int32 Idx = Poly->verts[DetailTri[TriVertIdx]];
												TriVertexes[TriVertIdx] = ImVec2{ ViewportContext.WorldToScreenLocation(FVector2D{ Recast2UnrealPoint(&Tile->verts[Idx * 3]) }) };
											}
											else
											{
												const int32 Idx = DetailPoly->vertBase + DetailTri[TriVertIdx] - Poly->vertCount;
												TriVertexes[TriVertIdx] = ImVec2{ ViewportContext.WorldToScreenLocation(FVector2D{ Recast2UnrealPoint(&Tile->detailVerts[Idx * 3]) }) };
											}
										}
										ViewportContext.DrawList->AddTriangleFilled(TriVertexes[0], TriVertexes[1], TriVertexes[2], LayerColor);
									}
								}
							}
						}

						// NavLink
						for (int32 i = 0; i < Header->offMeshConCount; ++i)
						{
							constexpr FColor LinkColor{ 0, 255, 127, 127 };
							if (const dtOffMeshConnection* OffMeshConnection = &Tile->offMeshCons[i])
							{
								dtPoly const* const LinkPoly = &Tile->polys[OffMeshConnection->poly];
								const double* va = &Tile->verts[LinkPoly->verts[0] * 3];
								const double* vb = &Tile->verts[LinkPoly->verts[1] * 3];
								const FVector2D Start = FVector2D(Recast2UnrealPoint(va));
								const FVector2D End = FVector2D(Recast2UnrealPoint(vb));

								const float Thickness = FMath::Clamp(ViewportContext.Viewport->ZoomFactor * 0.4f, 2.f, 4.f);
								ViewportContext.DrawLine(Start, End, LinkColor, Thickness);
							}
						}
					}
				}
			}
		}
	}
}
