// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGui_WS_Manager.h"

#include <sstream>
#include <thread>

#include "imgui-ws.h"
#include "imgui.h"
#include "ImGuiDelegates.h"
#include "ImGuiEx.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFontAtlas.h"
#include "ImGuiSettings.h"
#include "ImGuiUnrealContextManager.h"
#include "imgui_internal.h"
#include "imgui_notify.h"
#include "implot.h"
#include "UnrealImGuiStat.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiStyles.h"
#include "UnrealImGuiTexture.h"
#include "UnrealImGui_Log.h"
#include "WebKeyCodeToImGui.h"
#include "Containers/TripleBuffer.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/Thread.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"
#include "Record/imgui-ws-record.h"
#include "Record/ImGuiWS_Replay.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

FAutoConsoleCommand LaunchImGuiWeb
{
	TEXT("ImGui.WS.LaunchWeb"),
	TEXT("Open ImGui-WS Web"),
	FConsoleCommandDelegate::CreateLambda([]
	{
		const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
		Manager->OpenWebPage();
	})
};

UnrealImGui::FUTF8String GRecordSaveDirPathString;
TAutoConsoleVariable<int32> CVar_ImGui_WS_Port
{
	TEXT("ImGui.WS.Port"),
	INDEX_NONE,
	TEXT("ImGui-WS Web Port, Only Valid When Pre Game Start. Set In\n")
	TEXT("1. Engine.ini\n [ConsoleVariables] \n ImGui.WS.Port=8890\n")
	TEXT("2. UE4Editor.exe GAMENAME -ExecCmds=\"ImGui.WS.Port 8890\""),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*)
	{
		const int32 ImGui_WS_Port = CVar_ImGui_WS_Port.GetValueOnAnyThread();
		UImGuiSettings* Settings = GetMutableDefault<UImGuiSettings>();
		if (GIsEditor)
		{
			Settings->EditorPort = ImGui_WS_Port;
		}
		else if (GIsServer)
		{
			Settings->ServerPort = ImGui_WS_Port;
		}
		else
		{
			Settings->GamePort = ImGui_WS_Port;
		}
	})
};

TAutoConsoleVariable<int32> CVar_ImGui_WS_Enable
{
	TEXT("ImGui.WS.Enable"),
	INDEX_NONE,
	TEXT("Set ImGui-WS Enable 0: Disable 1: Enable"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*)
	{
		UImGuiSettings* Settings = GetMutableDefault<UImGuiSettings>();
		const bool bIsEnable = !!CVar_ImGui_WS_Enable.GetValueOnAnyThread();
		if (GIsEditor)
		{
			Settings->bEditorEnableImGui_WS = bIsEnable;
		}
		else if (GIsServer)
		{
			Settings->bServerEnableImGui_WS = bIsEnable;
		}
		else
		{
			Settings->bGameEnableImGui_WS = bIsEnable;
		}
		if (UImGui_WS_Manager* Manager = GEngine->GetEngineSubsystem<UImGui_WS_Manager>())
		{
			if (bIsEnable)
			{
				Manager->Enable();
			}
			else
			{
				Manager->Disable();
			}
		}
	})
};

TAutoConsoleVariable<FString> CVar_RecordSaveDirPathString
{
	TEXT("ImGui.WS.RecordDirPath"),
	TEXT("./"),
	TEXT("Set ImGui-WS Record Saved Path"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*)
	{
		const FString RecordSaveDirPathString = CVar_RecordSaveDirPathString.GetValueOnAnyThread();
		if (FPaths::DirectoryExists(RecordSaveDirPathString))
		{
			GRecordSaveDirPathString = UnrealImGui::FUTF8String{ *RecordSaveDirPathString };
		}
	})
};
FAutoConsoleCommand StartImGuiRecord
{
	TEXT("ImGui.WS.StartRecord"),
	TEXT("Start ImGui-WS Record"),
	FConsoleCommandDelegate::CreateLambda([]
	{
		UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
		if (Manager->IsEnable() && Manager->IsRecording() == false)
		{
			Manager->StartRecord();
		}
	})
};
FAutoConsoleCommand EndImGuiRecord
{
	TEXT("ImGui.WS.StopRecord"),
	TEXT("Stop ImGui-WS Record"),
	FConsoleCommandDelegate::CreateLambda([]
	{
		UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
		if (Manager->IsEnable() && Manager->IsRecording())
		{
			Manager->StopRecord();
		}
	})
};

#if PLATFORM_WINDOWS
#include <corecrt_io.h>
#endif
#include <fcntl.h>

class UImGui_WS_Manager::FImpl final : FTickableGameObject
{
	UImGui_WS_Manager& Manager;
	UImGuiUnrealContextManager& ContextManager;
public:
	ImGuiWS ImGuiWS;
	FThread WS_Thread;
    std::atomic_bool bRequestedExit{ false };

	ImGuiContext* Context;
	ImPlotContext* PlotContext;
	decltype(ImGuiPlatformIO::Platform_SetClipboardTextFn) SetClipboardTextFn_DefaultImpl = nullptr;

	FCriticalSection RecordCriticalSection;
	TSharedPtr<ImGuiWS_Record::Session, ESPMode::ThreadSafe> RecordSession;
	TUniquePtr<ImGuiWS_Record::FImGuiWS_Replay> RecordReplay;

	struct EServerEventType
	{
		enum Type : int32
		{
			SetClipboardText
		};
	};

	explicit FImpl(UImGui_WS_Manager& Manager)
		: Manager(Manager)
		, ContextManager(*UImGuiUnrealContextManager::GetChecked())
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT(UE_PLUGIN_NAME));
		const FString PluginResourcesPath = Plugin->GetBaseDir() / TEXT("Resources");

		IMGUI_CHECKVERSION();

		ImGuiContext* PrevContext = ImGui::GetCurrentContext();
		Context = ImGui::CreateContext(&UnrealImGui::GetDefaultFontAtlas());
		ImGui::SetCurrentContext(Context);
		ImGuiIO& IO = Context->IO;
		IO.FontGlobalScale = UnrealImGui::GetGlobalDPIScale();
		ON_SCOPE_EXIT
		{
			ImGui::SetCurrentContext(PrevContext);
		};

		IO.MouseDrawCursor = false;
		SetClipboardTextFn_DefaultImpl = ImGui::GetPlatformIO().Platform_SetClipboardTextFn;
		ImGui::GetPlatformIO().Platform_SetClipboardTextFn = [](ImGuiContext* ctx, const char* text)
		{
			const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
			const int32 Len = FCStringAnsi::Strlen(text);
			TArray<uint8> Payload;
			Payload.SetNumUninitialized(Len + 1);
			FMemory::Memcpy(Payload.GetData(), text, Len + 1);
			Manager->Impl->ImGuiWS.AddServerEvent(Manager->Impl->State.CurControlId, EServerEventType::SetClipboardText, MoveTemp(Payload));
		};

		// Enable Docking
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		UnrealImGui::DefaultStyle();
		ImGui::GetStyle().AntiAliasedFill = false;
		ImGui::GetStyle().AntiAliasedLines = false;
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT(UE_PLUGIN_NAME);
		// Make sure that directory is created.
		PlatformFile.CreateDirectory(*IniDirectory);
		static const UnrealImGui::FUTF8String IniFilePath = IniDirectory / TEXT("ImGui_WS.ini");
		IO.IniFilename = IniFilePath.GetData();
	    IO.DisplaySize = ImVec2(0, 0);

		PlotContext = ImPlot::CreateContext();

		// setup imgui-ws
		const FString HtmlRelativePath = PluginResourcesPath / TEXT("HTML");
		const FString HtmlPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::ConvertRelativePathToFull(HtmlRelativePath));
		if (FPaths::DirectoryExists(HtmlPath) == false)
		{
			// TODO: Android platform pack file to obb, we need copy them to user folder
			// because fopen can't read this, find batter way to resolve this problem
			TArray<FString> HtmlFiles{ TEXT("draw-mouse-pos.js"), TEXT("imgui-ws.js"), TEXT("incppect.js"), TEXT("index.html") };
			for (const FString& File : HtmlFiles)
			{
				const FString FileRelativePath = HtmlRelativePath / File;
				const FString FilePath = HtmlPath / File;
				UE_LOG(LogImGui, Log, TEXT("Write web file from %s to %s"), *FileRelativePath, *FilePath);

				TArray<uint8> Bin;
				ensure(FFileHelper::LoadFileToArray(Bin, *FileRelativePath));
				FFileHelper::SaveArrayToFile(Bin, *FilePath);
			}
		}
		ImGuiWS.Init(Manager.GetPort(), HtmlPath);
		WS_Thread = FThread{ TEXT("ImGui_WS"), [this, Interval = GetDefault<UImGuiSettings>()->ServerTickInterval]
		{
			while (bRequestedExit == false)
			{
				const double StartSeconds = FPlatformTime::Seconds();
				WS_ThreadUpdate();
				const double SleepSeconds = Interval - (FPlatformTime::Seconds() - StartSeconds);
				if (SleepSeconds > 0.f)
				{
					FPlatformProcess::SleepNoStats(SleepSeconds);
				}
			}
		}, 0, TPri_Lowest };

		// prepare font texture
		{
			unsigned char* Pixels;
			int32 Width, Height;
			IO.Fonts->GetTexDataAsAlpha8(&Pixels, &Width, &Height);
			ImGuiWS.SetTexture(0, ImGuiWS::FTexture::Type::Alpha8, Width, Height, Pixels);
		}

		using namespace UnrealImGui;
		Private::UpdateTextureData_WS = [this](FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, const uint8* Data)
		{
			static_assert((int32_t)ImGuiWS::FTexture::Type::Alpha8 == (uint8)ETextureFormat::Alpha8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::Gray8 == (uint8)ETextureFormat::Gray8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::RGB24 == (uint8)ETextureFormat::RGB8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::RGBA32 == (uint8)ETextureFormat::RGBA8);
			ImGuiWS.SetTexture(Handle, ImGuiWS::FTexture::Type{ static_cast<uint8>(TextureFormat) }, Width, Height, Data);
		};

		FImGuiDelegates::OnImGui_WS_Enable.Broadcast();
	}
	~FImpl() override
	{
		FImGuiDelegates::OnImGui_WS_Disable.Broadcast();
		FImGuiDelegates::OnImGuiContextDestroyed.Broadcast(Context);
		ImGui::DestroyContext(Context);
		ImPlot::DestroyContext(PlotContext);
		UnrealImGui::Private::UpdateTextureData_WS.Reset();
		bRequestedExit = true;
		if (WS_Thread.IsJoinable())
		{
			WS_Thread.Join();
		}
	}

	struct FVSync
	{
		FVSync(double RateFps = 60.0) : tStep_us(1000000.0/RateFps) {}

		uint64_t t_us() const
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); // duh ..
		}

		float Delta_S()
		{
			uint64_t tNow_us = t_us();
			uint64_t res = tNow_us - tLast_us;
			tLast_us = tNow_us;
			return float(res)/1e6f;
		}

		uint64_t tStep_us;
		uint64_t tLast_us = t_us();
		uint64_t tNext_us = tLast_us + tStep_us;
	};
	struct FState
	{
		FState() = default;

		bool bShowImGuiDemo = false;
		bool bShowPlotDemo = false;

		// client control management
		struct ClientData
		{
			double ControlStartTime = 0.f;

			std::string IpString = "---";
			uint32 Ip;
		};

		int32 CurControlId = -1;
		bool bIsIdControlChanged = false;
		TMap<int32, ClientData> Clients;

		// client input
		TArray<ImGuiWS::FEvent, TInlineAllocator<16>> PendingEvents;
		// recover key down state
		TArray<ImGuiWS::FEvent, TInlineAllocator<16>> KeyDownEvents;

		void Handle(const ImGuiWS::FEvent& Event)
		{
		    switch (Event.Type)
			{
	        case ImGuiWS::FEvent::Connected:
				{
	                ClientData& Client = Clients.Add(Event.ClientId);

	        		std::stringstream ss;
			        { int32 a = uint8(Event.Ip); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.Ip >> 8); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.Ip >> 16); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.Ip >> 24); if (a < 0) a += 256; ss << a; }
	        		Client.Ip = Event.Ip;
	        		Client.IpString = ss.str();
	            }
	            break;
	        case ImGuiWS::FEvent::Disconnected:
				{
	                Clients.Remove(Event.ClientId);
	            }
	            break;
		    case ImGuiWS::FEvent::TakeControl:
			    {
		    		if (auto* Client = Clients.Find(Event.ClientId))
		    		{
		    			CurControlId = Event.ClientId;
		    			bIsIdControlChanged = true;
		    			Client->ControlStartTime = ImGui::GetTime();
		    		}
			    }
		    	break;
	        default:
	        	{
	        		if (Event.ClientId == CurControlId)
	        		{
	        			PendingEvents.Add(Event);
	        		}
	        	}
		    }
		}

		void Update(FImpl& Owner)
		{
			if (Clients.Contains(CurControlId) == false && Clients.Num() > 0)
			{
				CurControlId = Clients.CreateIterator().Key();
				bIsIdControlChanged = true;
			}
		    else if (Clients.Num() == 0 && CurControlId != INDEX_NONE)
			{
		        CurControlId = INDEX_NONE;
		    	bIsIdControlChanged = true;
		    }

			ImGuiIO& IO = ImGui::GetIO();
			auto AddKeyEvent = [&IO](ImGuiKey Key, bool bDown)
			{
		    	IO.AddKeyEvent(Key, bDown);
				switch (Key)
				{
				case ImGuiKey_LeftCtrl:
				case ImGuiKey_RightCtrl:
		    		IO.AddKeyEvent(ImGuiMod_Ctrl, bDown);
					IO.KeyCtrl = bDown;
					break;
				case ImGuiKey_LeftShift:
				case ImGuiKey_RightShift:
		    		IO.AddKeyEvent(ImGuiMod_Shift, bDown);
					IO.KeyShift = bDown;
					break;
				case ImGuiKey_LeftAlt:
				case ImGuiKey_RightAlt:
		    		IO.AddKeyEvent(ImGuiMod_Alt, bDown);
					IO.KeyAlt = bDown;
					break;
				case ImGuiKey_LeftSuper:
				case ImGuiKey_RightSuper:
		    		IO.AddKeyEvent(ImGuiMod_Super, bDown);
					IO.KeySuper = bDown;
					break;
				default:
					break;
				}
			};
			static auto ConvertWebMouseButtonToImGui = [](int32 WebMouseButton)
			{
				if (WebMouseButton == 1)
				{
					return 2;
				}
				else if (WebMouseButton == 2)
				{
					return 1;
				}
				return WebMouseButton;
			};
			if (bIsIdControlChanged)
			{
				IO.ClearInputKeys();
				KeyDownEvents.Empty();
				bIsIdControlChanged = false;
			}
		    if (CurControlId > 0)
			{
		    	for (const ImGuiWS::FEvent& Event : PendingEvents)
		    	{
		    		switch (Event.Type)
		    		{
		    		case ImGuiWS::FEvent::MouseMove:
		    			{
				            IO.AddMousePosEvent(Event.MouseX, Event.MouseY);
		    			}
		    			break;
		    		case ImGuiWS::FEvent::MouseDown:
		    			{
		    				IO.AddMousePosEvent(Event.MouseX, Event.MouseY);
		    				IO.AddMouseButtonEvent(ConvertWebMouseButtonToImGui(Event.MouseBtn), true);
		    				KeyDownEvents.Add(Event);
		    			}
		    			break;
		    		case ImGuiWS::FEvent::MouseUp:
		    			{
		    				IO.AddMousePosEvent(Event.MouseX, Event.MouseY);
		    				IO.AddMouseButtonEvent(ConvertWebMouseButtonToImGui(Event.MouseBtn), false);
		    				KeyDownEvents.RemoveAll([&Event](const ImGuiWS::FEvent& E) { return E.Type == Event.Type && E.MouseBtn == Event.MouseBtn; } );
		    			}
		    			break;
		    		case ImGuiWS::FEvent::MouseWheel:
		    			{
		    				IO.AddMouseWheelEvent(Event.WheelX, Event.WheelY);
		    			}
		    			break;
		    		case ImGuiWS::FEvent::KeyPress:
		    			{
		    				IO.AddInputCharacter(Event.Key);
		    			}
		    			break;
		    		case ImGuiWS::FEvent::KeyDown:
		    			{
		    				const ImGuiKey Key = ToImGuiKey(EWebKeyCode(Event.Key));
		    				AddKeyEvent(Key, true);
		    				KeyDownEvents.Add(Event);
		    			}
		            	break;
		            case ImGuiWS::FEvent::KeyUp:
			            {
		            		const ImGuiKey Key = ToImGuiKey(EWebKeyCode(Event.Key));
				            AddKeyEvent(Key, false);
		    				KeyDownEvents.RemoveAll([&Event](const ImGuiWS::FEvent& E) { return E.Type == Event.Type && E.MouseBtn == Event.MouseBtn; } );
			            }
		            	break;
		    		case ImGuiWS::FEvent::Resize:
		    			{
		    				IO.DisplaySize = { (float)Event.ClientWidth, (float)Event.ClientHeight };
		    			}
		    			break;
		    		case ImGuiWS::FEvent::PasteClipboard:
		    			{
		    				if (ensure(Owner.SetClipboardTextFn_DefaultImpl))
		    				{
		    					Owner.SetClipboardTextFn_DefaultImpl(Owner.Context, Event.ClipboardText.c_str());
		    				}
		    			}
		    			break;
		    		case ImGuiWS::FEvent::InputText:
		    			{
		    				IO.AddInputCharactersUTF8(Event.InputtedText.c_str());
		    			}
		    			break;
		            default:
		            	ensureMsgf(false, TEXT("Unhandle input event %d"), Event.Type);
		            }
		    	}
		    	PendingEvents.Empty();
		    }
		}
	};

	FVSync VSync;
	FState State;

	bool IsTickableWhenPaused() const override { return true; }
	bool IsTickableInEditor() const override { return true; }
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UImGui_WS_Manager_FDrawer, STATGROUP_Tickables); }
	UWorld* GetTickableGameObjectWorld() const override { return ContextManager.GetContextIndexWorld(Manager.DrawContextIndex); }

	struct FImGuiData : FNoncopyable
	{
		ImDrawData CopiedDrawData;
		const ImGuiWS::FDrawInfo DrawInfo;

		FImGuiData(const ImDrawData* DrawData, ImGuiWS_Record::FImGuiWS_Replay* Replay, const ImGuiWS::FDrawInfo&& DrawInfo)
			: CopiedDrawData{ *DrawData }
			, DrawInfo{ DrawInfo }
		{
			ImGuiWS_Record::FImGuiWS_Replay::FDrawData ReplayDrawData;
			if (Replay && Replay->GetDrawData(ReplayDrawData))
			{
				CopiedDrawData.CmdListsCount = ReplayDrawData->CmdListsCount + DrawData->CmdListsCount;
				CopiedDrawData.CmdLists.resize(CopiedDrawData.CmdListsCount);
				for (int32 Idx = 0; Idx < ReplayDrawData->CmdListsCount; ++Idx)
				{
					CopiedDrawData.CmdLists[Idx] = ReplayDrawData->CmdLists[Idx]->CloneOutput();
				}
				for (int32 Idx = 0; Idx < DrawData->CmdListsCount; ++Idx)
				{
					CopiedDrawData.CmdLists[ReplayDrawData->CmdListsCount + Idx] = DrawData->CmdLists[Idx]->CloneOutput();
				}
			}
			else
			{
				CopiedDrawData.CmdLists.resize(DrawData->CmdListsCount);
				for (int32 Idx = 0; Idx < DrawData->CmdListsCount; ++Idx)
				{
					CopiedDrawData.CmdLists[Idx] = DrawData->CmdLists[Idx]->CloneOutput();
				}
			}
		}
		~FImGuiData()
		{
			for (int32 Idx = 0; Idx < CopiedDrawData.CmdListsCount; ++Idx)
			{
				IM_DELETE(CopiedDrawData.CmdLists[Idx]);
			}
		}
	};
	TTripleBuffer<TSharedPtr<FImGuiData>> ImGuiDataTripleBuffer;

	void Tick(float DeltaTime) override
	{
		if (ImGuiWS.NumConnected() == 0 && RecordSession.IsValid() == false)
		{
	        return;
	    }

		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Tick"), STAT_ImGuiWS_Tick, STATGROUP_ImGui);

	    ImGuiContext* OldContent = ImGui::GetCurrentContext();
		ImPlotContext* OldPlotContent = ImPlot::GetCurrentContext();
	    ON_SCOPE_EXIT
		{
	        ImGui::SetCurrentContext(OldContent);
	        ImPlot::SetCurrentContext(OldPlotContent);
	    };
	    ImGui::SetCurrentContext(Context);
		ImPlot::SetCurrentContext(PlotContext);

	    ImGui::NewFrame();

	    // websocket event handling
		auto& Events = ImGuiWS.TakeEvents();
		while (Events.IsEmpty() == false)
		{
			ImGuiWS::FEvent Event;
			Events.Dequeue(Event);
	        State.Handle(Event);
		}
	    State.Update(*this);

	    ImGuiIO& IO = ImGui::GetIO();
	    IO.DeltaTime = VSync.Delta_S();

		bool CloseRecord = false;
		if (RecordReplay.IsValid())
		{
			RecordReplay->Draw(DeltaTime, CloseRecord);
		}
		else
		{
			ContextManager.DrawViewport(Manager.DrawContextIndex, DeltaTime);

			if (ImGui::BeginMainMenuBar())
			{
				ImGui::Separator();
				
				if (ImGui::BeginMenu("ImGui_WS"))
				{
					if (ImGui::BeginMenu("Settings"))
					{
						UnrealImGui::ShowStyleSelector();
						UnrealImGui::ShowGlobalDPISettings();
						
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Demo"))
					{
						ImGui::Checkbox("ImGui Demo", &State.bShowImGuiDemo);
						ImGui::Checkbox("ImPlot Demo", &State.bShowPlotDemo);
						
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Recorder"))
					{
						if (RecordSession.IsValid() == false)
						{
							if (ImGui::Button("Start Record"))
							{
								ImGui::OpenPopup("RecordSettings");
							}
							if (ImGui::BeginPopupModal("RecordSettings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
							{
								static UnrealImGui::FUTF8String SaveFilePath = GRecordSaveDirPathString;
								ImGui::Text("Save Path:");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(600.f);
								ImGui::InputText("##RecordSavePath", GRecordSaveDirPathString);

								ImGui::SameLine();
								if (ImGui::ArrowButton("SaveRecordFile", ImGuiDir_Down))
								{
									ImGui::OpenPopup("Select Save Record Directory");
								}
								static ImGui::FFileDialogState FileDialogState;
								ImGui::ShowFileDialog("Select Save Record Directory", FileDialogState, SaveFilePath, nullptr, ImGui::FileDialogType::SelectFolder);

								constexpr auto StartText = "Start";
								constexpr auto CancelText = "Cancel";
								ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (ImGui::CalcTextSize(StartText).x + ImGui::CalcTextSize(CancelText).x + ImGui::GetTextLineHeightWithSpacing() * 2.f));
								if (ImGui::Button(StartText))
								{
									if (FPaths::DirectoryExists(GRecordSaveDirPathString.ToString()))
									{
										StartRecord();
										ImGui::InsertNotification(ImGuiToastType_Info, "Start Record ImGui");
										ImGui::CloseCurrentPopup();
									}
									else
									{
										ImGui::InsertNotification(ImGuiToastType_Error, "Input Directory Not Exist");
									}
								}
								ImGui::SameLine();
								if (ImGui::Button(CancelText))
								{
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}

							if (ImGui::Button("Load Record"))
							{
								ImGui::OpenPopup("ReplaySettings");
							}
							if (ImGui::BeginPopupModal("ReplaySettings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
							{
								static UnrealImGui::FUTF8String LoadFilePath = GRecordSaveDirPathString;
								ImGui::Text("File Path:");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(600.f);
								ImGui::InputText("##RecordFilePath", LoadFilePath);

								ImGui::SameLine();
								if (ImGui::ArrowButton("OpenRecordFile", ImGuiDir_Down))
								{
									ImGui::OpenPopup("Load Replay File");
								}
								static ImGui::FFileDialogState FileDialogState;
								ImGui::ShowFileDialog("Load Replay File", FileDialogState, LoadFilePath, ".imgrcd", ImGui::FileDialogType::OpenFile);

								ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100.f);
								if (ImGui::Button("Load"))
								{
									if (FPaths::FileExists(LoadFilePath.ToString()))
									{
										RecordReplay = MakeUnique<ImGuiWS_Record::FImGuiWS_Replay>(*LoadFilePath);
										ImGui::CloseCurrentPopup();
									}
									else
									{
										ImGui::InsertNotification(ImGuiToastType_Error, "File Not Exist");
									}
								}
								ImGui::SameLine();
								if (ImGui::Button("Cancel"))
								{
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}
						}
						else
						{
							if (ImGui::Button("End Record"))
							{
								StopRecord();
							}
						}
						
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}

				// imgui-ws info
				{
					const ImVec2 WindowSize = ImGui::GetWindowSize();
					const FString ConnectionTitleString = FString::Printf(TEXT("Connections: %d"), ImGuiWS.NumConnected());
					float TotalInfoWidth = ImGui::CalcTextSize(TCHAR_TO_UTF8(*ConnectionTitleString)).x + ImGui::GetTextLineHeightWithSpacing();

					FString RecordTitleString;
					float StopRecordButtonWidth = 0.f;
					if (RecordSession.IsValid())
					{
						RecordTitleString = FString::Printf(TEXT("Recording (%.0f MB)"), RecordSession->totalSize_bytes() / 1024.f / 1024.f);
						StopRecordButtonWidth = ImGui::CalcTextSize(TCHAR_TO_UTF8(*RecordTitleString)).x + ImGui::GetTextLineHeightWithSpacing();
						TotalInfoWidth += StopRecordButtonWidth;
					}
					ImGui::Indent(WindowSize.x - TotalInfoWidth);

					if (RecordSession.IsValid())
					{
						const ImGuiStyle& Style = ImGui::GetStyle();
						if (ImGui::Button(TCHAR_TO_UTF8(*(RecordTitleString + TEXT("###StopRecordButton"))), { StopRecordButtonWidth - Style.FramePadding.x * 2.f, 0.f }))
						{
							StopRecord();
						}
						if (RecordSession.IsValid() && ImGui::BeginItemTooltip())
						{
							ImGui::Text("Stop Current Record");
							ImGui::Text("Record Info:");
							ImGui::Text("	Frame: %d", RecordSession->nFrames());
							ImGui::Text("	Size: %.2f MB", RecordSession->totalSize_bytes() / 1024.f / 1024.f);
							ImGui::EndTooltip();
						}
					}

					{
						ImGui::TextUnformatted(TCHAR_TO_UTF8(*ConnectionTitleString));
						if (ImGui::BeginItemTooltip())
						{
							ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
							ImGui::Separator();
							ImGui::Text(" Id   Ip address");
							for (auto & [ cid, client ] : State.Clients)
							{
								ImGui::Text("%3d : %s", cid, client.IpString.c_str());
								if (cid == State.CurControlId)
								{
									ImGui::SameLine();
									const float ControlledSeconds = ImGui::GetTime() - client.ControlStartTime;
									if (ControlledSeconds < 60.f)
									{
										ImGui::TextDisabled(" [has controlled %.0f seconds]", ControlledSeconds);
									}
									else
									{
										ImGui::TextDisabled(" [has controlled %.1f minutes]", ControlledSeconds / 60.f);
									}
								}
							}
							ImGui::EndTooltip();
						}
					}
				}
				ImGui::EndMainMenuBar();
			}
			// demo
			if (State.bShowImGuiDemo)
			{
				ImGui::ShowDemoWindow(&State.bShowImGuiDemo);
			}
			if (State.bShowPlotDemo)
			{
				ImPlot::ShowDemoWindow(&State.bShowPlotDemo);
			}
		}

		{
			// store ImDrawData for asynchronous dispatching to WS clients
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Generate_ImGuiData"), STAT_ImGuiWS_Generate_ImGuiData, STATGROUP_ImGui);
			
			// generate ImDrawData
			ImGui::Render();
			const ImDrawData* DrawData = ImGui::GetDrawData();

			const auto CurControlIp = State.Clients.FindRef(State.CurControlId).Ip;
			ImGuiDataTripleBuffer.WriteAndSwap(MakeShared<FImGuiData>(DrawData, RecordReplay.Get(),
				ImGuiWS::FDrawInfo{
					ImGui::GetMouseCursor(),
					State.CurControlId,
					CurControlIp,
					FVector2f{ ImGui::GetMousePos() },
					FVector2f{ IO.DisplaySize },
					IO.WantTextInput,
					IO.WantTextInput ? FVector2f{ ImGui::GetCurrentContext()->PlatformImeData.InputPos } : FVector2f::ZeroVector
				}));
		}

	    ImGui::EndFrame();

		if (CloseRecord)
		{
			RecordReplay.Reset();
		}
	}
	void WS_ThreadUpdate()
	{
		if (ImGuiDataTripleBuffer.IsDirty())
		{
			const TSharedPtr<FImGuiData> ImGuiData = ImGuiDataTripleBuffer.SwapAndRead();
			{
				DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_SetDrawData"), STAT_ImGuiWS_SetDrawData, STATGROUP_ImGui);
				ImGuiWS.SetDrawData(&ImGuiData->CopiedDrawData);
				ImGuiWS.SetDrawInfo(ImGuiData->DrawInfo);
			}

			if (const auto RecordSessionKeeper = RecordSession)
			{
				DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Record_AddFrame"), STAT_ImGuiWS_Record_AddFrame, STATGROUP_ImGui);
				FScopeLock ScopeLock{ &RecordCriticalSection };
				RecordSessionKeeper->addFrame(&ImGuiData->CopiedDrawData);
			}
		}
		ImGuiWS.Tick();
	}

	void StartRecord()
	{
		// TODO：stream save
		RecordSession = MakeShared<ImGuiWS_Record::Session>();
	}
	void StopRecord()
	{
		check(RecordSession.IsValid());
		const FString SaveDirPath = GRecordSaveDirPathString.ToString();
		FScopeLock ScopeLock{ &RecordCriticalSection };
		const FString SavePath = FString::Printf(TEXT("%s/%s.imgrcd"), *SaveDirPath, *FDateTime::Now().ToString());
		if (RecordSession->save(TCHAR_TO_UTF8(*SavePath)))
		{
			RecordSession.Reset();

			ImGui::InsertNotification(ImGuiToastType_Success, "Save Record To:\n%s", TCHAR_TO_UTF8(*SavePath));
		}
	}
};

UImGui_WS_Manager* UImGui_WS_Manager::GetChecked()
{
	check(GEngine);
	UImGui_WS_Manager* Manager = GEngine->GetEngineSubsystem<UImGui_WS_Manager>();
	check(Manager);
	return Manager;
}

UImGui_WS_Manager::~UImGui_WS_Manager()
{
	if (Impl)
	{
		delete Impl;
	}
}

bool UImGui_WS_Manager::IsSettingsEnable()
{
	const UImGuiSettings* Settings = GetDefault<UImGuiSettings>();
	if (GIsEditor)
	{
		return Settings->bEditorEnableImGui_WS;
	}
	else if (GIsServer)
	{
#if WITH_EDITOR
		return Settings->bEditorEnableImGui_WS;
#else
		return Settings->bServerEnableImGui_WS;
#endif
	}
	else if (GIsClient)
	{
		return Settings->bGameEnableImGui_WS;
	}
	return false;
}

void UImGui_WS_Manager::Enable()
{
	if (IsEnable())
	{
		return;
	}
	UE_LOG(LogImGui, Log, TEXT("Enable ImGui WS"));
	Impl = new FImpl{ *this };
}

void UImGui_WS_Manager::Disable()
{
	if (IsEnable() == false)
	{
		return;
	}
	delete Impl;
	Impl = nullptr;
	UE_LOG(LogImGui, Log, TEXT("Disable ImGui WS"));
}

int32 UImGui_WS_Manager::GetPort() const
{
	const UImGuiSettings* Settings = GetDefault<UImGuiSettings>();
	if (GIsEditor)
	{
		// Editor
		return Settings->EditorPort;
	}
	if (GIsServer)
	{
		// Server
		return Settings->ServerPort;
	}
	// Game
	return Settings->GamePort;
}

int32 UImGui_WS_Manager::GetConnectionCount() const
{
	return Impl ? Impl->ImGuiWS.NumConnected() : 0;
}

void UImGui_WS_Manager::OpenWebPage(bool bServerPort) const
{
	if (bServerPort == false && IsEnable() == false)
	{
		FPlatformProcess::LaunchURL(TEXT("http://localhost#ImGui_WS_Not_Enable!"), nullptr, nullptr);
	}
	else
	{
		const int32 Port = bServerPort ? GetDefault<UImGuiSettings>()->ServerPort : GetPort();
		FPlatformProcess::LaunchURL(*FString::Printf(TEXT("http://localhost:%d"), Port), nullptr, nullptr);
	}
}

bool UImGui_WS_Manager::IsRecording() const
{
	if (Impl && Impl->RecordSession)
	{
		return true;
	}
	return false;
}

void UImGui_WS_Manager::StartRecord()
{
	if (Impl && Impl->RecordSession.IsValid() == false)
	{
		Impl->StartRecord();
	}
}

void UImGui_WS_Manager::StopRecord()
{
	if (Impl && Impl->RecordSession.IsValid())
	{
		Impl->StopRecord();
	}
}

void UImGui_WS_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GRecordSaveDirPathString = *(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT(UE_PLUGIN_NAME));
	CVar_RecordSaveDirPathString->Set(*GRecordSaveDirPathString.ToString());
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		if (GEngine->DeferredCommands.Num() > 0)
		{
			return true;
		}
		if (IsSettingsEnable())
		{
			Enable();
		}
		else
		{
			UE_LOG(LogImGui, Log, TEXT("ImGui WS disable when engine initialized, you can use console variable ImGui.WS.Enable 1 open it."));
		}
		return false;
	}));
}

void UImGui_WS_Manager::Deinitialize()
{
	Disable();
	Super::Deinitialize();
}

#undef LOCTEXT_NAMESPACE
