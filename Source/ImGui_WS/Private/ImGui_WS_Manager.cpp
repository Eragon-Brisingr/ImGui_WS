// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGui_WS_Manager.h"
#include <thread>
#include <sstream>
#include <Interfaces/IPluginManager.h>
#include <Containers/TripleBuffer.h>

#include "imgui.h"
#include "implot.h"
#include "imgui-ws.h"
#include "UnrealImGuiStat.h"
#include "WebKeyCodeToImGui.h"

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

TAutoConsoleVariable<int32> ImGui_WS_Port
{
	TEXT("ImGui.WS.Port"),
	INDEX_NONE,
	TEXT("ImGui-WS Web Port, Only Valid When Pre Game Start. Set In\n")
	TEXT("1. Engine.ini\n [ConsoleVariables] \n ImGui.WS.Port=8890\n")
	TEXT("2. UE4Editor.exe GAMENAME -ExecCmds=\"ImGui.WS.Port 8890\""),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*)
	{
		GetMutableDefault<UImGui_WS_Settings>()->Port = ImGui_WS_Port.GetValueOnGameThread();
	})
};

TAutoConsoleVariable<int32> ImGui_WS_Enable
{
	TEXT("ImGui.WS.Enable"),
	-1,
	TEXT("Set ImGui-WS Enable 0: Disable 1: Enable"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*)
	{
		UImGui_WS_Settings* Settings = GetMutableDefault<UImGui_WS_Settings>();
		if (GIsEditor)
		{
			Settings->bEditorEnableImGui_WS = !!ImGui_WS_Enable.GetValueOnGameThread();
		}
		else if (GIsServer)
		{
			Settings->bServerEnableImGui_WS = !!ImGui_WS_Enable.GetValueOnGameThread();
		}
		else if (GIsClient)
		{
			Settings->bClientEnableImGui_WS = !!ImGui_WS_Enable.GetValueOnGameThread();
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

class UImGui_WS_Manager::FDrawer final : FTickableGameObject
{
	UImGui_WS_Manager& Manager;

public:
	ImGuiWS ImGuiWS;
	ImGuiContext* Context;
	ImPlotContext* PlotContext;
	TArray<ANSICHAR> IniFileNameArray;
	ImFontAtlas FontAtlas;
	float DPIScale = 1.f;
	TTripleBuffer<std::string> ClipboardTextTripleBuffer;

	explicit FDrawer(UImGui_WS_Manager& Manager)
		: Manager(Manager)
	{
		const FString PluginPath = IPluginManager::Get().FindPlugin(TEXT("ImGui_WS"))->GetBaseDir();
		// fonts
		{
			ImFontConfig ChineseFontConfig;
			ChineseFontConfig.GlyphRanges = FontAtlas.GetGlyphRangesChineseSimplifiedCommon();
			FPlatformString::Strcpy(ChineseFontConfig.Name, sizeof(ChineseFontConfig.Name), "Zfull-GB, 12px");
			const FString ChineseFontPath = PluginPath / TEXT("Resources/Zfull-GB.ttf");
			FontAtlas.AddFontFromFileTTF(TCHAR_TO_UTF8(*ChineseFontPath), 12.0f*DPIScale, &ChineseFontConfig);
		}
		
		IMGUI_CHECKVERSION();
		Context = ImGui::CreateContext(&FontAtlas);
		ImGuiIO& IO = ImGui::GetIO();

		IO.KeyMap[ImGuiKey_Tab]         = 9;
		IO.KeyMap[ImGuiKey_LeftArrow]   = 37;
		IO.KeyMap[ImGuiKey_RightArrow]  = 39;
		IO.KeyMap[ImGuiKey_UpArrow]     = 38;
		IO.KeyMap[ImGuiKey_DownArrow]   = 40;
		IO.KeyMap[ImGuiKey_PageUp]      = 33;
		IO.KeyMap[ImGuiKey_PageDown]    = 34;
		IO.KeyMap[ImGuiKey_Home]        = 36;
		IO.KeyMap[ImGuiKey_End]         = 35;
		IO.KeyMap[ImGuiKey_Insert]      = 45;
		IO.KeyMap[ImGuiKey_Delete]      = 46;
		IO.KeyMap[ImGuiKey_Backspace]   = 8;
		IO.KeyMap[ImGuiKey_Space]       = 32;
		IO.KeyMap[ImGuiKey_Enter]       = 13;
		IO.KeyMap[ImGuiKey_Escape]      = 27;
		IO.KeyMap[ImGuiKey_A]           = 65;
		IO.KeyMap[ImGuiKey_C]           = 67;
		IO.KeyMap[ImGuiKey_V]           = 86;
		IO.KeyMap[ImGuiKey_X]           = 88;
		IO.KeyMap[ImGuiKey_Y]           = 89;
		IO.KeyMap[ImGuiKey_Z]           = 90;

		IO.MouseDrawCursor = false;

		static auto SetClipboardTextFn_DefaultImpl = [](void* user_data, const char* text)
		{
			const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
			FDrawer* Drawer = Manager->Drawer;
			Drawer->ClipboardTextTripleBuffer.WriteAndSwap(text);
		};
		IO.SetClipboardTextFn = SetClipboardTextFn_DefaultImpl;
		
		// Enable Docking
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// 16-bit indices + allow large meshes
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		
		ImGui::StyleColorsDark();
		ImGui::GetStyle().AntiAliasedFill = false;
		ImGui::GetStyle().AntiAliasedLines = false;
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;

		{
			const FString IniDirectory = FPaths::ProjectSavedDir() / TEXT("ImGui_WS");
			// Make sure that directory is created.
			IPlatformFile::GetPlatformPhysical().CreateDirectory(*IniDirectory);

			const auto StringPoint = FTCHARToUTF8(*(IniDirectory / TEXT("Imgui_WS.ini")));
			IniFileNameArray.SetNumUninitialized(StringPoint.Length() + 1);
			FMemory::Memcpy(IniFileNameArray.GetData(), StringPoint.Get(), StringPoint.Length() + 1);
		}
		IO.IniFilename = IniFileNameArray.GetData();
	    IO.DisplaySize = ImVec2(0, 0);

		PlotContext = ImPlot::CreateContext();
		ImPlot::SetCurrentContext(PlotContext);
		
		// setup imgui-ws
		const FString HtmlPath = PluginPath / TEXT("Source/ImGui_WS/HTML");
		ImGuiWS.init(Manager.GetPort(), TCHAR_TO_UTF8(*HtmlPath), { "", "index.html", "imgui-ws.js" }, [this]
		{
			WS_ThreadUpdate();
		});

		// prepare font texture
		{
			unsigned char* pixels;
			int width, height;
			ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
			ImGuiWS.setTexture(0, ImGuiWS::Texture::Type::Alpha8, width, height, (const char*)pixels);
		}
	}
	virtual ~FDrawer()
	{
		ImGui::DestroyContext(Context);
		ImPlot::DestroyContext(PlotContext);
	}
private:
	struct FVSync
	{
		FVSync(double RateFps = 60.0) : tStep_us(1000000.0/RateFps) {}

		uint64_t tStep_us;
		uint64_t tLast_us = t_us();
		uint64_t tNext_us = tLast_us + tStep_us;

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

		int CurControlId = -1;
		bool bIsIdControlChanged = false;
		TMap<int32, ClientData> Clients;

		// client input
		TArray<ImGuiWS::Event, TInlineAllocator<16>> PendingEvents;
		// recover key down state
		TArray<ImGuiWS::Event, TInlineAllocator<16>> KeyDownEvents;
		
		void Handle(const ImGuiWS::Event& Event)
		{
		    switch (Event.type)
			{
	        case ImGuiWS::Event::Connected:
				{
	                ClientData& Client = Clients.Add(Event.clientId);
	        		
	        		std::stringstream ss;
			        { int32 a = uint8(Event.ip); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.ip >> 8); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.ip >> 16); if (a < 0) a += 256; ss << a << "."; }
			        { int32 a = uint8(Event.ip >> 24); if (a < 0) a += 256; ss << a; }
	        		Client.Ip = Event.ip;
	        		Client.IpString = ss.str();
	            }
	            break;
	        case ImGuiWS::Event::Disconnected:
				{
	                Clients.Remove(Event.clientId);
	            }
	            break;
	        case ImGuiWS::Event::MouseMove:
	        case ImGuiWS::Event::MouseDown:
	        case ImGuiWS::Event::MouseUp:
	        case ImGuiWS::Event::MouseWheel:
	        case ImGuiWS::Event::KeyUp:
	        case ImGuiWS::Event::KeyDown:
	        case ImGuiWS::Event::KeyPress:
				{
	                if (Event.clientId == CurControlId)
					{
		                PendingEvents.Add(Event);
	                }
	            }
	            break;
			case ImGuiWS::Event::Resize:
				{
					if (Event.clientId == CurControlId)
					{
						ImGuiIO& IO = ImGui::GetIO();
						if (Event.clientId == CurControlId)
						{
							IO.DisplaySize = { (float)Event.client_width, (float)Event.client_height };
						}
					}
				}
		    	break;
		    case ImGuiWS::Event::TakeControl:
			    {
		    		if (auto* Client = Clients.Find(Event.clientId))
		    		{
		    			CurControlId = Event.clientId;
		    			bIsIdControlChanged = true;
		    			Client->ControlStartTime = ImGui::GetTime();
		    		}
			    }
		    	break;
		    case ImGuiWS::Event::PasteClipboard:
		    	{
		    		if (Event.clientId == CurControlId)
		    		{
		    			ImGui::SetClipboardText(Event.clipboard_text.c_str());
		    		}
		    	}
		    	break;
	        default:
				{
	                ensureMsgf(false, TEXT("Unknown input event\n"));
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
			auto SyncKeyMods = [&IO](ImGuiKey Key, bool bDown)
			{
				switch (Key)
				{
				case ImGuiKey_LeftCtrl:
				case ImGuiKey_RightCtrl:
					IO.KeyCtrl = bDown;
					break;
				case ImGuiKey_LeftShift:
				case ImGuiKey_RightShift:
					IO.KeyShift = bDown;
					break;
				case ImGuiKey_LeftAlt:
				case ImGuiKey_RightAlt:
					IO.KeyAlt = bDown;
					break;
				case ImGuiKey_LeftSuper:
				case ImGuiKey_RightSuper:
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
				// when id control changed release all button
				for (const ImGuiWS::Event& Event : KeyDownEvents)
				{
					switch (Event.type)
					{
					case ImGuiWS::Event::MouseDown:
						{
							IO.AddMouseButtonEvent(ConvertWebMouseButtonToImGui(Event.key), false);
						}
						break;
					case ImGuiWS::Event::KeyDown:
						{
							const ImGuiKey Key = ToImGuiKey(EWebKeyCode(Event.key));
							IO.AddKeyEvent(Key, false);
							SyncKeyMods(Key, false);
						}
						break;
					default:
						ensure(false);
					}
				}
				KeyDownEvents.Empty();
				bIsIdControlChanged = false;
			}
		    if (CurControlId > 0)
			{
		    	for (const ImGuiWS::Event& Event : PendingEvents)
		    	{
		    		switch (Event.type)
		    		{
		    		case ImGuiWS::Event::MouseMove:
		    			{
				            IO.AddMousePosEvent(Event.mouse_x, Event.mouse_y);
		    			}
		    			break;
		    		case ImGuiWS::Event::MouseDown:
		    			{
		    				IO.AddMousePosEvent(Event.mouse_x, Event.mouse_y);
		    				IO.AddMouseButtonEvent(ConvertWebMouseButtonToImGui(Event.mouse_but), true);
		    				KeyDownEvents.Add(Event);
		    			}
		    			break;
		    		case ImGuiWS::Event::MouseUp:
		    			{
		    				IO.AddMousePosEvent(Event.mouse_x, Event.mouse_y);
		    				IO.AddMouseButtonEvent(ConvertWebMouseButtonToImGui(Event.mouse_but), false);
		    				KeyDownEvents.RemoveAll([&Event](const ImGuiWS::Event& E) { return E.type == Event.type && E.mouse_but == Event.mouse_but; } );
		    			}
		    			break;
		    		case ImGuiWS::Event::MouseWheel:
		    			{
		    				IO.AddMouseWheelEvent(Event.wheel_x, Event.wheel_y);
		    			}
		    			break;
		    		case ImGuiWS::Event::KeyPress:
		    			{
		    				IO.AddInputCharacter(Event.key);
		    			}
		    			break;
		    		case ImGuiWS::Event::KeyDown:
		    			{
		    				const ImGuiKey Key = ToImGuiKey(EWebKeyCode(Event.key));
		    				IO.AddKeyEvent(Key, true);
		    				SyncKeyMods(Key, true);
		    				KeyDownEvents.Add(Event);
		    			}
		            	break;
		            case ImGuiWS::Event::KeyUp:
			            {
		            		const ImGuiKey Key = ToImGuiKey(EWebKeyCode(Event.key));
		    				IO.AddKeyEvent(Key, false);
				            SyncKeyMods(Key, false);
		    				KeyDownEvents.RemoveAll([&Event](const ImGuiWS::Event& E) { return E.type == Event.type && E.mouse_but == Event.mouse_but; } );
			            }
		            	break;
		            default: ;
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
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UImGui_WS_Manager_FDrawer, STATGROUP_Tickables); }

	struct FImGuiData : FNoncopyable
	{
		ImDrawData CopiedDrawData;
		const ImGuiMouseCursor MouseCursor;
		const int32 ControlId;
		const uint32 ControlIp;
		const ImVec2 MousePos;
		const ImVec2 ViewportSize;

		FImGuiData(const ImDrawData* DrawData, const ImGuiMouseCursor MouseCursor, const int32 ControlId, uint32 ControlIp, const ImVec2& MousePos, const ImVec2& ViewportSize)
			: CopiedDrawData{ *DrawData }
			, MouseCursor(MouseCursor)
			, ControlId(ControlId)
			, ControlIp(ControlIp)
			, MousePos{ MousePos }
			, ViewportSize(ViewportSize)
		{
			CopiedDrawData.CmdLists = new ImDrawList*[DrawData->CmdListsCount];
			for (int32 Idx = 0; Idx < DrawData->CmdListsCount; ++Idx)
			{
				CopiedDrawData.CmdLists[Idx] = DrawData->CmdLists[Idx]->CloneOutput();
			}
		}
		~FImGuiData()
		{
			for (int32 Idx = 0; Idx < CopiedDrawData.CmdListsCount; ++Idx)
			{
				IM_DELETE(CopiedDrawData.CmdLists[Idx]);
			}
			delete CopiedDrawData.CmdLists;
		}
	};
	TTripleBuffer<TSharedPtr<FImGuiData>> ImGuiDataTripleBuffer;
	
	void Tick(float DeltaTime) override
	{
		if (ImGuiWS.nConnected() == 0)
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
	    const auto Events = ImGuiWS.takeEvents();
	    for (const ImGuiWS::Event& Event : Events)
		{
	        State.Handle(Event);
	    }
	    State.Update();

	    ImGuiIO& IO = ImGui::GetIO();
	    IO.DeltaTime = VSync.Delta_S();

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
			Manager.EditorContext.OnDraw.Broadcast(DeltaTime);	
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
				ImGui::Indent(WindowSize.x - 180.f);
				{
					ImGui::Text("Connections: %d", ImGuiWS.nConnected());
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
								ImGui::TextDisabled(" [has controlled %4.2f seconds]", ImGui::GetTime() - client.ControlStartTime);
							}
						}
						ImGui::EndTooltip();
					}
				}
				ImGui::EndMainMenuBar();
			}
		}

		const ImGuiMouseCursor MouseCursor = ImGui::GetMouseCursor();
		
	    // generate ImDrawData
	    ImGui::Render();

		{
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_Generate_ImGuiData"), STAT_ImGuiWS_Generate_ImGuiData, STATGROUP_ImGui);
			const ImDrawData* DrawData = ImGui::GetDrawData();
			const ImVec2 MousePos = ImGui::GetMousePos();
			const auto CurControlIp = State.Clients.FindRef(State.CurControlId).Ip;
			ImGuiDataTripleBuffer.WriteAndSwap(MakeShared<FImGuiData>(DrawData, MouseCursor, State.CurControlId, CurControlIp, MousePos, IO.DisplaySize));
		}

	    ImGui::EndFrame();
	}
	void WS_ThreadUpdate()
	{
		if (ImGuiDataTripleBuffer.IsDirty())
		{
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_SetDrawData"), STAT_ImGuiWS_SetDrawData, STATGROUP_ImGui);
			const FImGuiData* ImGuiData = ImGuiDataTripleBuffer.SwapAndRead().Get();
			// store ImDrawData for asynchronous dispatching to WS clients
			const std::string ClipboardText = ClipboardTextTripleBuffer.IsDirty() ? ClipboardTextTripleBuffer.SwapAndRead() : "";
			const ImVec2 MousePos = ImGuiData->MousePos;
			const ImVec2 ViewportSize = ImGuiData->ViewportSize;
			ImGuiWS.setDrawData(&ImGuiData->CopiedDrawData, ImGuiData->MouseCursor, ClipboardText, ImGuiData->ControlId, ImGuiData->ControlIp, MousePos.x, MousePos.y, ViewportSize.x, ViewportSize.y);
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
	UImGui_WS_WorldSubsystem* WorldSubsystem = World->GetSubsystem<UImGui_WS_WorldSubsystem>();
	return WorldSubsystem ? &WorldSubsystem->Context : nullptr;
}

FImGui_WS_Context* UImGui_WS_Manager::GetImGuiEditorContext()
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
		return Settings->bServerEnableImGui_WS;
	}
	else if (GIsClient)
	{
		return Settings->bClientEnableImGui_WS;
	}
	return false;
}

int32 UImGui_WS_Manager::GetPort() const
{
	const int32 BasePort = GetDefault<UImGui_WS_Settings>()->Port;
#if WITH_EDITOR
	if (GIsEditor)
	{
		// Editor
		return BasePort + 2;
	}
	if (IsRunningDedicatedServer())
	{
		// DedicatedServer
		return BasePort + 1;
	}
#endif
	// Game
	return BasePort;
}

int32 UImGui_WS_Manager::GetConnectionCount() const
{
	return Drawer ? Drawer->ImGuiWS.nConnected() : 0;
}

void UImGui_WS_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		if (GEngine->DeferredCommands.Num() == 0)
		{
			if (IsSettingsEnable() == false)
			{
				return false;
			}
			Drawer = new FDrawer{ *this };
			return false;
		}
		return true;
	}));
}

void UImGui_WS_Manager::Deinitialize()
{
	if (Drawer)
	{
		delete Drawer;
		Drawer = nullptr;
	}

	Super::Deinitialize();
}
