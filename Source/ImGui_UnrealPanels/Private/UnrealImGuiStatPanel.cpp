// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiStatPanel.h"

#include "imgui.h"
#include "UnrealImGuiStat.h"
#include "Engine/Engine.h"
#include "Stats/StatsData.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

UUnrealImGuiStatPanel::UUnrealImGuiStatPanel()
{
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	DefaultState = { false, true };
	Title = LOCTEXT("Stat", "Stat");
	Categories = { LOCTEXT("ToolsCategory", "Tools") };
}

void UUnrealImGuiStatPanel::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
#if STATS
	// From RenderGroupedWithHierarchy
	const FGameThreadStatsData* ViewData = FLatestGameThreadStatsData::Get().Latest;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiStatDevice_Draw"), STAT_UnrealImGuiStatDevice_Draw, STATGROUP_ImGui);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Stat Group"))
		{
			if (ImGui::MenuItem("Deactivate All"))
			{
				if (ViewData)
				{
					for (const FName& GroupName : ViewData->GroupNames)
					{
						UWorld* World = Owner->GetWorld();
						const FString StatName = GroupName.ToString().RightChop(sizeof("STATGROUP_") - 1);
						GEngine->Exec(World, *FString::Printf(TEXT("Stat %s"), *StatName));
					}
				}
			}
			ImGui::Separator();
			{
				bool bIsActivated = ViewData && ViewData->GroupNames.Contains(TEXT("STATGROUP_Slow"));
				if (ImGui::Checkbox("Slow", &bIsActivated))
				{
					UWorld* World = Owner->GetWorld();
					GEngine->Exec(World, TEXT("Stat Slow"));
				}
			}
			const TSet<FName>& StatGroupNames = FStatGroupGameThreadNotifier::Get().StatGroupNames;
			for (const FName& StatGroupName : StatGroupNames)
			{
				bool bIsActivated = ViewData && ViewData->GroupNames.Contains(StatGroupName);
				const FString StatName = StatGroupName.ToString().RightChop(sizeof("STATGROUP_") - 1);
				if (ImGui::Checkbox(TCHAR_TO_UTF8(*StatName), &bIsActivated))
				{
					UWorld* World = Owner->GetWorld();
					GEngine->Exec(World, *FString::Printf(TEXT("Stat %s"), *StatName));
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	
	if (!ViewData || !ViewData->bRenderStats)
	{
		return;
	}

	// Render ComplexStat groups.
	for( int32 GroupIndex = 0; GroupIndex < ViewData->ActiveStatGroups.Num(); ++GroupIndex )
	{
		const FActiveStatGroupInfo& StatGroup = ViewData->ActiveStatGroups[GroupIndex];
		
		const FName& GroupName = ViewData->GroupNames[GroupIndex];
		const FString& GroupDesc = ViewData->GroupDescriptions[GroupIndex];

		if (ImGui::CollapsingHeader(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s [%s]"), *GroupDesc, *GroupName.GetPlainNameString())), ImGuiTreeNodeFlags_DefaultOpen))
		{
			constexpr auto TableFlags = ImGuiTableFlags_Resizable
										| ImGuiTableFlags_Reorderable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Hideable
										| ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			auto RenderCycleInfo = [](const FComplexStatMessage& ComplexStat)
			{
				// CComplexStatCount
				ImGui::TableNextColumn();
				ImGui::Text("%u", ComplexStat.GetValue_CallCount(EComplexStatField::IncAve));

				// InclusiveAvg
				ImGui::TableNextColumn();
				ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::IncAve)));

				// InclusiveMax
				ImGui::TableNextColumn();
				ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::IncMax)));

				// ExclusiveAvg
				ImGui::TableNextColumn();
				ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::ExcAve)));
				
				// ExclusiveMax
				ImGui::TableNextColumn();
				ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::ExcMax)));
			};
			
			if (StatGroup.HierAggregate.Num() && ImGui::BeginTable("CycleHierarchyStatTable", 6, TableFlags))
			{
				ImGui::TableSetupColumn("Cycle counters (hierarchy)", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("CallCount", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("InclusiveAvg", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("InclusiveMax", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ExclusiveAvg", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ExclusiveMax", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				ImGuiListClipper Clipper;
				Clipper.Begin(StatGroup.HierAggregate.Num());
				while (Clipper.Step())
				{
					for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
					{
						const FComplexStatMessage& ComplexStat = StatGroup.HierAggregate[Idx];

						// Cycle counters
						ImGui::TableNextColumn();
						const FString StatDesc = ComplexStat.GetDescription();
						const FString StatDisplay = StatDesc.Len() == 0 ? ComplexStat.GetShortName().GetPlainNameString() : StatDesc;
						const float IndentWidth = StatGroup.Indentation[Idx] * 4.f;
						ImGui::Indent(IndentWidth);
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*StatDisplay));
						ImGui::Unindent(IndentWidth);

						RenderCycleInfo(ComplexStat);
					}
				}
				Clipper.End();
				ImGui::EndTable();
			}
			
			if (StatGroup.FlatAggregate.Num() && ImGui::BeginTable("CycleFlatStatTable", 6, TableFlags))
			{
				ImGui::TableSetupColumn("Cycle counters (flat)", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("CallCount", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("InclusiveAvg", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("InclusiveMax", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ExclusiveAvg", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ExclusiveMax", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				ImGuiListClipper Clipper;
				Clipper.Begin(StatGroup.FlatAggregate.Num());
				while (Clipper.Step())
				{
					for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
					{
						const FComplexStatMessage& ComplexStat = StatGroup.FlatAggregate[Idx];

						// Cycle counters
						ImGui::TableNextColumn();
						const FString StatDesc = ComplexStat.GetDescription();
						const FString StatDisplay = StatDesc.Len() == 0 ? ComplexStat.GetShortName().GetPlainNameString() : StatDesc;
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*StatDisplay));

						RenderCycleInfo(ComplexStat);
					}
				}
				Clipper.End();
				ImGui::EndTable();
			}

			if (StatGroup.MemoryAggregate.Num() && ImGui::BeginTable("MemoryStatTable", 5, TableFlags))
			{
				ImGui::TableSetupColumn("Memory Counters", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("UsedMax", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Mem%", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("MemPool", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Pool Capacity", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				ImGuiListClipper Clipper;
				Clipper.Begin(StatGroup.MemoryAggregate.Num());
				while (Clipper.Step())
				{
					for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
					{
						const FComplexStatMessage& ComplexStat = StatGroup.MemoryAggregate[Idx];

						// Memory Counters
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*ComplexStat.GetDescription()));

						const float MaxMemUsed = ComplexStat.GetValue_double(EComplexStatField::IncMax);
						// UsedMax
						ImGui::TableNextColumn();
						ImGui::Text("%.2f MB",  MaxMemUsed / (1024.0 * 1024.0));

						const FPlatformMemory::EMemoryCounterRegion Region = FPlatformMemory::EMemoryCounterRegion(ComplexStat.NameAndInfo.GetField<EMemoryRegion>());
						const auto PoolCapacity = ViewData->PoolCapacity.Find(Region);
						// Mem%
						ImGui::TableNextColumn();
						if (PoolCapacity)
						{
							ImGui::Text("%.0f%%", float(100.0 * MaxMemUsed / double(*PoolCapacity)));
						}

						// MemPool
						ImGui::TableNextColumn();
						if (const auto PoolAbbreviation = ViewData->PoolAbbreviation.Find(Region))
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(**PoolAbbreviation));
						}

						// Pool Capacity
						ImGui::TableNextColumn();
						if (PoolCapacity)
						{
							ImGui::Text("%.2f MB", double(*PoolCapacity) / (1024.0 * 1024.0));
						}
					}
				}
				Clipper.End();
				ImGui::EndTable();
			}

			if (StatGroup.CountersAggregate.Num() && ImGui::BeginTable("CountersStatTable", 4, TableFlags))
			{
				ImGui::TableSetupColumn("Counters", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Average", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				ImGuiListClipper Clipper;
				Clipper.Begin(StatGroup.CountersAggregate.Num());
				while (Clipper.Step())
				{
					for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
					{
						const FComplexStatMessage& ComplexStat = StatGroup.CountersAggregate[Idx];
						const bool bIsCycle = ComplexStat.NameAndInfo.GetFlag(EStatMetaFlags::IsCycle);
						
						// Draw the label
						ImGui::TableNextColumn();
						const FString StatDesc = ComplexStat.GetDescription();
						const FString StatDisplay = StatDesc.Len() == 0 ? ComplexStat.GetShortName().GetPlainNameString() : StatDesc;
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*StatDisplay));

						if(bIsCycle)
						{
							// Average
							ImGui::TableNextColumn();
							ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::IncAve)));

							// Max
							ImGui::TableNextColumn();
							ImGui::Text("%1.2f ms", FPlatformTime::ToMilliseconds(ComplexStat.GetValue_Duration(EComplexStatField::IncMax)));

							ImGui::TableNextColumn();
							ImGui::Text("-");
							
							continue;
						}
						
						auto FormatStatValueFloat = [](const float Value)
						{
							const float QuantizedValue = FMath::RoundToFloat(Value * 100.0f) / 100.0f;
							const float Frac = FMath::Frac(QuantizedValue);
							// #TODO: Move to stats thread, add support for int64 type, int32 may not be sufficient all the time.
							const int32 Integer = FMath::FloorToInt(QuantizedValue);
							const FString IntString = FString::FormatAsNumber(Integer);
							const FString FracString = FString::Printf(TEXT("%0.2f"), Frac);
							const FString Result = FString::Printf(TEXT("%s.%s"), *IntString, *FracString.Mid(2));
							return Result;
						};
						auto FormatStatValueInt64 = [](const int64 Value)
						{
							const FString IntString = FString::FormatAsNumber((int32)Value);
							return IntString;
						};

						// Average
						ImGui::TableNextColumn();
						const bool bDisplayComplexStat = ComplexStat.NameAndInfo.GetFlag(EStatMetaFlags::ShouldClearEveryFrame);
						if(bDisplayComplexStat)
						{
							// Append the average.
							if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_double)
							{
								const FString ValueFormatted = FormatStatValueFloat(ComplexStat.GetValue_double(EComplexStatField::IncAve));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
							}
							else if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_int64)
							{
								const FString ValueFormatted = FormatStatValueInt64(ComplexStat.GetValue_int64(EComplexStatField::IncAve));
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
							}
						}

						// Max
						ImGui::TableNextColumn();
						if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_double)
						{
							const FString ValueFormatted = FormatStatValueFloat(ComplexStat.GetValue_double(EComplexStatField::IncMax));
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
						}
						else if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_int64)
						{
							const FString ValueFormatted = FormatStatValueInt64(ComplexStat.GetValue_int64(EComplexStatField::IncMax));
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
						}

						// Min
						ImGui::TableNextColumn();
						if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_double)
						{
							const FString ValueFormatted = FormatStatValueFloat(ComplexStat.GetValue_double(EComplexStatField::IncMin));
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
						}
						else if (ComplexStat.NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_int64)
						{
							const FString ValueFormatted = FormatStatValueInt64(ComplexStat.GetValue_int64(EComplexStatField::IncMin));
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*ValueFormatted));
						}
					}
				}
				Clipper.End();
				ImGui::EndTable();
			}
		}
	}
#else
	ImGui::Text("Stat Not Enable");
#endif
}

#undef LOCTEXT_NAMESPACE
