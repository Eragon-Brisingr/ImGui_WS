// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerOutlinerPanel.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiWorldDebuggerBase.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerViewportPanel.h"

UImGuiWorldDebuggerOutlinerPanel::UImGuiWorldDebuggerOutlinerPanel()
	: bInvokeRefreshSortOrder(false)
{
	Title = NSLOCTEXT("ImGuiWorldDebugger", "Outliner", "Outliner");
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Outliner }
	};
}

void UImGuiWorldDebuggerOutlinerPanel::Register(AImGuiWorldDebuggerBase* WorldDebugger)
{
	UWorld* World = GetWorld();
	RefreshDisplayActors();
	OnLevelAdd_DelegateHandle = FWorldDelegates::LevelAddedToWorld.AddWeakLambda(this, [this](ULevel* Level, UWorld* World)
	{
		if (World != GetWorld())
		{
			return;
		}

		for (AActor* Actor : Level->Actors)
		{
			if (Actor && CanActorDisplay(Actor))
			{
				DisplayActors.Add(Actor);
				Actor->OnDestroyed.AddUniqueDynamic(this, &UImGuiWorldDebuggerOutlinerPanel::WhenActorDestroy);
			}
		}
		bInvokeRefreshSortOrder |= true;
	});
	OnActorSpawnedHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [this](AActor* Actor)
	{
		if (CanActorDisplay(Actor) && Actor->OnDestroyed.IsAlreadyBound(this, &UImGuiWorldDebuggerOutlinerPanel::WhenActorDestroy) == false)
		{
			DisplayActors.Add(Actor);
			Actor->OnDestroyed.AddUniqueDynamic(this, &UImGuiWorldDebuggerOutlinerPanel::WhenActorDestroy);
			bInvokeRefreshSortOrder |= true;
		}
	}));
}

void UImGuiWorldDebuggerOutlinerPanel::Unregister(AImGuiWorldDebuggerBase* WorldDebugger)
{
	for (const TWeakObjectPtr<AActor>& ActorPtr : DisplayActors)
	{
		if (AActor* Actor = ActorPtr.Get())
		{
			Actor->OnDestroyed.RemoveAll(this);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandler);
	}
	FWorldDelegates::LevelAddedToWorld.Remove(OnLevelAdd_DelegateHandle);
}

void UImGuiWorldDebuggerOutlinerPanel::Draw(AImGuiWorldDebuggerBase* WorldDebugger, float DeltaSeconds)
{
	UImGuiWorldDebuggerViewportPanel* Viewport = WorldDebugger->PanelBuilder.FindPanel<UImGuiWorldDebuggerViewportPanel>();
	if (Viewport == nullptr)
	{
		return;
	}

	if (bInvokeRefreshSortOrder)
	{
		bInvokeRefreshSortOrder = false;
		RefreshSortOrder();
	}

	ImGui::Text("Filter:");
	ImGui::SameLine();
	TArray<ANSICHAR, TInlineAllocator<256>> FilterStringArray;
	{
		const auto StringPoint = FTCHARToUTF8(*FilterString);
		FilterStringArray.SetNumZeroed(FMath::Max(256, StringPoint.Length() + 128));
		FMemory::Memcpy(FilterStringArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
	}
	if (ImGui::InputText("##FilterInput", FilterStringArray.GetData(), FilterStringArray.Num()))
	{
		FilterString = UTF8_TO_TCHAR(FilterStringArray.GetData());
		RefreshDisplayActors();
	}

	const float FooterHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); 
	if (ImGui::BeginChild("OutlinerScrollRegion", ImVec2(0.f, -FooterHeightToReserve)))
	{
		constexpr ImGuiTableFlags OutlinerTableFlags =
					ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
					| ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
					| ImGuiTableFlags_ScrollY;

		if (ImGui::BeginTable("OutlinerTable", OutlinerTableColumnID_Max, OutlinerTableFlags))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.f, OutlinerTableColumnID_Name);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.f, OutlinerTableColumnID_Type);
            ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			if (ImGuiTableSortSpecs* SortsSpecs = ImGui::TableGetSortSpecs())
			{
				if (SortsSpecs->SpecsDirty)
				{
					Orders.Empty();
					for (int Idx = 0; Idx < SortsSpecs->SpecsCount; Idx++)
					{
						const ImGuiTableColumnSortSpecs& Specs = SortsSpecs->Specs[Idx];
						Orders.Add(Specs.ColumnUserID);
						Orders.Add(Specs.SortDirection);
					}
					RefreshSortOrder();
					SortsSpecs->SpecsDirty = false;
				}
			}
			
			ImGuiListClipper Clipper;
			Clipper.Begin(DisplayActors.Num());
			while (Clipper.Step())
			{
				for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
				{
					AActor* Actor = DisplayActors[Idx].Get();
					if (Actor == nullptr)
					{
						continue;
					}
					// OutlinerTableColumnID_Name
					if (ImGui::TableNextColumn())
					{
						if (ImGui::Selectable(TCHAR_TO_UTF8(*Actor->GetName()), Viewport->SelectedActors.Contains(Actor)))
						{
							Viewport->SetSelectedEntities({ Actor });
							Viewport->FocusActor(Actor);
						}
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetName()));
							ImGui::EndTooltip();
						}
					}
					// OutlinerTableColumnID_Type
					if (ImGui::TableNextColumn())
					{
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetClass()->GetName()));
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetClass()->GetName()));
							ImGui::EndTooltip();
						}
					}
				}
			}
			Clipper.End();
			
			ImGui::EndTable();
		}
		
		ImGui::EndChild();
	}

	ImGui::Text("%d Selected | %d Filtered | %d Existed ", Viewport->SelectedActors.Num(), DisplayActors.Num(), Viewport->GetDrawableActorsCount());
}

void UImGuiWorldDebuggerOutlinerPanel::RefreshDisplayActors()
{
	for (const TWeakObjectPtr<AActor>& ActorPtr : DisplayActors)
	{
		if (AActor* Actor = ActorPtr.Get())
		{
			Actor->OnDestroyed.RemoveAll(this);
		}
	}
	DisplayActors.Empty();
	for (TActorIterator<AActor> It{ GetWorld() }; It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && CanActorDisplay(Actor))
		{
			DisplayActors.Add(Actor);
			It->OnDestroyed.AddDynamic(this, &UImGuiWorldDebuggerOutlinerPanel::WhenActorDestroy);
		}
	}
	RefreshSortOrder();
}

void UImGuiWorldDebuggerOutlinerPanel::RefreshSortOrder()
{
	const int32 RemoveCount = DisplayActors.Remove(nullptr);
	ensure(RemoveCount == 0);
	DisplayActors.Sort([&](const TWeakObjectPtr<AActor>& LHS, const TWeakObjectPtr<AActor>& RHS)
	{
		for (int32 Idx = 0; Idx < Orders.Num() / 2; ++Idx)
		{
			const EOutlinerTableColumnID OutlinerTableColumnID = static_cast<EOutlinerTableColumnID>(Orders[Idx * 2]);
			const ImGuiSortDirection_ SortDirection = static_cast<ImGuiSortDirection_>(Orders[Idx * 2 + 1]);
			if (SortDirection == ImGuiSortDirection_None)
			{
				continue;
			}
			
			switch (OutlinerTableColumnID)
			{
			case OutlinerTableColumnID_Name:
				if (LHS->GetFName() != RHS->GetFName())
				{
					return SortDirection == ImGuiSortDirection_Ascending ? LHS->GetName() < RHS->GetName() : LHS->GetName() > RHS->GetName();
				}
				break;
			case OutlinerTableColumnID_Type:
				if (LHS->GetClass() != RHS->GetClass())
				{
					return SortDirection == ImGuiSortDirection_Ascending ? LHS->GetClass()->GetClass()->GetName() < RHS->GetClass()->GetName() : LHS->GetClass()->GetName() > RHS->GetClass()->GetName();
				}
				break;
			default:
				checkNoEntry();
				break;
			}
		}
		return LHS->GetName() < RHS->GetName();
	});
}

bool UImGuiWorldDebuggerOutlinerPanel::CanActorDisplay(const AActor* Actor) const
{
	if (FilterString.IsEmpty())
	{
		return true;
	}
	
	if (Actor->GetName().Contains(FilterString))
	{
		return true;
	}

	return false;
}

void UImGuiWorldDebuggerOutlinerPanel::WhenActorDestroy(AActor* Actor)
{
	DisplayActors.RemoveSingle(Actor);
}
