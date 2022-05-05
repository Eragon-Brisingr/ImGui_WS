// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGui_WS_Manager.h"
#include <thread>
#include <map>
#include <Interfaces/IPluginManager.h>

#include "imgui.h"
#include "implot.h"
#include "imgui-ws.h"
#include "UnrealImGuiStat.h"

FAutoConsoleCommand LaunchImGuiWeb
{
	TEXT("ImGui.LaunchWeb"),
	TEXT("Open ImGui-WS Web"),
	FConsoleCommandDelegate::CreateLambda([]
	{
		const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
		FPlatformProcess::LaunchURL(*FString::Printf(TEXT("http://localhost:%d"), Manager->GetPort()), nullptr, nullptr);
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
		UE_LOG(LogTemp, Log, TEXT("ImGui_WS_Port=%d"), ImGui_WS_Port.GetValueOnGameThread());
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

	explicit FDrawer(UImGui_WS_Manager& Manager)
		: Manager(Manager)
	{
		// fonts
		{
			ImFontConfig ChineseFontConfig;
			ChineseFontConfig.GlyphRanges = FontAtlas.GetGlyphRangesChineseSimplifiedCommon();
			FPlatformString::Strcpy(ChineseFontConfig.Name, sizeof(ChineseFontConfig.Name), "Zfull-GB, 12px");
			const FString ChineseFontPath = FPaths::ProjectPluginsDir() / TEXT("UnrealImGui") / TEXT("Resources/Zfull-GB.ttf");
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

		// Enable Docking
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		
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

		PlotContext = ImPlot::CreateContext();
		ImPlot::SetCurrentContext(PlotContext);
		
		// setup imgui-ws
		static FString HtmlPath = IPluginManager::Get().FindPlugin(TEXT("ImGui_WS"))->GetBaseDir() / TEXT("Source/ImGui_WS/HTML");
		ImGuiWS.init(Manager.GetPort(), TCHAR_TO_UTF8(*HtmlPath), { "", "index.html", "imgui-ws.js" });

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
	struct FVSync {
		FVSync(double RateFps = 60.0) : tStep_us(1000000.0/RateFps) {}

		uint64_t tStep_us;
		uint64_t tLast_us = t_us();
		uint64_t tNext_us = tLast_us + tStep_us;

		inline uint64_t t_us() const {
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); // duh ..
		}

		inline void wait() {
			uint64_t tNow_us = t_us();
			while (tNow_us < tNext_us - 100) {
				std::this_thread::sleep_for(std::chrono::microseconds((uint64_t) (0.9*(tNext_us - tNow_us))));
				tNow_us = t_us();
			}

			tNext_us += tStep_us;
		}

		inline float Delta_S() {
			uint64_t tNow_us = t_us();
			uint64_t res = tNow_us - tLast_us;
			tLast_us = tNow_us;
			return float(res)/1e6f;
		}
	};
	struct FState {
		FState() {
			for (int i = 0; i < 512; ++i) {
				lastKeysDown[i] = false;
			}
		}

		bool bShowImGuiDemo = false;
		bool bShowPlotDemo = false;

		// client control management
		struct ClientData {
			bool hasControl = false;

			std::string ip = "---";
		};

		// client control
		float tControl_s = 10.0f;
		float tControlNext_s = 0.0f;

		int controlIteration = 0;
		int curIdControl = -1;
		std::map<int, ClientData> clients;

		// client input
		float lastMousePos[2] = { 0.0, 0.0 };
		bool  lastMouseDown[5] = { false, false, false, false, false };
		float lastMouseWheel = 0.0;
		float lastMouseWheelH = 0.0;

		std::string lastAddText = "";
		bool lastKeysDown[512];

		void Handle(const ImGuiWS::Event& event) {
		    switch (event.type) {
		        case ImGuiWS::Event::Connected:
		            {
		                clients[event.clientId].ip = event.ip;
		            }
		            break;
		        case ImGuiWS::Event::Disconnected:
		            {
		                clients.erase(event.clientId);
		            }
		            break;
		        case ImGuiWS::Event::MouseMove:
		            {
		                if (event.clientId == curIdControl) {
		                    lastMousePos[0] = event.mouse_x;
		                    lastMousePos[1] = event.mouse_y;
		                }
		            }
		            break;
		        case ImGuiWS::Event::MouseDown:
		            {
		                if (event.clientId == curIdControl) {
		                    lastMouseDown[event.mouse_but] = true;
		                    lastMousePos[0] = event.mouse_x;
		                    lastMousePos[1] = event.mouse_y;
		                }
		            }
		            break;
		        case ImGuiWS::Event::MouseUp:
		            {
		                if (event.clientId == curIdControl) {
		                    lastMouseDown[event.mouse_but] = false;
		                    lastMousePos[0] = event.mouse_x;
		                    lastMousePos[1] = event.mouse_y;
		                }
		            }
		            break;
		        case ImGuiWS::Event::MouseWheel:
		            {
		                if (event.clientId == curIdControl) {
		                    lastMouseWheelH = event.wheel_x;
		                    lastMouseWheel  = event.wheel_y;
		                }
		            }
		            break;
		        case ImGuiWS::Event::KeyUp:
		            {
		                if (event.clientId == curIdControl) {
		                    if (event.key > 0) {
		                        lastKeysDown[event.key] = false;
		                    }
		                }
		            }
		            break;
		        case ImGuiWS::Event::KeyDown:
		            {
		                if (event.clientId == curIdControl) {
		                    if (event.key > 0) {
		                        lastKeysDown[event.key] = true;
		                    }
		                }
		            }
		            break;
		        case ImGuiWS::Event::KeyPress:
		            {
		                if (event.clientId == curIdControl) {
		                    lastAddText.resize(1);
		                    lastAddText[0] = event.key;
		                }
		            }
		            break;
		        default:
		            {
		                printf("Unknown input event\n");
		            }
		    }
		}

		void Update() {
		    if (clients.size() > 0 && (clients.find(curIdControl) == clients.end() || ImGui::GetTime() > tControlNext_s)) {
		        if (clients.find(curIdControl) != clients.end()) {
		            clients[curIdControl].hasControl = false;
		        }
		        int k = ++controlIteration % clients.size();
		        auto client = clients.begin();
		        std::advance(client, k);
		        client->second.hasControl = true;
		        curIdControl = client->first;
		        tControlNext_s = ImGui::GetTime() + tControl_s;
		    }

		    if (clients.size() == 0) {
		        curIdControl = -1;
		    }

		    if (curIdControl > 0) {
		        ImGuiIO& io = ImGui::GetIO();
		        io.MousePos = ImVec2{ lastMousePos[0], lastMousePos[1] };
		        io.MouseWheelH = lastMouseWheelH;
		        io.MouseWheel = lastMouseWheel;
		        io.AddMouseWheelEvent(lastMouseWheelH, lastMouseWheel);
		        io.MouseDown[0] = lastMouseDown[0];
		        // JS上2代表右键但是ImGui定义1为右键，转换下
		        io.MouseDown[1] = lastMouseDown[2];
		        io.MouseDown[2] = lastMouseDown[1];
		        io.MouseDown[3] = lastMouseDown[3];
		        io.MouseDown[4] = lastMouseDown[4];

		        if (lastAddText.size() > 0) {
		            io.AddInputCharactersUTF8(lastAddText.c_str());
		        }

		        for (int i = 0; i < 512; ++i) {
		            io.KeysDown[i] = lastKeysDown[i];
		        }

		        lastMouseWheelH = 0.0;
		        lastMouseWheel = 0.0;
		        lastAddText = "";
		    }
		}
	};
	
	FVSync VSync;
	FState State;
	
	bool IsTickableWhenPaused() const { return true; }
	bool IsTickableInEditor() const { return true; }
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UImGui_WS_Manager_FDrawer, STATGROUP_Tickables); }
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
	    for (const ImGuiWS::Event& Event : Events) {
	        State.Handle(Event);
	    }
	    State.Update();

	    ImGuiIO& IO = ImGui::GetIO();
		// TODO：读取当前绘制网页的尺寸
	    IO.DisplaySize = ImVec2(1920, 1080);
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
						ImGui::Text(" Id   Ip addr");
						for (auto & [ cid, client ] : State.clients) {
							ImGui::Text("%3d : %s", cid, client.ip.c_str());
							if (client.hasControl) {
								ImGui::SameLine();
								ImGui::TextDisabled(" [has control for %4.2f seconds]", State.tControlNext_s - ImGui::GetTime());
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
			DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ImGuiWS_SetDrawData"), STAT_ImGuiWS_SetDrawData, STATGROUP_ImGui);
			// store ImDrawData for asynchronous dispatching to WS clients
			ImGuiWS.setDrawData(ImGui::GetDrawData(), MouseCursor);
		}

	    ImGui::EndFrame();
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

int32 UImGui_WS_Manager::GetPort() const
{
	// Console Variable
	const int32 CustomPort = ImGui_WS_Port.GetValueOnGameThread();
	if (CustomPort != INDEX_NONE)
	{
		return CustomPort;
	}
	
	if (GIsEditor)
	{
		// Editor
		return 8890;
	}
	
	if (IsRunningDedicatedServer())
	{
		// DedicatedServer
		return 8891;
	}
	
	// Game
	return 8892;
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
