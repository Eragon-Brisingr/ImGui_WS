// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerViewportPanel.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "ImGuiWorldDebuggerManager.h"
#include "ImGuiWorldDebuggerDrawer.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "UnrealImGuiPropertyDetails.h"
#include "UnrealImGuiStat.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UObject/UObjectIterator.h"

UImGuiWorldDebuggerViewportPanel::UImGuiWorldDebuggerViewportPanel()
{
	Title = TEXT("Viewport");
	Categories = { TEXT("Viewport") };
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Viewport }
	};
}

bool UImGuiWorldDebuggerViewportPanel::ShouldCreatePanel(UObject* Owner) const
{
	return Owner && Owner->IsA<UImGuiWorldDebuggerManager>();
}

void UImGuiWorldDebuggerViewportPanel::DrawCurrentViewFrustum(UObject* Owner, const FUnrealImGuiViewportContext& Context)
{
	UWorld* World = GetWorld();
	auto Config = GetConfigObject<ThisClass>();
	// Draw player location
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PlayerController = It->Get())
		{
			// Draw Camera
			FVector PlayerViewLocation;
			FRotator PlayerViewRotation;
			PlayerController->GetPlayerViewPoint(PlayerViewLocation, PlayerViewRotation);
			float Fov;
			if (PlayerController->PlayerCameraManager && PlayerController->PlayerCameraManager->GetCameraCacheTime() > 0.f)
			{
				Fov = PlayerController->PlayerCameraManager->GetFOVAngle();
			}
			else if (auto CDO = PlayerController->PlayerCameraManagerClass.GetDefaultObject())
			{
				Fov = CDO->DefaultFOV;
			}
			else
			{
				Fov = 70.f;
			}
			Context.DrawViewFrustum(FTransform{ PlayerViewRotation, PlayerViewLocation }, FMath::Tan(Fov / 2.f), Config->NearPlaneDistance, Config->FarPlaneDistance, FColor{ 0, 127, 255, 127 });
			const FVector2D ViewLocation2D{ PlayerViewLocation };
			Context.DrawCircleFilled(ViewLocation2D, 3.f / CurrentZoom, FColor::White);
			if (const APlayerState* PlayerState = PlayerController->PlayerState)
			{
				Context.DrawText(ViewLocation2D + FVector2D{ -20.f / CurrentZoom, 6.f / CurrentZoom }, PlayerState->GetPlayerName(), FColor::White);
			}
		}
	}
	if (World->GetGameViewport() && World->GetGameViewport()->IsSimulateInEditorViewport())
	{
		Super::DrawCurrentViewFrustum(Owner, Context);
	}
}

UImGuiWorldDebuggerViewportActorExtent::UImGuiWorldDebuggerViewportActorExtent()
{
	ExtentName = TEXT("Actor");
	Priority = 1;
}

bool UImGuiWorldDebuggerViewportActorExtent::ShouldCreateExtent(UObject* Owner, UUnrealImGuiViewportBase* Viewport) const
{
	return Viewport && Viewport->IsA<UImGuiWorldDebuggerViewportPanel>();
}

void UImGuiWorldDebuggerViewportActorExtent::Register(UObject* Owner, UUnrealImGuiViewportBase* Viewport)
{
	UWorld* World = Owner->GetWorld();
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

	auto TryAddActorToDraw = [this, Viewport](AActor* Actor)
	{
		if (Actor == nullptr || Actor->IsA<UImGuiWorldDebuggerManager>())
		{
			return;
		}

		for (const UClass* TestClass = Actor->GetClass(); TestClass != UObject::StaticClass(); TestClass = TestClass->GetSuperClass())
		{
			if (const TSubclassOf<UImGuiWorldDebuggerDrawerBase>* Drawer = DrawerMap.Find(TestClass))
			{
				DrawableActors.Add(Actor, *Drawer);
				if (CanAddToDraw(Viewport, Actor, Drawer->GetDefaultObject()))
				{
					ActorsToDraw.Add(Actor, *Drawer);
				}
				return;
			}
		}
	};
	for (TActorIterator<AActor> It{World}; It; ++It)
	{
		TryAddActorToDraw(*It);
	}
	OnLevelAddHandle = FWorldDelegates::LevelAddedToWorld.AddWeakLambda(this, [this, TryAddActorToDraw](ULevel* Level, UWorld* World)
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
	OnActorSpawnedHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [TryAddActorToDraw](AActor* Actor)
	{
#if WITH_EDITOR
		if (Actor->bIsEditorPreviewActor)
		{
			return;
		}
#endif
		TryAddActorToDraw(Actor);
	}));
	OnActorDestroyedHandle = World->AddOnActorDestroyedHandler(FOnActorDestroyed::FDelegate::CreateWeakLambda(this, [this](AActor* Actor)
	{
		DrawableActors.Remove(Actor);
		ActorsToDraw.Remove(Actor);
	}));
}

void UImGuiWorldDebuggerViewportActorExtent::Unregister(UObject* Owner, UUnrealImGuiViewportBase* Viewport)
{
	if (const UWorld* World = Owner->GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);
		World->RemoveOnActorDestroyedHandler(OnActorDestroyedHandle);
	}
	FWorldDelegates::LevelAddedToWorld.Remove(OnLevelAddHandle);
}

void UImGuiWorldDebuggerViewportActorExtent::DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty)
{

}

void UImGuiWorldDebuggerViewportActorExtent::DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebuggerViewportExtent_Draw"), STAT_ImGuiWorldDebuggerViewportExtent_Draw, STATGROUP_ImGui);
	
	UWorld* World = GetWorld();
	check(World);

	AActor* TopSelectedActor = nullptr;
	// Draw actor location
	{
		const float Zoom = ViewportContext.Zoom;
		constexpr float ScreenMinRadius = 2.f;
		const float MinRadius = ScreenMinRadius / Zoom;
		auto DrawActor = [&](AActor* Actor, const UImGuiWorldDebuggerDrawerBase* Drawer, bool IsSelected)
		{
			const FVector Location = Actor->GetActorLocation();
			const FRotator Rotation = Actor->GetActorRotation();
			const float Radius = Drawer->Radius < MinRadius ? MinRadius : Drawer->Radius;

			const FBox2D ActorBounds = FBox2D{ FVector2D{Location} - FVector2D(Radius), FVector2D{Location} + FVector2D(Radius) };
			if (IsSelected || ViewportContext.ViewBounds.Intersect(ActorBounds))
			{
				if (IsSelected == false)
				{
					if (Drawer->bAlwaysDebuggerDraw == false && Drawer->Radius < MinRadius)
					{
						return;
					}
				}

				if (ViewportContext.bIsSelectDragging && ViewportContext.SelectDragBounds.Intersect(ActorBounds))
				{
					SelectedActors.Add(Actor);
					TopSelectedActor = Actor;
				}
				else if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseReleased[ImGuiMouseButton_Left])
				{
					const float DistanceSquared = (FVector2D{ Location } - ViewportContext.MouseWorldPos).SizeSquared();
					if (DistanceSquared < Radius * Radius)
					{
						TopSelectedActor = Actor;
					}
				}

				FGuardValue_Bitfield(ViewportContext.bIsSelected, IsSelected);
				Drawer->DrawImGuiDebuggerExtendInfo(Actor, ViewportContext);

				ViewportContext.DrawCircleFilled(FVector2D(Location), Radius, Drawer->Color, 8);
				if ((ViewportContext.MouseWorldPos - FVector2D(Location)).SizeSquared() < Radius * Radius)
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
					ViewportContext.DrawLine(FVector2D(Location), FVector2D(Location + Rotation.Vector() * Radius * 1.5f), FColor::Black, Thickness);
				}

				if (IsSelected)
				{
					const FColor SelectedColor = FLinearColor{ 0.8f, 0.4f, 0.f }.ToFColor(true);
					const float Thickness = FMath::GetMappedRangeValueClamped(TRange<float>{ 4.f, 16.f }, TRange<float>{ 2.f, 8.f }, Radius * Zoom);
					ViewportContext.DrawCircle(FVector2D(Location), Radius, SelectedColor, 8, Thickness);
				}
			}
		};
		for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : ActorsToDraw)
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
		for (AActor* Actor : TSet<TObjectPtr<AActor>>(SelectedActors))
		{
			if (Actor)
			{
				if (const TSubclassOf<UImGuiWorldDebuggerDrawerBase>* Drawer = DrawableActors.Find(Actor))
				{
					DrawActor(Actor, Drawer->GetDefaultObject(), true);
				}
			}
		}
	}

	// Sort selected actors by distance
	if (TopSelectedActor)
	{
		// Clear invalid actors
		{
			TArray<TObjectPtr<AActor>> SelectedWorldActorsArray = SelectedActors.Array();
			SelectedWorldActorsArray.RemoveAll([](const AActor* E) { return E == nullptr; });
			SelectedActors = TSet<TObjectPtr<AActor>>{ SelectedWorldActorsArray };
		}

		SelectedActors.Add(TopSelectedActor);
		SelectedActors.Sort([&](const AActor& LHS, const AActor& RHS)
		{
			const FVector2D LHS_Location = FVector2D(LHS.GetActorLocation());
			const FVector2D RHS_Location = FVector2D(RHS.GetActorLocation());
			return (LHS_Location - ViewportContext.MouseWorldPos).SizeSquared() < (RHS_Location - ViewportContext.MouseWorldPos).SizeSquared();
		});
	}

#if WITH_EDITOR
	// sync selection to editor
	if (ViewportContext.bIsSelectDragging == false)
	{
		static TSet<TObjectPtr<AActor>> PreSelectedActors;
		if (PreSelectedActors.Num() != SelectedActors.Num() || PreSelectedActors.Difference(SelectedActors).Num() > 0 || SelectedActors.Difference(PreSelectedActors).Num() > 0)
		{
			EditorSelectActors.ExecuteIfBound(World, SelectedActors);
			PreSelectedActors = SelectedActors;
		}
	}
#endif
}

void UImGuiWorldDebuggerViewportActorExtent::DrawDetailsPanel(UObject* Owner, UImGuiWorldDebuggerDetailsPanel* DetailsPanel)
{
	UnrealImGui::TObjectArray<AActor> FilteredSelectedActors;
	{
		FilteredSelectedActors.Reset(SelectedActors.Num());
		for (AActor* Actor : SelectedActors)
		{
			if (Actor)
			{
				FilteredSelectedActors.Add(Actor);
			}
		}
	}
	if (FilteredSelectedActors.Num() == 0)
	{
		return;
	}

	const AActor* FirstActor = FilteredSelectedActors[0];
	if (FilteredSelectedActors.Num() == 1)
	{
		ImGui::Text("Actor Name: %s", TCHAR_TO_UTF8(*FirstActor->GetName()));
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstActor->GetName()));
		}
	}
	else
	{
		ImGui::Text("%d Actors", FilteredSelectedActors.Num());
	}

	static UnrealImGui::FDetailsFilter DetailsFilter;
	DetailsFilter.Draw();
	if (FirstActor)
	{
		if (auto ChildWindow = ImGui::FChildWindow("ActorDetails"))
		{
			DrawDetailTable("Actor", GetTopClass(FilteredSelectedActors), FilteredSelectedActors, &DetailsFilter);
		}
	}
}

void UImGuiWorldDebuggerViewportActorExtent::SetSelectedEntities(const TSet<TObjectPtr<AActor>>& NewSelectedActors)
{
	SelectedActors = NewSelectedActors;
#if WITH_EDITOR
	EditorSelectActors.ExecuteIfBound(GetWorld(), NewSelectedActors);
#endif
}

AActor* UImGuiWorldDebuggerViewportActorExtent::GetFirstSelectActor() const
{
	for (AActor* Actor : SelectedActors)
	{
		if (Actor)
		{
			return Actor;
		}
	}
	return nullptr;
}

void UImGuiWorldDebuggerViewportActorExtent::FocusActor(AActor* Actor)
{
	if (IsValid(Actor) == false)
	{
		return;
	}
	GetViewport()->ViewLocation = FVector2D(Actor->GetActorLocation());
	SelectedActors.Reset();
	SelectedActors.Add(Actor);
}

void UImGuiWorldDebuggerViewportActorExtent::FocusActors(const TArray<AActor*>& Actors)
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
	GetViewport()->ViewLocation = CenterViewLocation;
	SelectedActors.Reset();
	for (AActor* Actor : Actors)
	{
		SelectedActors.Add(Actor);
	}
}

void UImGuiWorldDebuggerViewportActorExtent::WhenFilterStringChanged(UUnrealImGuiViewportBase* Viewport, const FString& FilterString)
{
	using namespace UnrealImGui::Viewport;
	FString FilterType;
	FString FilterValue;
	if (FilterString.Split(TEXT(":"), &FilterType, &FilterValue))
	{
		Viewport->FilterType = 0;
		if (FilterType == EFilterType::TypeFilter_FilterType)
		{
			Viewport->FilterClass = UClass::TryFindTypeSlow<UClass>(FilterValue);
			if (Viewport->FilterClass.IsValid() == false)
			{
				Viewport->FilterClass = LoadObject<UClass>(nullptr, *FilterValue);
			}
			if (Viewport->FilterClass.IsValid())
			{
				Viewport->FilterType = EFilterType::TypeFilter;
			}
		}
	}
	else if (FilterString.IsEmpty())
	{
		Viewport->FilterType = EFilterType::None;
	}
	else
	{
		Viewport->FilterType = EFilterType::NameFilter;
	}

	ActorsToDraw.Empty();
	for (const auto& [Actor, Drawer] : DrawableActors)
	{
		if (CanAddToDraw(Viewport, Actor.Get(), Drawer.GetDefaultObject()))
		{
			ActorsToDraw.Add(Actor, Drawer);
		}
	}
}

void UImGuiWorldDebuggerViewportActorExtent::DrawFilterTooltip(UUnrealImGuiViewportBase* Viewport)
{
	ImGui::Text("SpecialFilterTag\n  type:(Filter actors by type)");
}

void UImGuiWorldDebuggerViewportActorExtent::DrawFilterPopup(UUnrealImGuiViewportBase* Viewport)
{
	using namespace UnrealImGui::Viewport;
	const ImGuiIO& IO = ImGui::GetIO();

	const UWorld* World = GetWorld();
	FString FilterType;
	FString FilterValue;
	const FString& FilterActorString = Viewport->FilterString;
	if (FilterActorString.Split(TEXT(":"), &FilterType, &FilterValue))
	{
		if (FilterType == EFilterType::TypeFilter_FilterType)
		{
			TSet<UClass*> Classes;
			for (const TPair<TWeakObjectPtr<AActor>, TSubclassOf<UImGuiWorldDebuggerDrawerBase>>& Pair : DrawableActors)
			{
				if (AActor* Actor = Pair.Key.Get())
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
			for (const UClass* Class : PopupClasses)
			{
				const FString ClassPathName = Class->GetPathName();
				if (FilterValue.IsEmpty() || ClassPathName.Contains(FilterValue))
				{
					ImGui::Selectable(TCHAR_TO_UTF8(*Class->GetName()));
					if (ImGui::BeginItemTooltip())
					{
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class->GetFullName()));
						ImGui::EndTooltip();
						if (IO.MouseDown[ImGuiMouseButton_Left])
						{
							const FString FilterString = FString::Printf(TEXT("%s:%s"), *EFilterType::TypeFilter_FilterType, *ClassPathName);
							Viewport->FilterString = FilterString;
							WhenFilterStringChanged(Viewport, FilterString);
							FocusEntitiesByFilter(Viewport);
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
				if (ImGui::BeginItemTooltip())
				{
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetFullName(World)));
					ImGui::EndTooltip();
					if (IO.MouseDown[ImGuiMouseButton_Left])
					{
						Viewport->FilterString = Actor->GetName();
						WhenFilterStringChanged(Viewport, Actor->GetName());
						FocusActor(Actor);
					}
				}
			}
		}
	}
}

void UImGuiWorldDebuggerViewportActorExtent::FocusEntitiesByFilter(UUnrealImGuiViewportBase* Viewport)
{
	using namespace UnrealImGui::Viewport;
	if (Viewport->FilterType == EFilterType::NameFilter)
	{
		const FName ActorName = *Viewport->FilterString;
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

bool UImGuiWorldDebuggerViewportActorExtent::IsShowActorsByFilter(const UUnrealImGuiViewportBase* Viewport, const AActor* Actor) const
{
	if (Viewport->FilterType == UnrealImGui::Viewport::EFilterType::TypeFilter)
	{
		if (UClass* FilterClass = GetViewport()->FilterClass.Get())
		{
			return Actor->IsA(FilterClass);
		}
	}
	return false;
}

bool UImGuiWorldDebuggerViewportActorExtent::CanAddToDraw(const UUnrealImGuiViewportBase* Viewport, const AActor* Actor, const UImGuiWorldDebuggerDrawerBase* Drawer) const
{
	if (Actor == nullptr)
	{
		return false;
	}
	if (Viewport->FilterType != UnrealImGui::Viewport::EFilterType::None && Viewport->FilterType != UnrealImGui::Viewport::EFilterType::NameFilter)
	{
		if (IsShowActorsByFilter(Viewport, Actor) == false)
		{
			return false;
		}
	}
	else
	{
		if (Drawer->GetClass() == UImGuiWorldDebuggerDrawer_Default::StaticClass())
		{
			return false;
		}
	}
	return true;
}

#if WITH_EDITOR
void UImGuiWorldDebuggerViewportActorExtent::WhenEditorSelectionChanged(const TArray<AActor*>& SelectedActors)
{
	for (TObjectIterator<UImGuiWorldDebuggerViewportActorExtent> It; It; ++It)
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

UImGuiWorldDebuggerViewportActorExtent::FEditorSelectActors UImGuiWorldDebuggerViewportActorExtent::EditorSelectActors;
#endif
