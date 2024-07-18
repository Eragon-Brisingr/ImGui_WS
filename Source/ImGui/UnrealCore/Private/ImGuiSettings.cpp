// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiSettings.h"

#if WITH_EDITOR
void UImGuiPerUserSettingsSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SaveConfig();
}
#endif

UImGuiSettings::UImGuiSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("ImGui_WS_Settings");

	PreUserSettings = GetMutableDefault<UImGuiPerUserSettingsSettings>();
}

#if WITH_EDITOR
void UImGuiSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnPostEditChangeProperty.Broadcast(this, PropertyChangedEvent);
}
#endif
