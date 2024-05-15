// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerOutlinerPanel.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerViewportPanel.h"
#include "UnrealImGuiPanelBuilder.h"
#include "UnrealImGuiString.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

UImGuiWorldDebuggerOutlinerPanel::UImGuiWorldDebuggerOutlinerPanel()
	: bInvokeRefreshSortOrder(false)
{
	Title = LOCTEXT("Outliner", "Outliner");
	Categories = { LOCTEXT("ViewportCategory", "Viewport") };
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Outliner }
	};
}

void UImGuiWorldDebuggerOutlinerPanel::Register(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	const UWorld* World = GetWorld();
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
			}
		}
		bInvokeRefreshSortOrder |= true;
	});
	OnActorSpawnedHandler = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [this](AActor* Actor)
	{
		if (CanActorDisplay(Actor))
		{
			DisplayActors.Add(Actor);
			bInvokeRefreshSortOrder |= true;
		}
	}));
	OnActorDestroyedHandler = World->AddOnActorDestroyedHandler(FOnActorDestroyed::FDelegate::CreateWeakLambda(this, [this](AActor* Actor)
	{
		DisplayActors.RemoveSingle(Actor);
	}));
}

void UImGuiWorldDebuggerOutlinerPanel::Unregister(UObject* Owner, UUnrealImGuiPanelBuilder* Builder)
{
	if (UWorld* World = GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandler);
		World->RemoveOnActorDestroyededHandler(OnActorDestroyedHandler);
	}
	FWorldDelegates::LevelAddedToWorld.Remove(OnLevelAdd_DelegateHandle);
}

void UImGuiWorldDebuggerOutlinerPanel::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	UImGuiWorldDebuggerViewportPanel* Viewport = Builder->FindPanel<UImGuiWorldDebuggerViewportPanel>();
	if (Viewport == nullptr)
	{
		return;
	}
	UImGuiWorldDebuggerViewportActorExtent* ViewportExtent = Viewport->FindExtent<UImGuiWorldDebuggerViewportActorExtent>();
	if (ViewportExtent == nullptr)
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
	UnrealImGui::FUTF8String UTF8String{ FilterString };
	if (ImGui::InputText("##FilterInput", UTF8String))
	{
		FilterString = UTF8String.ToString();
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
						if (ImGui::Selectable(TCHAR_TO_UTF8(*Actor->GetName()), ViewportExtent->SelectedActors.Contains(Actor)))
						{
							if (!ImGui::GetIO().KeyCtrl)
							{
								Viewport->ResetSelection();
								ViewportExtent->SetSelectedEntities({ Actor });
								ViewportExtent->FocusActor(Actor);
							}
							else
							{
								ViewportExtent->SelectedActors.Add(Actor);
								ViewportExtent->SetSelectedEntities(MoveTemp(ViewportExtent->SelectedActors));
							}
						}
						if (ImGui::BeginItemTooltip())
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetName()));
							ImGui::EndTooltip();
						}
					}
					// OutlinerTableColumnID_Type
					if (ImGui::TableNextColumn())
					{
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetClass()->GetName()));
						if (ImGui::BeginItemTooltip())
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetClass()->GetName()));
							ImGui::EndTooltip();
						}
					}
				}
			}
			Clipper.End();
			
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();

	ImGui::Text("%d Selected | %d Filtered | %d Existed ", ViewportExtent->SelectedActors.Num(), DisplayActors.Num(), ViewportExtent->GetDrawableActorsCount());
}

void UImGuiWorldDebuggerOutlinerPanel::RefreshDisplayActors()
{
	DisplayActors.Empty();
	for (TActorIterator<AActor> It{ GetWorld() }; It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && CanActorDisplay(Actor))
		{
			DisplayActors.Add(Actor);
		}
	}
	RefreshSortOrder();
}

void UImGuiWorldDebuggerOutlinerPanel::RefreshSortOrder()
{
	DisplayActors.Sort([&](const TWeakObjectPtr<AActor>& LHSPtr, const TWeakObjectPtr<AActor>& RHSPtr)
	{
		const AActor* LHS = LHSPtr.Get();
		const AActor* RHS = RHSPtr.Get();
		if (LHS == nullptr || RHS == nullptr)
		{
			return RHS == nullptr;
		}
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
#if WITH_EDITOR
	if (Actor->bIsEditorPreviewActor)
	{
		return false;
	}
#endif

	if (FilterString.IsEmpty())
	{
		return true;
	}
	
	if (Actor->GetName().ToLower().Contains(FilterString.ToLower()))
	{
		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
