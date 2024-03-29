// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGui_WS_Manager.h"

#include <sstream>
#include <thread>

#include "imgui-ws.h"
#include "imgui.h"
#include "ImGuiEditorDefaultLayout.h"
#include "ImGuiFileDialog.h"
#include "imgui_notify.h"
#include "implot.h"
#include "UnrealImGuiStat.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiTexture.h"
#include "UnrealImGuiWrapper.h"
#include "UnrealImGui_Log.h"
#include "WebKeyCodeToImGui.h"
#include "Containers/TripleBuffer.h"
#include "Interfaces/IPluginManager.h"
#include "Record/imgui-ws-record.h"
#include "Record/ImGuiWS_Replay.h"

FAutoConsoleCommand LaunchImGuiWeb
{
	TEXT("ImGui.WS.LaunchWeb"),
	TEXT("Open ImGui-WS Web"),
	FConsoleCommandDelegate::CreateLambda([]
	{
		const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
		if (Manager->IsEnable())
		{
			FPlatformProcess::LaunchURL(*FString::Printf(TEXT("http://localhost:%d"), Manager->GetPort()), nullptr, nullptr);
		}
		else
		{
			FPlatformProcess::LaunchURL(TEXT("http://localhost#ImGui_WS_Not_Enable!"), nullptr, nullptr);
		}
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
		UImGui_WS_Settings* Settings = GetMutableDefault<UImGui_WS_Settings>();
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
		UImGui_WS_Settings* Settings = GetMutableDefault<UImGui_WS_Settings>();
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
				if (Manager->IsEnable() == false)
				{
					Manager->Enable();
				}
			}
			else
			{
				if (Manager->IsEnable())
				{
					Manager->Disable();
				}
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

void UImGui_WS_WorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
	Manager->WorldSubsystems.Add(this);
}

void UImGui_WS_WorldSubsystem::Deinitialize()
{
	UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
	Manager->WorldSubsystems.RemoveSingle(this);

	Super::Deinitialize();
}

#if PLATFORM_WINDOWS
#include <corecrt_io.h>
#endif
#include <fcntl.h>

class UImGui_WS_Manager::FImpl final : FTickableGameObject
{
	UImGui_WS_Manager& Manager;

public:
	ImGuiWS ImGuiWS;
	FThread WS_Thread;
    std::atomic_bool bRequestedExit{ false };

	ImGuiContext* Context;
	ImPlotContext* PlotContext;
	TArray<ANSICHAR> IniFileNameArray;
	ImFontAtlas FontAtlas;
	float DPIScale = 1.f;

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
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ImGui_WS"));
		const FString PluginResourcesPath = Plugin->GetBaseDir() / TEXT("Resources");

		// fonts
		{
			const UImGui_WS_Settings* Settings = GetDefault<UImGui_WS_Settings>();
			ImFontConfig FontConfig;
			FontConfig.FontDataOwnedByAtlas = false;
			switch (Settings->FontGlyphRanges)
			{
			case EImGuiFontGlyphRanges::Default:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesDefault();
				break;
			case EImGuiFontGlyphRanges::Greek:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesGreek();
				break;
			case EImGuiFontGlyphRanges::Korean:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesKorean();
				break;
			case EImGuiFontGlyphRanges::Japanese:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesJapanese();
				break;
			case EImGuiFontGlyphRanges::ChineseFull:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesChineseFull();
				break;
			case EImGuiFontGlyphRanges::ChineseSimplifiedCommon:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesChineseSimplifiedCommon();
				break;
			case EImGuiFontGlyphRanges::Cyrillic:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesCyrillic();
				break;
			case EImGuiFontGlyphRanges::Thai:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesThai();
				break;
			case EImGuiFontGlyphRanges::Vietnamese:
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesVietnamese();
				break;
			default:
				ensure(false);
				FontConfig.GlyphRanges = FontAtlas.GetGlyphRangesDefault();
			}
			FPlatformString::Strcpy(FontConfig.Name, sizeof(FontConfig.Name), "zpix, 12px");
			const FString ChineseFontPath = PluginResourcesPath / TEXT("zpix.ttf");

			TArray<uint8> Bin;
			ensure(FFileHelper::LoadFileToArray(Bin, *ChineseFontPath));
			constexpr char FontName[] = "zpix, 12px";
			FCStringAnsi::Strcpy(FontConfig.Name, sizeof(FontName), FontName);
			FontAtlas.AddFontFromMemoryTTF(Bin.GetData(), Bin.Num(), 12.0f * DPIScale, &FontConfig);
		}
		
		IMGUI_CHECKVERSION();
		Context = ImGui::CreateContext(&FontAtlas);
		ImGuiIO& IO = ImGui::GetIO();

		// Initialize notify
		ImGui::MergeIconsWithLatestFont(12.f * DPIScale, false);

		IO.MouseDrawCursor = false;
		IO.SetClipboardTextFn = [](void* user_data, const char* text)
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

		ImGui::StyleColorsDark();
		ImGui::GetStyle().AntiAliasedFill = false;
		ImGui::GetStyle().AntiAliasedLines = false;
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		{
			const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT("ImGui_WS");
			// Make sure that directory is created.
			PlatformFile.CreateDirectory(*IniDirectory);

			const auto StringPoint = FTCHARToUTF8(*(IniDirectory / TEXT("Imgui_WS.ini")));
			IniFileNameArray.SetNumUninitialized(StringPoint.Length() + 1);
			FMemory::Memcpy(IniFileNameArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
		}
		IO.IniFilename = IniFileNameArray.GetData();
	    IO.DisplaySize = ImVec2(0, 0);

		PlotContext = ImPlot::CreateContext();
		ImPlot::SetCurrentContext(PlotContext);

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
		WS_Thread = FThread{ TEXT("ImGui_WS"), [this, Interval = GetDefault<UImGui_WS_Settings>()->ServerTickInterval]
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
			unsigned char* pixels;
			int32 width, height;
			ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
			ImGuiWS.SetTexture(0, ImGuiWS::FTexture::Type::Alpha8, width, height, pixels);
		}

		using namespace UnrealImGui;
		Private::UpdateTextureData_WS = [this](FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, uint8* Data)
		{
			static_assert((int32_t)ImGuiWS::FTexture::Type::Alpha8 == (uint8)ETextureFormat::Alpha8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::Gray8 == (uint8)ETextureFormat::Gray8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::RGB24 == (uint8)ETextureFormat::RGB8);
			static_assert((int32_t)ImGuiWS::FTexture::Type::RGBA32 == (uint8)ETextureFormat::RGBA8);
			ImGuiWS.SetTexture(Handle, ImGuiWS::FTexture::Type{ static_cast<uint8>(TextureFormat) }, Width, Height, Data);
		};
	}
	~FImpl() override
	{
		ImGui::DestroyContext(Context);
		ImPlot::DestroyContext(PlotContext);
		UnrealImGui::Private::UpdateTextureData_WS.Reset();
		bRequestedExit = true;
		if (WS_Thread.IsJoinable())
		{
			WS_Thread.Join();
		}
	}
private:
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

		void Update()
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
		    				ImGui::SetClipboardText(Event.ClipboardText.c_str());
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

	bool IsTickableWhenPaused() const { return true; }
	bool IsTickableInEditor() const { return true; }
	UWorld* GetTickableGameObjectWorld() const { return GWorld; }
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UImGui_WS_Manager_FDrawer, STATGROUP_Tickables); }

	struct FImGuiData : FNoncopyable
	{
		ImDrawData CopiedDrawData;
		const ImGuiMouseCursor MouseCursor;
		const int32 ControlId;
		const uint32 ControlIp;
		const ImVec2 MousePos;
		const ImVec2 ViewportSize;
		const uint8 bWantTextInput : 1;

		FImGuiData(const ImDrawData* DrawData, ImGuiWS_Record::FImGuiWS_Replay* Replay, const ImGuiMouseCursor MouseCursor, const int32 ControlId, uint32 ControlIp, const ImVec2& MousePos, const ImVec2& ViewportSize, bool bWantTextInput)
			: CopiedDrawData{ *DrawData }
			, MouseCursor{ MouseCursor }
			, ControlId{ ControlId }
			, ControlIp{ ControlIp }
			, MousePos{ MousePos }
			, ViewportSize{ ViewportSize }
			, bWantTextInput{ bWantTextInput }
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

	void DefaultDraw(float DeltaTime)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("ImGui_WS"))
			{
#if WITH_EDITOR
				if (ImGui::RadioButton(TCHAR_TO_UTF8(*FString::Printf(TEXT("Editor"))), Manager.DrawContextIndex == EditorIndex || Manager.DrawContextIndex >= Manager.WorldSubsystems.Num()))
				{
					Manager.DrawContextIndex = EditorIndex;
				}
				if (Manager.WorldSubsystems.Num() > 0)
				{
					ImGui::Separator();
				}
#endif
				for (int32 Idx = 0; Idx < Manager.WorldSubsystems.Num(); ++Idx)
				{
					const UWorld* World = Manager.WorldSubsystems[Idx]->GetWorld();
					FString WorldDesc;
					switch(World->GetNetMode())
					{
					case NM_Client:
#if WITH_EDITOR
						WorldDesc = FString::Printf(TEXT("Client %d"), World->GetOutermost()->GetPIEInstanceID() - 1);
#else
						WorldDesc = TEXT("Client");
#endif
						break;
					case NM_DedicatedServer:
						WorldDesc = TEXT("DedicatedServer");
						break;
					case NM_ListenServer:
						WorldDesc = TEXT("Server");
						break;
					case NM_Standalone:
						WorldDesc = TEXT("Standalone");
					default:
						break;
					}
					if (ImGui::RadioButton(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d. %s"), Idx, *WorldDesc)), Idx == Manager.DrawContextIndex))
					{
						Manager.DrawContextIndex = Idx;
					}
				}

				ImGui::Separator();
				ImGui::Checkbox("ImGui Demo", &State.bShowImGuiDemo);
				ImGui::Checkbox("ImPlot Demo", &State.bShowPlotDemo);

				ImGui::Separator();
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
						UnrealImGui::InputText("##RecordSavePath", GRecordSaveDirPathString);

						ImGui::SameLine();
						if (ImGui::ArrowButton("SaveRecordFile", ImGuiDir_Down))
						{
							ImGui::OpenPopup("Select Save Record Directory");
						}
						static UnrealImGui::FFileDialogState FileDialogState;
						UnrealImGui::ShowFileDialog("Select Save Record Directory", FileDialogState, SaveFilePath, nullptr, UnrealImGui::FileDialogType::SelectFolder);

						ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 110.f);
						if (ImGui::Button("Start"))
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
						if (ImGui::Button("Cancel"))
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
						UnrealImGui::InputText("##RecordFilePath", LoadFilePath);

						ImGui::SameLine();
						if (ImGui::ArrowButton("OpenRecordFile", ImGuiDir_Down))
						{
							ImGui::OpenPopup("Load Replay File");
						}
						static UnrealImGui::FFileDialogState FileDialogState;
						UnrealImGui::ShowFileDialog("Load Replay File", FileDialogState, LoadFilePath, ".imgrcd", UnrealImGui::FileDialogType::OpenFile);

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

#if WITH_EDITOR
		if (Manager.DrawContextIndex == EditorIndex || Manager.DrawContextIndex >= Manager.WorldSubsystems.Num())
		{
			const FImGui_WS_EditorContext& DrawContext = Manager.EditorContext;
			if (DrawContext.bAlwaysDrawDefaultLayout || DrawContext.OnDraw.IsBound() == false)
			{
				if (GWorld)
				{
					static UImGuiEditorDefaultDebugger* DefaultDebugger = []
					{
						UImGuiEditorDefaultDebugger* Debugger = NewObject<UImGuiEditorDefaultDebugger>();
						Debugger->AddToRoot();
						Debugger->Register();
						return Debugger;
					}();
					DefaultDebugger->Draw(DeltaTime);
				}
			}
			DrawContext.OnDraw.Broadcast(DeltaTime);
		}
		else
#endif
		if (Manager.DrawContextIndex < Manager.WorldSubsystems.Num())
		{
			const UImGui_WS_WorldSubsystem* WorldSubsystem = Manager.WorldSubsystems[Manager.DrawContextIndex];
			WorldSubsystem->Context.OnDraw.Broadcast(DeltaTime);
		}

		// imgui-ws info
		{
			if (ImGui::BeginMainMenuBar())
			{
				const ImVec2 WindowSize = ImGui::GetWindowSize();
				constexpr float StopRecordButtonWidth = 140.f;
				float TotalInfoWidth = 140.f;
				if (RecordSession.IsValid())
				{
					TotalInfoWidth += StopRecordButtonWidth;
				}
				ImGui::Indent(WindowSize.x - TotalInfoWidth);

				if (RecordSession.IsValid())
				{
					const ImGuiStyle& Style = ImGui::GetStyle();
					if (ImGui::Button(TCHAR_TO_UTF8(*FString::Printf(TEXT("Recording (%.0f MB)###StopRecordButton"), RecordSession->totalSize_bytes() / 1024.f / 1024.f)), { StopRecordButtonWidth - Style.FramePadding.x * 2.f, 0.f }))
					{
						StopRecord();
					}
					if (RecordSession.IsValid() && ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Stop Current Record");
						ImGui::Text("Record Info:");
						ImGui::Text("	Frame: %d", RecordSession->nFrames());
						ImGui::Text("	Size: %.2f MB", RecordSession->totalSize_bytes() / 1024.f / 1024.f);
						ImGui::EndTooltip();
					}
				}

				{
					ImGui::Text("Connections: %d", ImGuiWS.NumConnected());
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
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
				ImGui::EndMainMenuBar();
			}
		}

		{
			// Notify
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
			ImGui::RenderNotifications();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
	}

	void Tick(float DeltaTime) override
	{
		if (ImGuiWS.NumConnected() == 0 && RecordSession.IsValid() == false)
		{
	        return;
	    }

		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Tick"), STAT_ImGuiWS_Tick, STATGROUP_ImGui);

	    ImGuiContext* OldContent = ImGui::GetCurrentContext();
	    ON_SCOPE_EXIT
		{
	        ImGui::SetCurrentContext(OldContent);
	    };
	    ImGui::SetCurrentContext(Context);
	    ImGui::NewFrame();

	    // websocket event handling
		auto& Events = ImGuiWS.TakeEvents();
		while (Events.IsEmpty() == false)
		{
			ImGuiWS::FEvent Event;
			Events.Dequeue(Event);
	        State.Handle(Event);
		}
	    State.Update();

	    ImGuiIO& IO = ImGui::GetIO();
	    IO.DeltaTime = VSync.Delta_S();

		bool CloseRecord = false;
		if (RecordReplay.IsValid())
		{
			RecordReplay->Draw(DeltaTime, CloseRecord);
		}
		else
		{
			DefaultDraw(DeltaTime);
		}

	    // generate ImDrawData
	    ImGui::Render();

		{
			// store ImDrawData for asynchronous dispatching to WS clients
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Generate_ImGuiData"), STAT_ImGuiWS_Generate_ImGuiData, STATGROUP_ImGui);
			const ImDrawData* DrawData = ImGui::GetDrawData();
			const ImVec2 MousePos = ImGui::GetMousePos();
			const auto CurControlIp = State.Clients.FindRef(State.CurControlId).Ip;
			ImGuiDataTripleBuffer.WriteAndSwap(MakeShared<FImGuiData>(DrawData, RecordReplay.Get(), ImGui::GetMouseCursor(), State.CurControlId, CurControlIp, MousePos, IO.DisplaySize, IO.WantTextInput));
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
				const ImVec2 MousePos = ImGuiData->MousePos;
				const ImVec2 ViewportSize = ImGuiData->ViewportSize;
				ImGuiWS.SetDrawData(&ImGuiData->CopiedDrawData);
				ImGuiWS.SetDrawInfo({
					ImGuiData->MouseCursor,
					ImGuiData->ControlId,
					ImGuiData->ControlIp,
					MousePos.x,
					MousePos.y,
					ViewportSize.x,
					ViewportSize.y,
					ImGuiData->bWantTextInput
				});
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

public:
	void StartRecord()
	{
		// TODO：改为流式写入内存
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

FImGui_WS_Context* UImGui_WS_Manager::GetImGuiContext(const UWorld* World)
{
	if (World->IsGameWorld())
	{
		UImGui_WS_WorldSubsystem* WorldSubsystem = World->GetSubsystem<UImGui_WS_WorldSubsystem>();
		return WorldSubsystem ? &WorldSubsystem->Context : nullptr;
	}
	return GetImGuiEditorContext();
}

FImGui_WS_EditorContext* UImGui_WS_Manager::GetImGuiEditorContext()
{
#if WITH_EDITOR
	UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
	return &Manager->EditorContext;
#else
	return nullptr;
#endif
}

bool UImGui_WS_Manager::IsSettingsEnable()
{
	const UImGui_WS_Settings* Settings = GetDefault<UImGui_WS_Settings>();
	if (GIsEditor)
	{
		return Settings->bEditorEnableImGui_WS;
	}
	else if (GIsServer)
	{
#if WITH_EDITOR
		return Settings->bEditorEnableImGui_WS;
#endif
		return Settings->bServerEnableImGui_WS;
	}
	else if (GIsClient)
	{
		return Settings->bGameEnableImGui_WS;
	}
	return false;
}

void UImGui_WS_Manager::Enable()
{
	check(IsEnable() == false);
	UE_LOG(LogImGui, Log, TEXT("Enable ImGui WS"));
	Impl = MakeUnique<FImpl>(*this);
}

void UImGui_WS_Manager::Disable()
{
	check(IsEnable());
	Impl.Reset();
	UE_LOG(LogImGui, Log, TEXT("Disable ImGui WS"));
}

int32 UImGui_WS_Manager::GetPort() const
{
	const UImGui_WS_Settings* Settings = GetDefault<UImGui_WS_Settings>();
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

	GRecordSaveDirPathString = *(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("ImGui_WS"));
	CVar_RecordSaveDirPathString->Set(*GRecordSaveDirPathString.ToString());
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		if (GEngine->DeferredCommands.Num() > 0)
		{
			return true;
		}
		if (IsSettingsEnable() && IsEnable() == false)
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
	if (IsEnable())
	{
		Disable();
	}
	Super::Deinitialize();
}
