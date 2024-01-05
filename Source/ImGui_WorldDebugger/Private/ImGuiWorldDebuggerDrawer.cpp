// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWorldDebuggerDrawer.h"

#include "GameFramework/Actor.h"


UImGuiWorldDebuggerDrawer_Default::UImGuiWorldDebuggerDrawer_Default()
{
	DrawActor = AActor::StaticClass();
	Color = FLinearColor::Gray.ToFColor(true);
	Radius = 50.f;
}
