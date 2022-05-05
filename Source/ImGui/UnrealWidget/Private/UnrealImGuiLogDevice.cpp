// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLogDevice.h"

#include "imgui.h"
#include "UnrealImGuiStat.h"

DECLARE_MEMORY_STAT(TEXT("UnrealImGuiOutputDevice Logs"), Stat_UnrealImGuiOutputDevice_Logs, STATGROUP_ImGui);

namespace UnrealImGui
{
	FUTF8String VerbosityString[ELogVerbosity::NumVerbosity] =
	{
		FUTF8String{ ::ToString(ELogVerbosity::Fatal) },
		FUTF8String{ ::ToString(ELogVerbosity::Error) },
		FUTF8String{ ::ToString(ELogVerbosity::Warning) },
		FUTF8String{ ::ToString(ELogVerbosity::Display) },
		FUTF8String{ ::ToString(ELogVerbosity::Log) },
		FUTF8String{ ::ToString(ELogVerbosity::Verbose) },
		FUTF8String{ ::ToString(ELogVerbosity::VeryVerbose) },
	};

	ImU32 VerbosityColor[ELogVerbosity::NumVerbosity] =
	{
		IM_COL32(255, 0, 255, 255), // Fatal
		IM_COL32(255, 0, 0, 255), // Error
		IM_COL32(255, 255, 0, 255), // Warning
		IM_COL32(255, 255, 255, 255), // Display
		IM_COL32(255, 255, 255, 255), // Log
		IM_COL32(123, 123, 123, 255), // Verbose
		IM_COL32(123, 123, 123, 255), // VeryVerbose
	};

	TSharedPtr<FUnrealImGuiOutputDevice> GUnrealImGuiOutputDevice;

	const FUTF8String& ToString(ELogVerbosity::Type Verbosity)
	{
		return VerbosityString[Verbosity - 1];
	}

	uint32 ToColor(ELogVerbosity::Type Verbosity)
	{
		return VerbosityColor[Verbosity - 1];
	}

	FUnrealImGuiOutputDevice::FUnrealImGuiOutputDevice()
	{
		check(GLog);
		GLog->AddOutputDevice(this);
		SET_MEMORY_STAT(Stat_UnrealImGuiOutputDevice_Logs, 0);
	}

	FUnrealImGuiOutputDevice::~FUnrealImGuiOutputDevice()
	{
		if (GLog != nullptr)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

	void FUnrealImGuiOutputDevice::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
	{
		const ELogVerbosity::Type TestVerbosity = static_cast<ELogVerbosity::Type>(Verbosity & ELogVerbosity::VerbosityMask);
		if (TestVerbosity)
		{
			const int32 LogLine = Logs.Add(FLog{ Message, TestVerbosity, Category });
			INC_MEMORY_STAT_BY(Stat_UnrealImGuiOutputDevice_Logs, sizeof(FLog) + Logs[LogLine].LogString.GetAllocatedSize());
			CategoryNames.Add(Category);
			for (FUnrealImGuiLogDevice* LogDevice : LogDevices)
			{
				LogDevice->PostLogAdded(Logs[LogLine], LogLine);
			}
		}
	}

	void FUnrealImGuiOutputDevice::Register(FUnrealImGuiLogDevice* LogDevice)
	{
		check(LogDevices.Contains(LogDevice) == false);

		LogDevices.Add(LogDevice);
	}

	void FUnrealImGuiOutputDevice::Unregister(FUnrealImGuiLogDevice* LogDevice)
	{
		check(LogDevices.Contains(LogDevice));

		LogDevices.Remove(LogDevice);
	}
}

FUnrealImGuiLogDevice::FUnrealImGuiLogDevice()
	: VerbosityVisibility{ true, true, true, true, true, true, false }
	, bIsFirstDraw(2)
{}

void FUnrealImGuiLogDevice::Register()
{
	using namespace UnrealImGui;
	GUnrealImGuiOutputDevice->Register(this);
	RefreshDisplayLines();
}

void FUnrealImGuiLogDevice::Unregister()
{
	using namespace UnrealImGui;
	GUnrealImGuiOutputDevice->Unregister(this);
}

void FUnrealImGuiLogDevice::Draw(UObject* Owner)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiLogDevice_Draw"), STAT_UnrealImGuiLogDevice_Draw, STATGROUP_ImGui);
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Verbosity"))
		{
			for (uint8 VerbosityIdx = ELogVerbosity::All; VerbosityIdx > ELogVerbosity::NoLogging; --VerbosityIdx)
			{
				const ELogVerbosity::Type Verbosity = static_cast<ELogVerbosity::Type>(VerbosityIdx);
				ImGui::PushStyleColor(ImGuiCol_Text, UnrealImGui::ToColor(Verbosity));
				if (ImGui::Checkbox(*UnrealImGui::ToString(Verbosity), &VerbosityVisibility[Verbosity - 1]))
				{
					RefreshDisplayLines();
					Owner->SaveConfig();
				}
				ImGui::PopStyleColor();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Category"))
		{
			if (ImGui::MenuItem("Show All"))
			{
				HideCategoryNames.Empty();
				RefreshDisplayLines();
				Owner->SaveConfig();
			}
			if (ImGui::MenuItem("Hide All"))
			{
				HideCategoryNames = UnrealImGui::GUnrealImGuiOutputDevice->CategoryNames;
				RefreshDisplayLines();
				Owner->SaveConfig();
			}
			ImGui::Separator();
			for (const FName& CategoryName : UnrealImGui::GUnrealImGuiOutputDevice->CategoryNames)
			{
				bool IsDisplay = HideCategoryNames.Contains(CategoryName) == false;
				if (ImGui::Checkbox(TCHAR_TO_UTF8(*CategoryName.ToString()), &IsDisplay))
				{
					if (IsDisplay == false)
					{
						HideCategoryNames.Add(CategoryName);
					}
					else
					{
						HideCategoryNames.Remove(CategoryName);
					}
					RefreshDisplayLines();
					Owner->SaveConfig();
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::Text("Filter:");
	ImGui::SameLine();
	TArray<ANSICHAR, TInlineAllocator<256>> FilterLogArray;
	{
		const auto StringPoint = FTCHARToUTF8(*FilterString);
		FilterLogArray.SetNumZeroed(FMath::Max(256, StringPoint.Length() + 128));
		FMemory::Memcpy(FilterLogArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
	}
	if (ImGui::InputText("##Filter", FilterLogArray.GetData(), FilterLogArray.Num()))
	{
		FilterString = UTF8_TO_TCHAR(FilterLogArray.GetData());
		RefreshDisplayLines();
		Owner->SaveConfig();
	}
	const float FooterHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); 
	if (ImGui::BeginChild("LogScrollingRegion", ImVec2(0, -FooterHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGuiListClipper Clipper;
		Clipper.Begin(DisplayLines.Num());

		while (Clipper.Step())
		{
			for (int32 Idx = Clipper.DisplayStart; Idx < Clipper.DisplayEnd; ++Idx)
			{
				const auto& Log = UnrealImGui::GUnrealImGuiOutputDevice->Logs[DisplayLines[Idx]];

				ImGui::PushStyleColor(ImGuiCol_Text, UnrealImGui::ToColor(Log.Verbosity));
				ImGui::Text("%s: %s: %s", TCHAR_TO_UTF8(*Log.Category.ToString()), *UnrealImGui::ToString(Log.Verbosity), *Log.LogString);
				ImGui::PopStyleColor();
			}
		}
		Clipper.End();
		if (bIsFirstDraw)
		{
			bIsFirstDraw -= 1;
			if (bIsFirstDraw == false)
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		else
		{
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();
		
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Clear Log"))
			{
				ClearCurrentLines();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Copy All"))
			{
				FString ToClipboardText;
				const auto& Logs = UnrealImGui::GUnrealImGuiOutputDevice->Logs;
				for (int32 Idx = StartDisplayLine; Idx < Logs.Num(); ++Idx)
				{
					const auto& Log = Logs[Idx];
					ToClipboardText += FString::Printf(TEXT("%s: %s: %s"), *Log.Category.ToString(), ToString(Log.Verbosity), UTF8_TO_TCHAR(*Log.LogString));;
					if (Idx != Logs.Num() - 1)
					{
						ToClipboardText += TEXT("\n");
					}
				}
				ImGui::SetClipboardText(TCHAR_TO_UTF8(*ToClipboardText));
			}
			ImGui::EndPopup();
		}
	}
}

void FUnrealImGuiLogDevice::RefreshDisplayLines()
{
	using namespace UnrealImGui;
	DisplayLines.Reset();
	for (int32 Idx = StartDisplayLine; Idx < GUnrealImGuiOutputDevice->Logs.Num(); ++Idx)
	{
		const auto& Log = GUnrealImGuiOutputDevice->Logs[Idx];
		if (CanLogDisplay(Log) == false)
		{
			continue;
		}

		DisplayLines.Add(Idx);
	}
}

void FUnrealImGuiLogDevice::ClearCurrentLines()
{
	using namespace UnrealImGui;
	StartDisplayLine = GUnrealImGuiOutputDevice->Logs.Num();
	DisplayLines.Empty();
}

bool FUnrealImGuiLogDevice::CanLogDisplay(const UnrealImGui::FUnrealImGuiOutputDevice::FLog& Log)
{
	if (VerbosityVisibility[Log.Verbosity - 1] == false)
	{
		return false;
	}

	if (HideCategoryNames.Contains(Log.Category))
	{
		return false;
	}

	if (FilterString.IsEmpty() == false)
	{
		const FString TestString = FString::Printf(TEXT("%s: %s: %s"), *Log.Category.ToString(), ToString(Log.Verbosity), UTF8_TO_TCHAR(*Log.LogString));
		if (TestString.Contains(FilterString) == false)
		{
			return false;
		}
	}
	return true;
}

void FUnrealImGuiLogDevice::PostLogAdded(const UnrealImGui::FUnrealImGuiOutputDevice::FLog& Log, int32 LogLine)
{
	if (CanLogDisplay(Log))
	{
		DisplayLines.Add(LogLine);
	}
}
