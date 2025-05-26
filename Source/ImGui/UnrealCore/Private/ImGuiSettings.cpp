// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiSettings.h"

void UImGuiPerUserSettings::RecordRecentlyOpenPanel(const TSoftClassPtr<UObject>& PanelClass)
{
	RecentlyOpenPanels.RemoveSingle(PanelClass);
	RecentlyOpenPanels.Insert(PanelClass, 0);
	if (RecentlyOpenPanels.Num() > MaxRecordRecentlyNum)
	{
		RecentlyOpenPanels.RemoveAt(MaxRecordRecentlyNum, RecentlyOpenPanels.Num() - MaxRecordRecentlyNum);
	}
	SaveConfig();
}

#if WITH_EDITOR
void UImGuiPerUserSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SaveConfig();
}
#endif

UImGuiSettings::UImGuiSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("ImGui_WS_Settings");

	PreUserSettings = GetMutableDefault<UImGuiPerUserSettings>();
}

#if WITH_EDITOR
UImGuiSettings::FOnPostEditChangeProperty UImGuiSettings::OnPostEditChangeProperty;

void UImGuiSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnPostEditChangeProperty.Broadcast(this, PropertyChangedEvent);
}
#endif
