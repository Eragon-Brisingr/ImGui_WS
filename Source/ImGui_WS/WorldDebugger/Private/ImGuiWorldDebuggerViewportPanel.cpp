// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerViewportPanel.h"
#include <NavigationSystem.h>
#include <NavMesh/RecastNavMesh.h>
#include <Detour/DetourNavMesh.h>
#include <NavMesh/RecastHelpers.h>
#include <WorldPartition/WorldPartition.h>
#include <WorldPartition/WorldPartitionRuntimeHash.h>
#include <WorldPartition/WorldPartitionRuntimeSpatialHash.h>
#include <GameFramework/PlayerState.h>
#include <EngineUtils.h>
#include <GameFramework/PlayerController.h>

#include "imgui.h"
#include "ImGuiWorldDebuggerBase.h"
#include "UnrealImGuiStat.h"
#include "ImGuiWorldDebuggerDrawer.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerPanel.h"

#define LOCTEXT_NAMESPACE "ImGuiWorldDebugger"

UImGuiWorldDebuggerViewportPanel::UImGuiWorldDebuggerViewportPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	Title = LOCTEXT("Viewport", "Viewport");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Viewport }
	};
}

void UImGuiWorldDebuggerViewportPanel::SetSelectedEntities(const TSet<TWeakObjectPtr<AActor>>& NewSelectedActors)
{
	SelectedActors = NewSelectedActors;
#if WITH_EDITOR
	EditorSelectActors.ExecuteIfBound(GetWorld(), NewSelectedActors);
#endif
}

void UImGuiWorldDebuggerViewportPanel::Register(AImGuiWorldDebuggerBase* WorldDebugger)
{
	Super::Register(WorldDebugger);
	
	CurrentViewLocation = ViewLocation;
	UWorld* World = GetWorld();
	static TMap<TSoftClassPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>> DrawerMap = []
	{
		TMap<TSoftClassPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>> ReturnDrawerMap;
		TArray<UClass*> DerivedDrawers;
		GetDerivedClasses(UImGuiWorldDebuggerDrawerBase::StaticClass(), DerivedDrawers);
		for (UClass* DerivedDrawer : DerivedDrawers)
		{
			if (DerivedDrawer->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}
			if (DerivedDrawer == UImGuiWorldDebuggerDrawer_Default::StaticClass())
			{
				continue;
			}

			const UImGuiWorldDebuggerDrawerBase* Drawer = DerivedDrawer->GetDefaultObject<UImGuiWorldDebuggerDrawerBase>();
			if (Drawer->DrawActor.IsNull() == false)
			{
				ReturnDrawerMap.Add(Drawer->DrawActor, DerivedDrawer);
			}
		}

		if (ReturnDrawerMap.Contains(AActor::StaticClass()) == false)
		{
			ReturnDrawerMap.Add(AActor::StaticClass(), UImGuiWorldDebuggerDrawer_Default::StaticClass());
		}
		
		return ReturnDrawerMap;
	}();

	auto TryAddActorToDraw = [this](AActor* Actor)
	{
		if (Actor == nullptr || Actor->IsA<AImGuiWorldDebuggerBase>())
		{
			return;
		}

		for (const UClass* TestClass = Actor->GetClass(); TestClass != UObject::StaticClass(); TestClass = TestClass->GetSuperClass())
		{
			if (const TSubclassOf<UImGuiWorldDebuggerDrawerBase>* Drawer = DrawerMap.Find(TestClass))
			{
				DrawableActors.Add(Actor, *Drawer);
				Actor->OnEndPlay.AddUniqueDynamic(this, &UImGuiWorldDebuggerViewportPanel::WhenActorEndPlay);
				return;
			}
		}
	};
	for (TActorIterator<AActor> It{World}; It; ++It)
	{
		TryAddActorToDraw(*It);
	}
	OnLevelAdd_DelegateHandle = FWorldDelegates::LevelAddedToWorld.AddWeakLambda(this, [this, TryAddActorToDraw](ULevel* Level, UWorld* World)
	{
		if (World != GetWorld())
		{
			return;
		}

		for (AActor* Actor : Level->Actors)
		{
			TryAddActorToDraw(Actor);
		}
	});
	OnActorSpawnedHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [TryAddActorToDraw](AActor* Actor)
	{
		TryAddActorToDraw(Actor);
	}));
}

void UImGuiWorldDebuggerViewportPanel::Unregister(AImGuiWorldDebuggerBase* WorldDebugger)
{
	Super::Unregister(WorldDebugger);
	
	if (UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandler);
	}
	FWorldDelegates::LevelAddedToWorld.Remove(OnLevelAdd_DelegateHandle);
}

void UImGuiWorldDebuggerViewportPanel::FocusActors(const TArray<AActor*>& Actors)
{
	check(Actors.Contains(nullptr) == false);
	
	if (Actors.Num() == 0)
	{
		SelectedActors.Reset();
		return;
	}

	FVector2D CenterViewLocation{ Actors[0]->GetActorLocation() };
	FBox2D Bounds{ CenterViewLocation, CenterViewLocation };
	for (int32 Idx = 1; Idx < Actors.Num(); ++Idx)
	{
		const FVector2D ActorLocation{ Actors[Idx]->GetActorLocation() };
		Bounds = Bounds.ShiftBy(ActorLocation);
		CenterViewLocation += ActorLocation;
		CenterViewLocation /= 2.f;
	}
	ViewLocation = CenterViewLocation;
	SelectedActors.Reset();
	for (AActor* Actor : Actors)
	{
		SelectedActors.Add(Actor);
	}
	// TODO：缓存ViewBounds，对视口进行缩放
}

void UImGuiWorldDebuggerViewportPanel::SetFilterString(const FString& FilterString)
{
	FilterActorString = FilterString;
	WhenFilterStringChanged(FilterString);
}

const FString UImGuiWorldDebuggerViewportPanel::EFilterActorType::TypeFilter_FilterType = TEXT("type");

void UImGuiWorldDebuggerViewportPanel::WhenFilterStringChanged(const FString& FilterString)
{
	FilterActorString = FilterString;

	FString FilterType;
	FString FilterValue;
	if (FilterString.Split(TEXT(":"), &FilterType, &FilterValue))
	{
		FilterActorType = 0;
		if (FilterType == EFilterActorType::TypeFilter_FilterType)
		{
			FilterActorClass = UClass::TryFindTypeSlow<UClass>(FilterValue);
			if (FilterActorClass.IsValid() == false)
			{
				FilterActorClass = LoadObject<UClass>(nullptr, *FilterValue);
			}
			if (FilterActorClass.IsValid())
			{
				FilterActorType = EFilterActorType::TypeFilter;
				return;
			}
		}
	}
	else if (FilterString.IsEmpty())
	{
		FilterActorType = EFilterActorType::None;
	}
	else
	{
		FilterActorType = EFilterActorType::NameFilter;
	}
}

void UImGuiWorldDebuggerViewportPanel::DrawFilterTooltip()
{
	ImGui::Text("SpecialFilterTag\n  type:(Filter actors by type)");
}

void UImGuiWorldDebuggerViewportPanel::DrawFilterPopup(UWorld* World)
{
	ImGuiIO& IO = ImGui::GetIO();

	FString FilterType;
	FString FilterValue;
	if (FilterActorString.Split(TEXT(":"), &FilterType, &FilterValue))
	{
		if (FilterType == EFilterActorType::TypeFilter_FilterType)
		{
			TSet<UClass*> Classes;
			for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : DrawableActors)
			{
				AActor* Actor = Pair.Key.Get();
				if (Actor)
				{
					Classes.Add(Actor->GetClass());
				}
			}
			TSet<UClass*> PopupClasses;
			for (UClass* Class : Classes)
			{
				for (UClass* SuperClass = Class; SuperClass != AActor::StaticClass(); SuperClass = SuperClass->GetSuperClass())
				{
					PopupClasses.Add(SuperClass);
				}
			}
			PopupClasses.Sort([](const UClass& LHS, const UClass& RHS){ return LHS.GetFName().FastLess(RHS.GetFName()); });
			for (UClass* Class : PopupClasses)
			{
				const FString ClassPathName = Class->GetPathName();
				if (FilterValue.IsEmpty() || ClassPathName.Contains(FilterValue))
				{
					ImGui::Selectable(TCHAR_TO_UTF8(*Class->GetName()));
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class->GetFullName()));
						ImGui::EndTooltip();
						if (IO.MouseDown[ImGuiMouseButton_Left])
						{
							WhenFilterStringChanged(FString::Printf(TEXT("%s:%s"), *EFilterActorType::TypeFilter_FilterType, *ClassPathName));
							FocusActorsByFilter(World);
						}
					}
				}
			}
		}
	}
	else
	{
		for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : DrawableActors)
		{
			AActor* Actor = Pair.Key.Get();
			if (Actor == nullptr)
			{
				continue;
			}
						
			if (FilterActorString.IsEmpty() || Actor->GetName().Contains(FilterActorString))
			{
				ImGui::Selectable(TCHAR_TO_UTF8(*Actor->GetName()));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetFullName(World)));
					ImGui::EndTooltip();
					if (IO.MouseDown[ImGuiMouseButton_Left])
					{
						const auto StringPoint = FTCHARToUTF8(*Actor->GetName());
						WhenFilterStringChanged(Actor->GetName());
						FocusActor(Actor);
					}
				}
			}
		}
	}
}

bool UImGuiWorldDebuggerViewportPanel::IsShowActorsByFilter(const AActor* Actor)
{
	if (FilterActorType == EFilterActorType::TypeFilter)
	{
		if (UClass* FilterClass = FilterActorClass.Get())
		{
			return Actor->IsA(FilterClass);
		}
	}
	return false;
}

void UImGuiWorldDebuggerViewportPanel::FocusActorsByFilter(UWorld* World)
{
	if (FilterActorType == EFilterActorType::NameFilter)
	{
		const FName ActorName = *FilterActorString;
		for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : DrawableActors)
		{
			AActor* Actor = Pair.Key.Get();
			if (Actor && Actor->GetFName() == ActorName)
			{
				FocusActor(Actor);
				break;
			}
		}
	}
}

void UImGuiWorldDebuggerViewportPanel::WhenActorEndPlay(AActor* Actor, const EEndPlayReason::Type EndPlayReason)
{
	DrawableActors.Remove(Actor);
}

#if WITH_EDITOR
void UImGuiWorldDebuggerViewportPanel::WhenEditorSelectionChanged(const TArray<AActor*>& SelectedActors)
{
	for (TObjectIterator<UImGuiWorldDebuggerViewportPanel> It; It; ++It)
	{
		if (It->IsTemplate() == false)
		{
			UWorld* World = It->GetWorld();
			if (World && World == GWorld)
			{
				It->SelectedActors.Empty();
				for (AActor* Actor : SelectedActors)
				{
					if (Actor->GetWorld() == World)
					{
						It->SelectedActors.Add(Actor);
					}
				}
			}
		}
	}
}

UImGuiWorldDebuggerViewportPanel::FEditorSelectActors UImGuiWorldDebuggerViewportPanel::EditorSelectActors;
#endif

namespace ImGuiHelper
{
	inline const ImVec2 Convert(const FVector2D& UnrealVector)
	{
		return ImVec2{ (float)UnrealVector.X, (float)UnrealVector.Y };
	}

	inline const FVector2D Convert(const ImVec2& ImGuiVector)
	{
		return FVector2D{ ImGuiVector.x, ImGuiVector.y };
	}
}

void UImGuiWorldDebuggerViewportPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	Super::Draw(WorldDebugger, DeltaSeconds);

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebuggerViewportPanel_Draw"), STAT_ImGuiWorldDebuggerViewportPanel_Draw, STATGROUP_ImGui);
	
	UWorld* World = GetWorld();
	check(World);
	
	using namespace ImGuiHelper;

	bool IsConfigDirty = false;
	CurrentViewLocation = FMath::Vector2DInterpTo(CurrentViewLocation, ViewLocation, DeltaSeconds, 10.f);

	ImGuiIO& IO = ImGui::GetIO();
	// 防止左键对窗口的拖拽
	TGuardValue<bool> ConfigWindowsMoveFromTitleBarOnlyGuard{ IO.ConfigWindowsMoveFromTitleBarOnly, true };

	const ImVec2 WindowSize = ImGui::GetWindowSize();
	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const TArray<FSpatialHashStreamingGrid>* StreamingGridsPtr = nullptr;
	if (UWorldPartition* WorldPartition = World->GetWorldPartition())
	{
		const UWorldPartitionRuntimeSpatialHash* RuntimeHash = Cast<UWorldPartitionRuntimeSpatialHash>(WorldPartition->RuntimeHash);
		if (RuntimeHash)
		{
			static FProperty* StreamingGridsProperty = FindFProperty<FProperty>(RuntimeHash->GetClass(), TEXT("StreamingGrids"));
			StreamingGridsPtr = StreamingGridsProperty->ContainerPtrToValuePtr<TArray<FSpatialHashStreamingGrid>>(RuntimeHash);
		}
	}

	bool IsMenuPopupState = false;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Game World"))
		{
			if (ImGui::Button("Back Origin"))
			{
				ViewLocation = FVector2D::ZeroVector;
			}
			ImGui::EndMenu();
		}
		DrawDebugInfoMenu(IsConfigDirty);
		if (NavigationSystem)
		{
			if (ImGui::BeginMenu("Nav Mesh"))
			{
				if (ImGui::RadioButton("None", NavMeshAgentIndex == INDEX_NONE))
				{
					NavMeshAgentIndex = INDEX_NONE;
					IsConfigDirty |= true;
				}

				const TArray<FNavDataConfig>& Agents = NavigationSystem->GetSupportedAgents();
				for (int32 Idx = 0; Idx < Agents.Num(); ++Idx)
				{
					const FNavDataConfig& Agent = Agents[Idx];
					if (ImGui::RadioButton(TCHAR_TO_UTF8(*Agent.Name.ToString()), NavMeshAgentIndex == Idx))
					{
						NavMeshAgentIndex = Idx;
						IsConfigDirty |= true;
					}
				}
				ImGui::EndMenu();
			}
		}
		if (StreamingGridsPtr)
		{
			if (ImGui::BeginMenu("World Partition"))
			{
				if (ImGui::RadioButton("None", WorldPartitionGridIndex == INDEX_NONE))
				{
					WorldPartitionGridIndex = INDEX_NONE;
					IsConfigDirty |= true;
				}
				const TArray<FSpatialHashStreamingGrid>& StreamingGrids = *StreamingGridsPtr;
				for (int32 Idx = 0; Idx < StreamingGrids.Num(); ++Idx)
				{
					const FSpatialHashStreamingGrid& Grid = StreamingGrids[Idx];
					if (ImGui::RadioButton(TCHAR_TO_UTF8(*Grid.GridName.ToString()), WorldPartitionGridIndex == Idx))
					{
						WorldPartitionGridIndex = Idx;
						IsConfigDirty |= true;
					}
				}
				if (StreamingGrids.IsValidIndex(WorldPartitionGridIndex))
				{
					const TArray<FSpatialHashStreamingGridLevel>& Levels = StreamingGrids[WorldPartitionGridIndex].GridLevels;
					ImGui::Text("Level Range");
					if (ImGui::DragIntRange2("##Level Range", &WorldPartitionGridLevelRange[0], &WorldPartitionGridLevelRange[1], 1, 0, Levels.Num() - 1))
					{
						IsConfigDirty |= true;
					}
				}
				ImGui::EndMenu();
			}
		}

		{
			ImGui::Indent(WindowSize.x - 240.f);
			ImGui::SetNextItemWidth(200.f);
			TArray<ANSICHAR, TInlineAllocator<256>> FilterActorArray;
			{
				const auto StringPoint = FTCHARToUTF8(*FilterActorString);
				FilterActorArray.SetNumZeroed(FMath::Max(256, StringPoint.Length() + 128));
				FMemory::Memcpy(FilterActorArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
			}
			if (ImGui::InputTextWithHint("##FilterActor", "Filter Actor", FilterActorArray.GetData(), FilterActorArray.Num()))
			{
				WhenFilterStringChanged(UTF8_TO_TCHAR(FilterActorArray.GetData()));
			}
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
			{
				ImGui::BeginTooltip();
				DrawFilterTooltip();
				ImGui::EndTooltip();
			}

			static bool PreInputTextIsActive = false;
			bool InputTextIsActive = ImGui::IsItemActive();
			const ImVec2 FilterActorBarPos = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				FocusActorsByFilter(World);
			}

			ImGui::SameLine();
			if (ImGui::Button("x##ClearFilterActor"))
			{
				WhenFilterStringChanged(TEXT(""));
			}
			if (PreInputTextIsActive || InputTextIsActive)
			{
				IsMenuPopupState |= true;
				ImGui::SetNextWindowPos(FilterActorBarPos, ImGuiCond_Always);
				ImGui::SetNextWindowSize({200.f, 200.f});
				ImGui::OpenPopup("##FilterActorBar");
				if (ImGui::BeginPopup("##FilterActorBar", ImGuiWindowFlags_ChildWindow))
				{
					ImGui::PushAllowKeyboardFocus(false);

					DrawFilterPopup(World);

					ImGui::PopAllowKeyboardFocus();
					ImGui::EndPopup();
				}
			}
			PreInputTextIsActive = InputTextIsActive;
		}
		ImGui::EndMenuBar();
	}

	const float MapSize[]{ WindowSize.x - 20.f, WindowSize.y - 60.f };
	ImGui::BeginChild("Map", { MapSize[0], MapSize[1] });
	{
		const ImVec2 RectMin = ImGui::GetItemRectMin();
		const ImVec2 RectMax{ RectMin.x + MapSize[0], RectMin.y + MapSize[1] };

		const bool IsMultiSelect = IO.KeyCtrl;
		const bool IsSelectDragging = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		AActor* TopSelectedActor = nullptr;
		
		float Zoom = FMath::Pow(2.f, -ZoomFactor);
		const FVector2D RelativeMousePos{ IO.MousePos.x - RectMin.x, IO.MousePos.y - RectMin.y };
		if (ImGui::IsWindowHovered())
		{
			if (IO.MouseWheel != 0.f && IsMenuPopupState == false)
			{
				ZoomFactor = FMath::Clamp(int32(ZoomFactor - IO.MouseWheel), 0, 10);
				IsConfigDirty |= true;
			}
			static bool PreIsMouseDragging = false;
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			{
				PreIsMouseDragging = true;
				ViewLocation -= FVector2D(IO.MouseDelta.x, IO.MouseDelta.y) / Zoom;
				CurrentViewLocation = ViewLocation;
			}
			else if (PreIsMouseDragging)
			{
				IsConfigDirty |= true;
				PreIsMouseDragging = false;
			}
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				if (IsMultiSelect == false)
				{
					SelectedActors.Reset();
				}
			}
		}

		ImDrawList* DrawList = ImGui::GetWindowDrawList();

		FImGuiWorldViewportContext DebuggerContext
		{
			this,
			DrawList,
			{ RectMin.x, RectMin.y },
			{ MapSize[0], MapSize[1] },
			CurrentViewLocation,
			Zoom,
			{ IO.MousePos.x, IO.MousePos.y },
			IsSelectDragging,
			{ IO.MouseClickedPos[0].x,IO.MouseClickedPos[0].y },
			DeltaSeconds
		};
		const FVector2D MouseWorldPosition = DebuggerContext.MapToWorldLocation(RelativeMousePos);
		
		// 绘制世界分块
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
						for (UWorldPartitionRuntimeSpatialHashCell* SpatialHashCell : LayerCell.GridCells)
						{
							const FVector Location = SpatialHashCell->Position;
							const int32 CellExtent = Grid.CellSize * (1 << LevelIdx) / 2;
							const FBox2D CellBox{ FVector2D(Location) - CellExtent, FVector2D(Location) + CellExtent };
							DebuggerContext.DrawRectFilled(CellBox, SpatialHashCell->GetDebugColor(EWorldPartitionRuntimeCellVisualizeMode::StreamingStatus));
						}
					}
				}
				DebuggerContext.DrawRect(FBox2D{ FVector2D(Grid.WorldBounds.Min), FVector2D(Grid.WorldBounds.Max) }, FLinearColor::Yellow);
			}
		}
		
		// 绘制MavMesh
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

							// 绘制NavMesh
							if (DebuggerContext.ViewBounds.Intersect(FBox2D(FVector2D(TileBounds.Min), FVector2D(TileBounds.Max))))
							{
								const ImU32 LayerColor = FImGuiWorldViewportContext::LinearColorToU32({ 0.0f, 0.4f, 0.0f, 0.5f });
								TArray<ImVec2> Verts;
								Verts.SetNumUninitialized(Header->vertCount);
								for (int32 VertIdx = 0; VertIdx < Header->vertCount; ++VertIdx)
								{
									FVector const VertPos = Recast2UnrealPoint(&Tile->verts[VertIdx * 3]);
									Verts[VertIdx] = Convert(DebuggerContext.WorldToScreenLocation(FVector2D(VertPos)));
								}

								int32 const DetailVertIndexBase = Header->vertCount;
								for (int32 PolyIdx = 0; PolyIdx < Header->polyCount; ++PolyIdx)
								{
									dtPoly const* const Poly = &Tile->polys[PolyIdx];

									if (Poly->getType() == DT_POLYTYPE_GROUND)
									{
										dtPolyDetail const* const DetailPoly = &Tile->detailMeshes[PolyIdx];
										for (int32 TriIdx = 0; TriIdx < DetailPoly->triCount; ++TriIdx)
										{
											int32 DetailTriIdx = (DetailPoly->triBase + TriIdx) * 4;
											const unsigned char* DetailTri = &Tile->detailTris[DetailTriIdx];

											// TODO：处理索引越界问题
											bool HasErrorTris = false;

											ImVec2 ImGuiPloyVerts[3];
											// calc indices into the vert buffer we just populated
											for (int32 TriVertIdx = 0; TriVertIdx < 3; ++TriVertIdx)
											{
												if (DetailTri[TriVertIdx] < Poly->vertCount)
												{
													ImGuiPloyVerts[TriVertIdx] = Verts[Poly->verts[DetailTri[TriVertIdx]]];
												}
												else
												{
													const int32 Offset = DetailVertIndexBase + (DetailPoly->vertBase + DetailTri[TriVertIdx] - Poly->vertCount);
													if (Verts.IsValidIndex(Offset))
													{
														ImGuiPloyVerts[TriVertIdx] = Verts[Offset];
													}
													else
													{
														HasErrorTris = true;
														break;
													}
												}
											}
											if (HasErrorTris == false)
											{
												DrawList->AddTriangleFilled(ImGuiPloyVerts[0], ImGuiPloyVerts[1], ImGuiPloyVerts[2], LayerColor);
											}
										}
									}
								}
							}

							// 绘制NavLink
							for (int32 i = 0; i < Header->offMeshConCount; ++i)
							{
								const FLinearColor LinkColor = { 0.0f, 1.f, 0.5f, 0.25f};
								if (const dtOffMeshConnection* OffMeshConnection = &Tile->offMeshCons[i])
								{
									dtPoly const* const LinkPoly = &Tile->polys[OffMeshConnection->poly];
									const double* va = &Tile->verts[LinkPoly->verts[0] * 3];
									const double* vb = &Tile->verts[LinkPoly->verts[1] * 3];
									const FVector2D Start = FVector2D(Recast2UnrealPoint(va));
									const FVector2D End = FVector2D(Recast2UnrealPoint(vb));

									const float Thickness = FMath::Clamp(ZoomFactor * 0.4f, 2.f, 4.f);
									DebuggerContext.DrawLine(Start, End, LinkColor, Thickness);
								}
							}
						}
					}
				}
			}
		}

		DrawDebugInfoUnderActors(DebuggerContext);
		
		// 绘制原点
		if (DebuggerContext.ViewBounds.Intersect(FBox2D{ FVector2D(-10.f) / Zoom, FVector2D(10.f) / Zoom }))
		{
			const FVector2D ScreenLocation = DebuggerContext.WorldToScreenLocation(FVector2D::ZeroVector);
			DrawList->AddLine({ (float)ScreenLocation.X, (float)ScreenLocation.Y }, { (float)ScreenLocation.X + 10.f, (float)ScreenLocation.Y }, FImGuiWorldViewportContext::LinearColorToU32(FLinearColor::Red));
			DrawList->AddLine({ (float)ScreenLocation.X, (float)ScreenLocation.Y }, { (float)ScreenLocation.X, (float)ScreenLocation.Y + 10.f }, FImGuiWorldViewportContext::LinearColorToU32(FLinearColor::Green));
		}

		// 绘制实体位置
		{
			const float ScreenMinRadius = 2.f;
			const float MinRadius = ScreenMinRadius / Zoom;
			auto DrawActor = [&](AActor* Actor, const UImGuiWorldDebuggerDrawerBase* Drawer, bool IsSelected)
			{
				const FVector Location = Actor->GetActorLocation();
				const FRotator Rotation = Actor->GetActorRotation();
				const float Radius = Drawer->Radius < MinRadius ? MinRadius : Drawer->Radius;

				const FBox2D ActorBounds = FBox2D{ FVector2D{Location} - FVector2D(Radius), FVector2D{Location} + FVector2D(Radius) };
				if (IsSelected || DebuggerContext.ViewBounds.Intersect(ActorBounds))
				{
					if (IsSelected == false)
					{
						if (FilterActorType != EFilterActorType::None && FilterActorType != EFilterActorType::NameFilter)
						{
							if (IsShowActorsByFilter(Actor) == false)
							{
								return;
							}
						}
						else
						{
							// Actor的默认绘制类默认不显示
							if (Drawer->GetClass() == UImGuiWorldDebuggerDrawer_Default::StaticClass())
							{
								return;
							}
							if (Drawer->bAlwaysDebuggerDraw == false && Drawer->Radius < MinRadius)
							{
								return;
							}
						}
					}
					
					if (IsSelectDragging && DebuggerContext.SelectDragBounds.Intersect(ActorBounds))
					{
						SelectedActors.Add(Actor);
						TopSelectedActor = Actor;
					}
					else if (ImGui::IsWindowHovered() && IO.MouseReleased[ImGuiMouseButton_Left])
					{
						const float DistanceSquared = (FVector2D{ Location } - MouseWorldPosition).SizeSquared();
						if (DistanceSquared < Radius * Radius)
						{
							TopSelectedActor = Actor;
						}
					}

					FGuardValue_Bitfield(DebuggerContext.bIsSelected, IsSelected);
					Drawer->DrawImGuiDebuggerExtendInfo(Actor, DebuggerContext);

					DebuggerContext.DrawCircleFilled(FVector2D(Location), Radius, Drawer->Color, 8);
					if ((MouseWorldPosition - FVector2D(Location)).SizeSquared() < Radius * Radius)
					{
						ImGui::BeginTooltip();
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetName()));
						ImGui::Separator();
						Drawer->DrawImGuiDebuggerToolTips(Actor);
						ImGui::EndTooltip();
					}

					if (Radius > MinRadius * MinRadius)
					{
						const float Thickness = FMath::GetMappedRangeValueClamped(TRange<float>{ 4.f, 16.f }, TRange<float>{ 2.f, 4.f }, Radius * Zoom);
						DebuggerContext.DrawLine(FVector2D(Location), FVector2D(Location + Rotation.Vector() * Radius * 1.5f), FLinearColor::Black, Thickness);
					}

					if (IsSelected)
					{
						const FLinearColor SelectedColor{ 0.8f, 0.4f, 0.f };
						const float Thickness = FMath::GetMappedRangeValueClamped(TRange<float>{ 4.f, 16.f }, TRange<float>{ 2.f, 8.f }, Radius * Zoom);
						DebuggerContext.DrawCircle(FVector2D(Location), Radius, SelectedColor, 8, Thickness);
					}
				}
			};
			for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : DrawableActors)
			{
				if (AActor* Actor = Pair.Key.Get())
				{
					UImGuiWorldDebuggerDrawerBase* Drawer = Pair.Value.GetDefaultObject();
					const bool IsSelected = SelectedActors.Contains(Actor);
					if (IsSelected)
					{
						continue;
					}
					DrawActor(Actor, Drawer, false);
				}
			}
			for (const TWeakObjectPtr<AActor>& ActorPtr : TSet<TWeakObjectPtr<AActor>>(SelectedActors))
			{
				if (AActor* Actor = ActorPtr.Get())
				{
					if (const TSubclassOf<UImGuiWorldDebuggerDrawerBase>* Drawer = DrawableActors.Find(Actor))
					{
						DrawActor(Actor, Drawer->GetDefaultObject(), true);
					}
				}
			}
		}

		// 绘制玩家位置
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PlayerController = It->Get())
			{
				// 绘制相机的锥体
				FVector ControlLocation;
				FRotator ControlRotation;
				PlayerController->GetPlayerViewPoint(ControlLocation, ControlRotation);

				const FTransform CameraViewTransform{ ControlRotation.Quaternion(), ControlLocation };
				const FVector2D CameraViewShape[3] = { FVector2D{ ControlLocation }, FVector2D(CameraViewTransform.TransformPosition({300.f, 100.f, 0.f})), FVector2D(CameraViewTransform.TransformPosition({300.f, -100.f, 0.f})) };

				if (DebuggerContext.ViewBounds.Intersect(FBox2D(TArray<FVector2D>{ CameraViewShape, 3 })))
				{
					const ImU32 CameraViewColor = FImGuiWorldViewportContext::LinearColorToU32({ 0.f, 0.5f, 1.f, 0.5f });
					const ImVec2 CameraScreenRelativeLocation = Convert(DebuggerContext.WorldToScreenLocation(CameraViewShape[0]));
					DrawList->AddTriangleFilled(CameraScreenRelativeLocation, Convert(DebuggerContext.WorldToScreenLocation(CameraViewShape[1])), Convert(DebuggerContext.WorldToScreenLocation(CameraViewShape[2])), CameraViewColor);

					// 绘制玩家名
					if (APlayerState* PlayerState = PlayerController->PlayerState)
					{
						const ImU32 PlayerNameColor = FImGuiWorldViewportContext::LinearColorToU32(FLinearColor::White);
						DrawList->AddText({ CameraScreenRelativeLocation.x - 40.f, CameraScreenRelativeLocation.y }, PlayerNameColor, TCHAR_TO_UTF8(*PlayerState->GetPlayerName()));
					}
				}
			}
		}

		DrawDebugInfoUpperActors(DebuggerContext);
		
		// 绘制拖拽选择
		if (IsSelectDragging)
		{
			const ImU32 DragColor = FImGuiWorldViewportContext::LinearColorToU32({1.f, 1.f, 1.f, 0.2f});
			DrawList->AddRectFilled(IO.MouseClickedPos[0], IO.MousePos, DragColor);
			DrawList->AddRect(IO.MouseClickedPos[0], IO.MousePos, DragColor);
		}
		
		// 绘制比例尺
		{
			const ImU32 RatioColor = FImGuiWorldViewportContext::LinearColorToU32(FLinearColor::White);
			const float PixelLength = 50.f;
			const float RealLength = PixelLength / 100.f / Zoom;
			const ImVec2 Start{ RectMin.x + PixelLength, RectMax.y - 30.f };
			const ImVec2 End{ Start.x + PixelLength, Start.y };

			DrawList->AddLine(Start, End, RatioColor);
			DrawList->AddLine({ Start.x, Start.y - 4.f }, Start, RatioColor);
			DrawList->AddLine({ End.x, End.y - 4.f }, End, RatioColor);
			DrawList->AddText({ Start.x + 4.f, Start.y + 2.f }, RatioColor, TCHAR_TO_UTF8(*FString::Printf(TEXT("%.1f m"), RealLength)));
		}

		// 绘制信息
		{
			// 绘制信息底板
			const ImU32 BackgroundColor = FImGuiWorldViewportContext::LinearColorToU32(FLinearColor{0.f, 0.f, 0.f, 0.8f});
			DrawList->AddRectFilled({ RectMin.x + 5.f, RectMin.y + 8.f}, { RectMin.x + 350.f + 5.f, RectMin.y + (DebuggerContext.Messages.Num() + 1) * DebuggerContext.MessageRowHeight - 2.f }, BackgroundColor);
			
			// 绘制暂存的消息
			for (const auto& Message : DebuggerContext.Messages)
			{
				DebuggerContext.AddMessageTextImmediately(Message.Text, Message.Color);
			}
		}
		
		// 绘制边框
		{
			const ImU32 EdgeColor = FImGuiWorldViewportContext::LinearColorToU32(FLinearColor::White);
			const ImVec2 Min{ RectMin.x, RectMin.y };
			const ImVec2 Max{ RectMax.x - 1.f, RectMax.y - 1.f };
			DrawList->AddLine({ Min.x, Min.y }, { Max.x, Min.y }, EdgeColor);
			DrawList->AddLine({ Max.x, Min.y }, { Max.x, Max.y }, EdgeColor);
			DrawList->AddLine({ Max.x, Max.y }, { Min.x, Max.y }, EdgeColor);
			DrawList->AddLine({ Min.x, Max.y }, { Min.x, Min.y }, EdgeColor);
		}

		// 根据选中Actor距离进行排序
		if (TopSelectedActor)
		{
			// 清理无效的Actor
			{
				TArray<TWeakObjectPtr<AActor>> SelectedWorldActorsArray = SelectedActors.Array();
				SelectedWorldActorsArray.RemoveAll([](const TWeakObjectPtr<AActor>& E) { return E.IsValid() == false; });
				SelectedActors = TSet<TWeakObjectPtr<AActor>>{ SelectedWorldActorsArray };
			}
			
			SelectedActors.Add(TopSelectedActor);
			SelectedActors.Sort([&](const TWeakObjectPtr<AActor>& LHS, const TWeakObjectPtr<AActor>& RHS)
			{
				const FVector2D LHS_Location = FVector2D(LHS->GetActorLocation());
				const FVector2D RHS_Location = FVector2D(RHS->GetActorLocation());
				return (LHS_Location - MouseWorldPosition).SizeSquared() < (RHS_Location - MouseWorldPosition).SizeSquared();
			});
		}

#if WITH_EDITOR
		// 编辑器下同步选择
		if (IsSelectDragging == false)
		{
			static TSet<TWeakObjectPtr<AActor>> PreSelectedActors;
			if (PreSelectedActors.Num() != SelectedActors.Num() || PreSelectedActors.Difference(SelectedActors).Num() > 0 || SelectedActors.Difference(PreSelectedActors).Num() > 0)
			{
				EditorSelectActors.ExecuteIfBound(World, SelectedActors);
				PreSelectedActors = SelectedActors;
			}
		}
#endif

		IsConfigDirty |= DebuggerContext.bIsConfigDirty;
	}
	ImGui::EndChild();

	if (IsConfigDirty)
	{
		SaveConfig();
	}
}

AActor* UImGuiWorldDebuggerViewportPanel::GetFirstSelectActor() const
{
	for (const TWeakObjectPtr<AActor>& ActorPtr : SelectedActors)
	{
		if (AActor* Actor = ActorPtr.Get())
		{
			return Actor;
		}
	}
	return nullptr;
}

void UImGuiWorldDebuggerViewportPanel::FocusActor(AActor* Actor)
{
	if (IsValid(Actor) == false)
	{
		return;
	}
	ViewLocation = FVector2D(Actor->GetActorLocation());
	SelectedActors.Reset();
	SelectedActors.Add(Actor);
}

ImU32 FImGuiWorldViewportContext::LinearColorToU32(const FLinearColor& Color)
{
	return ImGui::ColorConvertFloat4ToU32((ImVec4&)Color);
}

void FImGuiWorldViewportContext::DrawRect(const FBox2D& Box, const FLinearColor& Color, float Rounding) const
{
	if (ViewBounds.Intersect(Box) && Box.ExpandBy(Rounding).IsInside(ViewBounds) == false)
	{
		using namespace ImGuiHelper;
		DrawList->AddRect(Convert(WorldToScreenLocation(Box.Min)), Convert(WorldToScreenLocation(Box.Max)), LinearColorToU32(Color), Rounding);
	}
}

void FImGuiWorldViewportContext::DrawRectFilled(const FBox2D& Box, const FLinearColor& Color, float Rounding) const
{
	if (ViewBounds.Intersect(Box))
	{
		using namespace ImGuiHelper;
		DrawList->AddRectFilled(Convert(WorldToScreenLocation(Box.Min)), Convert(WorldToScreenLocation(Box.Max)), LinearColorToU32(Color), Rounding);
	}
}

void FImGuiWorldViewportContext::DrawLine(const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness) const
{
	if (ViewBounds.Intersect(FBox2D(TArray<FVector2D>{Start, End})))
	{
		using namespace ImGuiHelper;
		DrawList->AddLine(Convert(WorldToScreenLocation(Start)), Convert(WorldToScreenLocation(End)), LinearColorToU32(Color), Thickness);
	}
}

void FImGuiWorldViewportContext::DrawCircleFilled(const FVector2D& Center, float Radius, const FLinearColor& Color, int NumSegments) const
{
	const FBox2D CircleBounds = FBox2D{ FVector2D{Center} - FVector2D(Radius), FVector2D{Center} + FVector2D(Radius) };
	if (ViewBounds.Intersect(CircleBounds))
	{
		using namespace ImGuiHelper;
		DrawList->AddCircleFilled(Convert(WorldToScreenLocation(Center)), Radius * Zoom, LinearColorToU32(Color), NumSegments);
	}
}

void FImGuiWorldViewportContext::DrawCircle(const FVector2D& Center, float Radius, const FLinearColor& Color, int NumSegments, float Thickness) const
{
	const FBox2D CircleBounds = FBox2D{ FVector2D{Center} - FVector2D(Radius), FVector2D{Center} + FVector2D(Radius) };
	if (ViewBounds.Intersect(CircleBounds))
	{
		const float InnerRectRadius = Radius * 0.7071f;
		const FBox2D InnerRectBounds = FBox2D{ FVector2D{Center} - FVector2D(InnerRectRadius), FVector2D{Center} + FVector2D(InnerRectRadius) };
		
		if (InnerRectBounds.IsInside(ViewBounds) == false)
		{
			using namespace ImGuiHelper;
			DrawList->AddCircle(Convert(WorldToScreenLocation(Center)), Radius * Zoom, LinearColorToU32(Color), NumSegments, Thickness);
		}
	}
}

void FImGuiWorldViewportContext::DrawText(const FVector2D& Position, const FVector2D& ScreenOffset, const FString& Text, const FLinearColor& Color) const
{
	const FVector2D DrawPosition = AddScreenOffset(Position, ScreenOffset);
	if (ViewBounds.IsInside(DrawPosition))
	{
		using namespace ImGuiHelper;
		DrawList->AddText(Convert(WorldToScreenLocation(DrawPosition)), LinearColorToU32(Color), TCHAR_TO_UTF8(*Text));
	}
}

void FImGuiWorldViewportContext::AddMessageTextImmediately(const FString& Message, const FLinearColor& Color) const
{
	const ImU32 TextColor = LinearColorToU32(Color);
	DrawList->AddText({ (float)RectMin.X + 12.f, (float)RectMin.Y + 10.f + MessageRowHeight * MessageRowIdx }, TextColor, TCHAR_TO_UTF8(*Message));
	MessageRowIdx += 1;
}

#undef LOCTEXT_NAMESPACE
