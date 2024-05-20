// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerViewportPanel.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiWorldDebuggerBase.h"
#include "ImGuiWorldDebuggerDrawer.h"
#include "ImGuiWorldDebuggerLayout.h"
#include "ImGuiWorldDebuggerPanel.h"
#include "UnrealImGuiPropertyDetails.h"
#include "UnrealImGuiStat.h"
#include "GameFramework/PlayerController.h"
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
	return Owner && Owner->IsA<AImGuiWorldDebuggerBase>();
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

void UImGuiWorldDebuggerViewportActorExtent::SetSelectedEntities(const TSet<TWeakObjectPtr<AActor>>& NewSelectedActors)
{
	SelectedActors = NewSelectedActors;
#if WITH_EDITOR
	EditorSelectActors.ExecuteIfBound(GetWorld(), NewSelectedActors);
#endif
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
	}));
}

void UImGuiWorldDebuggerViewportActorExtent::Unregister(UObject* Owner, UUnrealImGuiViewportBase* Viewport)
{
	if (const UWorld* World = Owner->GetWorld())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);
		World->RemoveOnActorDestroyededHandler(OnActorDestroyedHandle);
	}
	FWorldDelegates::LevelAddedToWorld.Remove(OnLevelAddHandle);
}

void UImGuiWorldDebuggerViewportActorExtent::DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty)
{

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
	// TODO：缓存ViewBounds，对视口进行缩放
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
				return;
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

bool UImGuiWorldDebuggerViewportActorExtent::IsShowActorsByFilter(const AActor* Actor) const
{
	using namespace UnrealImGui::Viewport;
	if (GetViewport()->FilterType == EFilterType::TypeFilter)
	{
		if (UClass* FilterClass = GetViewport()->FilterClass.Get())
		{
			return Actor->IsA(FilterClass);
		}
	}
	return false;
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

void UImGuiWorldDebuggerViewportActorExtent::DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWorldDebuggerViewportExtent_Draw"), STAT_ImGuiWorldDebuggerViewportExtent_Draw, STATGROUP_ImGui);
	
	UWorld* World = GetWorld();
	check(World);

	AActor* TopSelectedActor = nullptr;
	const float Zoom = ViewportContext.Zoom;
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
			if (IsSelected || ViewportContext.ViewBounds.Intersect(ActorBounds))
			{
				if (IsSelected == false)
				{
					using namespace UnrealImGui::Viewport;
					if (ViewportContext.Viewport->FilterType != EFilterType::None && ViewportContext.Viewport->FilterType != EFilterType::NameFilter)
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
			return (LHS_Location - ViewportContext.MouseWorldPos).SizeSquared() < (RHS_Location - ViewportContext.MouseWorldPos).SizeSquared();
		});
	}

#if WITH_EDITOR
	// 编辑器下同步选择
	if (ViewportContext.bIsSelectDragging == false)
	{
		static TSet<TWeakObjectPtr<AActor>> PreSelectedActors;
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
		for (const TWeakObjectPtr<AActor>& ActorPtr : SelectedActors)
		{
			if (AActor* Actor = ActorPtr.Get())
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
		if (ImGui::BeginItemTooltip())
		{
			ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstActor->GetName()));
			ImGui::EndTooltip();
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
		DrawDetailTable("Actor", GetTopClass(FilteredSelectedActors), FilteredSelectedActors, &DetailsFilter);
	}
}

AActor* UImGuiWorldDebuggerViewportActorExtent::GetFirstSelectActor() const
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
