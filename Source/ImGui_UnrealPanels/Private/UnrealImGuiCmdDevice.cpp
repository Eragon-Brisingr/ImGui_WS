// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiCmdDevice.h"

#include "ConsoleSettings.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "imgui_internal.h"
#include "ShowFlags.h"
#include "UnrealImGuiStat.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Stats/StatsData.h"
#include "UObject/UObjectIterator.h"

void FUnrealImGuiCmdDevice::SetCmdString(const FString InCmdString)
{
	ClearCmdString();
	CmdString = InCmdString;
	if (CmdString.IsEmpty())
	{
		return;
	}
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiCmdDevice_AutoCompleteList"), STAT_UnrealImGuiCmdDevice_AutoCompleteList, STATGROUP_ImGui);
	
	// UConsole::BuildRuntimeAutoCompleteList
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();
	
	// ManualAutoCompleteList
	for (const FAutoCompleteCommand& AutoCompleteCommand : ConsoleSettings->ManualAutoCompleteList)
	{
		if (AutoCompleteCommand.Command.StartsWith(CmdString))
		{
			MatchedCmd.Add({ *AutoCompleteCommand.Command, *AutoCompleteCommand.Desc });
		}
	}

	// ConsoleObject
	IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(FConsoleObjectVisitor::CreateLambda([&](const TCHAR* Cmd, IConsoleObject* ConsoleObject)
	{
		MatchedCmd.Add({ Cmd, ConsoleObject->GetHelp() });
	}), *CmdString);

	// FUNC_Exec
	for (TObjectIterator<UFunction> It; It; ++It)
	{
		UFunction* Func = *It;

		// Determine whether or not this is a level script event that we can call (must be defined in the level script actor and not in parent, and has no return value)
		const UClass* FuncOuter = Cast<UClass>(Func->GetOuter());
		const bool bIsLevelScriptFunction = FuncOuter
			&& (FuncOuter->IsChildOf(ALevelScriptActor::StaticClass()))
			&& (FuncOuter != ALevelScriptActor::StaticClass())
			&& (Func->ReturnValueOffset == MAX_uint16)
			&& (Func->GetSuperFunction() == nullptr);

		// exec functions that either have no parent, level script events, or are in the global state (filtering some unnecessary dupes)
		if ((Func->HasAnyFunctionFlags(FUNC_Exec) && (Func->GetSuperFunction() == nullptr || FuncOuter))
			|| bIsLevelScriptFunction)
		{
			FString FuncName = Func->GetName();
			if (FDefaultValueHelper::HasWhitespaces(FuncName))
			{
				FuncName = FString::Printf(TEXT("\"%s\""), *FuncName);
			}
			if (bIsLevelScriptFunction)
			{
				FuncName = FString(TEXT("ce ")) + FuncName;
			}
			if (FuncName.StartsWith(CmdString) == false)
			{
				continue;
			}

			FString Desc;
			// build a help string
			// append each property (and it's type) to the help string
			for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
			{
				FProperty* Prop = *PropIt;
				Desc += FString::Printf(TEXT("%s[%s] "), *Prop->GetName(), *Prop->GetCPPType());
			}
			MatchedCmd.Add({ *FuncName, *Desc });
		}
	}

	// enumerate maps
	if (CmdString.StartsWith(TEXT("open")) || CmdString.StartsWith(TEXT("travel")) || CmdString.StartsWith(TEXT("servertravel")))
	{
		auto FindPackagesInDirectory = [](TArray<FString>& OutPackages, const FString& InPath)
		{
			FString PackagePath;
			if (FPackageName::TryConvertFilenameToLongPackageName(InPath, PackagePath))
			{
				if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::LoadModulePtr<FAssetRegistryModule>(TEXT("AssetRegistry")))
				{
					TArray<FAssetData> Assets;
					AssetRegistryModule->Get().GetAssetsByPath(FName(*PackagePath), Assets, true);

					for (const FAssetData& Asset : Assets)
					{
						if (!!(Asset.PackageFlags & PKG_ContainsMap) && Asset.IsUAsset())
						{
							OutPackages.AddUnique(Asset.AssetName.ToString());
						}
					}
				}
			}
			TArray<FString> Filenames;
			FPackageName::FindPackagesInDirectory(Filenames, InPath);

			for (const FString& Filename : Filenames)
			{
				const int32 NameStartIdx = Filename.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				const int32 ExtIdx = Filename.Find(*FPackageName::GetMapPackageExtension(), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

				if (NameStartIdx != INDEX_NONE && ExtIdx != INDEX_NONE)
				{
					OutPackages.AddUnique(Filename.Mid(NameStartIdx + 1, ExtIdx - NameStartIdx - 1));
				}
			}
		};

		TArray<FString> Packages;
		for (const FString& MapPath : ConsoleSettings->AutoCompleteMapPaths)
		{
			FindPackagesInDirectory(Packages, FString::Printf(TEXT("%s%s"), *FPaths::ProjectDir(), *MapPath));
		}

		FindPackagesInDirectory(Packages, FPaths::GameUserDeveloperDir());

		for (const FString& MapName : Packages)
		{
			const FString OpenMapCmd = FString::Printf(TEXT("open %s"), *MapName);
			if (OpenMapCmd.StartsWith(CmdString))
			{
				MatchedCmd.Add({ *OpenMapCmd, TEXT("") });
			}
			const FString TravelMapCmd = FString::Printf(TEXT("travel %s"), *MapName);
			if (TravelMapCmd.StartsWith(CmdString))
			{
				MatchedCmd.Add({ *TravelMapCmd, TEXT("") });
			}
			const FString ServerTravelMapCmd = FString::Printf(TEXT("servertravel %s"), *MapName);
			if (ServerTravelMapCmd.StartsWith(CmdString))
			{
				MatchedCmd.Add({ *ServerTravelMapCmd, TEXT("") });
			}
		}
	}

	// misc commands
	{
		const FString OpenUrlCmd = TEXT("open 127.0.0.1");
		if (OpenUrlCmd.Contains(CmdString))
		{
			MatchedCmd.Add({ *OpenUrlCmd, TEXT("(opens connection to localhost)") });
		}
	}

#if STATS
	// stat commands
	if (CmdString.StartsWith(TEXT("Stat")))
	{
		const TSet<FName>& StatGroupNames = FStatGroupGameThreadNotifier::Get().StatGroupNames;
		for (const FName& StatGroupName : StatGroupNames)
		{
			FString Command = FString(TEXT("Stat "));
			Command += StatGroupName.ToString().RightChop(sizeof("STATGROUP_") - 1);

			if (Command.StartsWith(InCmdString))
			{
				MatchedCmd.Add({ *Command, *StatGroupName.ToString() });
			}
		}
	}
#endif

	// Add all showflag commands.
	if (CmdString.StartsWith(TEXT("show")))
	{
		struct FIterSink
		{
			bool HandleShowFlag(uint32 InIndex, const FString& InName)
			{
				// Get localized name.
				FText LocName;
				FEngineShowFlags::FindShowFlagDisplayName(InName, LocName);

				const FString Cmd = TEXT("show ") + InName;
				if (Cmd.StartsWith(CmdDevice.CmdString))
				{
					CmdDevice.MatchedCmd.Add({ *Cmd, *FString::Printf(TEXT("(toggles the %s showflag)"), *LocName.ToString()) });
				}

				return true;
			}
			
			bool OnEngineShowFlag(uint32 InIndex, const FString& InName)
			{
				return HandleShowFlag(InIndex, InName);
			}

			bool OnCustomShowFlag(uint32 InIndex, const FString& InName)
			{
				return HandleShowFlag(InIndex, InName);
			}

			FUnrealImGuiCmdDevice& CmdDevice;
		};

		FIterSink Sink{ *this };
		FEngineShowFlags::IterateAllFlags(Sink);
	}
}

void FUnrealImGuiCmdDevice::ClearCmdString()
{
	CmdString.Empty();
	MatchedCmd.Reset();
	SelectedCmdBarIndex = INDEX_NONE;
}

void FUnrealImGuiCmdDevice::Draw(UObject* Owner)
{
	ImGui::Text("cmd:");
	ImGui::SameLine();
	bool ReclaimFocus = false;
	TArray<ANSICHAR, TInlineAllocator<256>> CmdArray;
	{
		const auto StringPoint = FTCHARToUTF8(*CmdString);
		CmdArray.SetNumZeroed(FMath::Max(256, StringPoint.Length() + 128));
		FMemory::Memcpy(CmdArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
	}
	struct FTextEditCallbackStubData
	{
		FUnrealImGuiCmdDevice& This;
		bool bInvokeNavCmdBar = false;
		bool bInvokeShowHistory = false;
	};
	FTextEditCallbackStubData CallbackStubData{ *this, false };
	static auto ExecuteCmd = [](const UObject* Owner, const FString& Cmd)
	{
		UWorld* World = Owner->GetWorld();
		if (World)
		{
			const UGameInstance* GameInstance = World->GetGameInstance();
			if (GameInstance && GameInstance->GetFirstLocalPlayerController())
			{
				APlayerController* PC = GameInstance->GetFirstLocalPlayerController();
				PC->ConsoleCommand(Cmd);
				IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *Cmd);
				return;
			}
		}
		GEngine->Exec(World, *Cmd);
		IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *Cmd);
	};
	static auto TextEditCallbackStub = [](ImGuiInputTextCallbackData* Data)
	{
		const ImGuiInputTextFlags_ EventFlag = static_cast<ImGuiInputTextFlags_>(Data->EventFlag);
		FTextEditCallbackStubData& CallbackStubData = *static_cast<FTextEditCallbackStubData*>(Data->UserData);
		FUnrealImGuiCmdDevice& This = CallbackStubData.This;
		switch (EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			{
				if (This.MatchedCmd.IsValidIndex(This.SelectedCmdBarIndex))
				{
					const auto Cmd = This.MatchedCmd[This.SelectedCmdBarIndex];
					Data->DeleteChars(0, Data->BufTextLen);
					Data->InsertChars(0, *Cmd.CmdString);
					This.SetCmdString(*Cmd.CmdString);
				}
			}
			break;
		case ImGuiInputTextFlags_CallbackHistory:
			{
				if (Data->EventKey == ImGuiKey_UpArrow)
				{
					if (This.CmdString.IsEmpty())
					{
						CallbackStubData.bInvokeShowHistory = true;
						This.MatchedCmd.Reset();
						TArray<FString> ConsoleHistory;
						IConsoleManager::Get().GetConsoleHistory(TEXT(""), ConsoleHistory);
						if (ConsoleHistory.Num() > 0)
						{
							for (const FString& HistoryCmd : ConsoleHistory)
							{
								const IConsoleObject* ConsoleObject = IConsoleManager::Get().FindConsoleObject(*HistoryCmd);
								This.MatchedCmd.Add({ *HistoryCmd, ConsoleObject ? ConsoleObject->GetHelp() : TEXT("") });
							}
							This.SelectedCmdBarIndex = ConsoleHistory.Num() - 1;
							Data->DeleteChars(0, Data->BufTextLen);
							Data->InsertChars(0, *This.MatchedCmd.Last().CmdString);
							This.CmdString = ConsoleHistory.Last();
						}
					}
					else if (This.SelectedCmdBarIndex > 0 || (This.SelectedCmdBarIndex == INDEX_NONE && This.MatchedCmd.Num() > 0))
					{
						CallbackStubData.bInvokeNavCmdBar = true;
						if (This.SelectedCmdBarIndex > 0)
						{
							This.SelectedCmdBarIndex -= 1;
						}
						else if (This.SelectedCmdBarIndex == INDEX_NONE)
						{
							This.SelectedCmdBarIndex = 0;
						}
						const auto Cmd = This.MatchedCmd[This.SelectedCmdBarIndex];
						Data->DeleteChars(0, Data->BufTextLen);
						Data->InsertChars(0, *Cmd.CmdString);
						This.CmdString = *Cmd.CmdString;
					}
				}
				else if (Data->EventKey == ImGuiKey_DownArrow)
				{
					CallbackStubData.bInvokeNavCmdBar = true;
					if (This.SelectedCmdBarIndex < This.MatchedCmd.Num() - 1)
					{
						This.SelectedCmdBarIndex += 1;
						const auto Cmd = This.MatchedCmd[This.SelectedCmdBarIndex];
						Data->DeleteChars(0, Data->BufTextLen);
						Data->InsertChars(0, *Cmd.CmdString);
						This.CmdString = *Cmd.CmdString;
					}
				}
			}
			break;
		case ImGuiInputTextFlags_CallbackEdit:
			{
				This.SetCmdString(UTF8_TO_TCHAR(Data->Buf));
			}
			break;
		default:
			break;
		}
		return 0;
	};
	if (ImGui::InputText("##cmd", CmdArray.GetData(), CmdArray.Num(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit, TextEditCallbackStub, &CallbackStubData))
	{
		const FString Cmd = UTF8_TO_TCHAR(CmdArray.GetData());
		if (Cmd.IsEmpty() == false)
		{
			ExecuteCmd(Owner, Cmd);
		}
		ClearCmdString();
		ReclaimFocus = true;
	}
	if (MatchedCmd.Num() > 0)
	{
		static bool PreInputTextIsActive = false;
		const bool InputTextIsActive = ImGui::IsItemActive();

		if (PreInputTextIsActive || ImGui::IsItemActive())
		{
			constexpr float CmdBarHeight = 100.f;
			const ImVec2 CmdBarPos = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y - CmdBarHeight };
			ImGui::SetNextWindowPos(CmdBarPos, ImGuiCond_Always);
			ImGui::SetNextWindowSize({0.f, CmdBarHeight});
			ImGui::OpenPopup("##CmdPopup");
			if (ImGui::BeginPopup("##CmdPopup", ImGuiWindowFlags_ChildWindow))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, false);

				if (ImGui::BeginTable("##CmdPopupTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
				{
					ImGui::TableSetupColumn("Cmd", ImGuiTableColumnFlags_WidthFixed, 200.f);
					ImGui::TableSetupColumn("Help", ImGuiTableColumnFlags_WidthFixed, 400.f);
					for (int32 Idx = 0; Idx < MatchedCmd.Num(); ++Idx)
					{
						ImGui::TableNextColumn();
						const auto& Cmd = MatchedCmd[Idx];
						if (ImGui::Selectable(*Cmd.CmdString, SelectedCmdBarIndex == Idx))
						{
							SetCmdString(*Cmd.CmdString);
						}
						if (CallbackStubData.bInvokeNavCmdBar && SelectedCmdBarIndex == Idx)
						{
							ImGui::ScrollToItem(ImGuiScrollFlags_AlwaysCenterY);
						}
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(*Cmd.CmdHelp);
						if (ImGui::BeginItemTooltip())
						{
							ImGui::TextUnformatted(*Cmd.CmdHelp);
							ImGui::EndTooltip();
						}
					}
					ImGui::EndTable();
					if (CallbackStubData.bInvokeShowHistory)
					{
						ImGui::SetScrollHereY(1.f);
					}
				}
				ImGui::PopItemFlag();
				ImGui::EndPopup();
			}
		}
		PreInputTextIsActive = InputTextIsActive;
	}
	if (ReclaimFocus)
	{
		ImGui::SetItemDefaultFocus();
		ImGui::SetKeyboardFocusHere(-1);
	}

	ImGui::SameLine();
	{
		ImGui::FDisabled Disabled{ CmdString.IsEmpty() };
		if (ImGui::Button("Execute"))
		{
			ExecuteCmd(Owner, CmdString);
			ClearCmdString();
		}
	}
}
