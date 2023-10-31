# ImGui_WS

[中文](README_ZH.md)|[English](README.md)

![Overview](Docs/Overview.gif)

Unreal ImGui_WS plugin provides the ability to display debugging information from the unreal engine on remote web pages and also supports packaged games. (e.g. standalone DS processes can use this debugger to preview in-game data visually)

## Feature

* ImGui webpage drawing
* Unreal world debugger
  * Top view of the unreal world
  * Details panel
  * World outline view
  * Logging and console features
* Panel layout system
* ImPlot data visualization library embedded


## Learn how to use ImGui

Select ImGui_WS on the ImGui webpage and check ImGuiDemo to open the Demo panel
For code viewing **ImGuiDemo.cpp**, refer to the widget drawing method required for copying the Demo panel
imgui_manual demo web version

> [imgui_manual](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html) demo web version  

## Webpage drawing

Click the ImGui button in the lower right corner to open the corresponding webpage, or enter **ImGui.WS.LaunchWeb** in the console to open the webpage

> ImGui_WS drawing is only enabled by default under the editor and the packaged DS or client adds ExecCmds="ImGui.WS.Enable 1" in the startup parameters to enable drawing

### Switch world context

![SwitchWorldContext](Docs/SwitchWorldContext.gif)  
The previewed world can be selected through the world context option under the ImGui_WS menu

### Port configuration

You can set the port number through Config or the command line

1. Port number can be configured in ProjectSettings - Plugins - ImGui_WS
2. File configured in ImGui_WS.ini 
    > [/Script/ImGui_WS.ImGui_WS_Settings]  
    GamePort=8890  
    ServerPort=8891  
    ServerPort=8892  

3. Startup parameter configuration -ExecCmds="ImGui.WS.Port 8890"

### Draw event

1. Gets ImGuiContext by calling **UImGui_WS_Manager::GetImGuiContext**
2. Function **OnDraw** draws the invoked event for the ImGui_WS drawing 
3. Bind this event to call ImGui to draw a specific panel

### Draw Image

1. UnrealImGui::FImGuiTextureHandle create texture handle
2. Call UnrealImGui::UpdateTextureData update texture data
3. Call ImGui::Image draw the texture

``` cpp
static UnrealImGui::FImGuiTextureHandle TextureHandle = []
{
    UTexture2D* Texture2D = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor"));
    // step 1
    const UnrealImGui::FImGuiTextureHandle Handle{ Texture2D };
    // step 2
    UnrealImGui::UpdateTextureData(Handle, UnrealImGui::ETextureFormat::RGB8, Texture2D);
    return Handle;
}();
// step 3
ImGui::Image(TextureHandle, ImVec2{ 256.f, 256.f });
```

## World Debugger

World Debugger is the default runtime unreal world debugger provided by this plugin, which provides the function of previewing and setting Actor properties in the game world

> You can configure the console variable ImGui.DebugGameWorld to control whether the debug panel is enabled. 
> If you need to turn off this function when enabled by default, you can set ImGuiWorldDebuggerBootstrap:: bLaunchImGuiWorldDebugger = false in the game module StartupModule.

## Top view of the unreal world

To avoid displaying too many unrelated Actors, the default preview will only display Actors drawn that inherit the UImGuiWorldDebuggerDrawerBase declaration.

![Viewport](Docs/Viewport.png)  

### Filter Actors by Type

After entering **type:ClassPath** in the FilterActor search box, only the Actor of the current type will be displayed in the map

### Add the Actor type that needs to be visualized

Create a type that inherits from UImGuiWorldDebuggerDrawerBase

* Add constructor

``` cpp
UShootWeaponBulletDrawer::UShootWeaponBulletDrawer()
{
	// Declare the Actor types supported by the Drawer
	DrawActor = AShootWeaponBullet::StaticClass();
	// Drawn solid radius
	Radius = 10.f;
	// Painted color
	Color = FLinearColor::Red;
}
```

* Rewrite functions such as DrawImGuiDebuggerExtendInfo to add extra debugging information drawing

``` cpp
void UShootWeaponBulletDrawer::DrawImGuiDebuggerExtendInfo(const AActor* Actor, const FImGuiWorldViewportContext& DebuggerContext) const
{
	const AShootWeaponBullet* Bullet = CastChecked<AShootWeaponBullet>(Actor);
	const FVector EndLocation = Bullet->GetActorLocation();
	const FVector StartLocation = EndLocation - Actor->GetVelocity() * DebuggerContext.DeltaSeconds;
	DebuggerContext.DrawLine(FVector2D{ StartLocation }, FVector2D{ EndLocation }, Color);
}
```

### Add extra world information drawing

Inherit UImGuiWorldDebuggerViewportPanel and rewrite the following virtual functions

* DrawDebugInfoUnderActors draws extra debugging information on the lower layer of Actors
* DrawDebugInfoUpperActors draws extra debugging information on the upper layer of Actors

It is recommended to add a **switch** to debug information for each world to avoid displaying too many elements in the debugging world at the same time

``` cpp
// declare switch
UPROPERTY(Config)
uint8 bExampleToggle : 1;

// Add the menu option of whether to enable the switch in the implementation
if (ImGui::BeginMenu("Example Menu"))
{
	{
		bool Value = bExampleToggle;
		if (ImGui::Checkbox("Example Toggle", &Value))
		{
			bShowGlobalLifeTime = Value;
			DebuggerContext.MarkConfigDirty();
		}
	}
	ImGui::EndMenu();
}

// The switch is judged in the logic, and the debug information is drawn when it is turned on.
```

## UObject Details Panel

All properties of the incoming UObject instance can be drawn, and multi-select editing is supported

![Details](Docs/Details.png)  

For usage, please refer to **UImGuiWorldDebuggerDetailsPanel::Draw**

### Add drawing methods of custom types

See how **FStructCustomizerScoped** is used

## Panel layout System

![DefaultLayout](Docs/DefaultLayout.png)  

### Overview

* Inherit UImGuiWorldDebuggerPanelBase to add a new panel to ImGuiWorldDebugger
* Inherit UImGuiWorldDebuggerLayoutBase to add layout description to ImGuiWorldDebugger

### UnrealImGuiPanelBuilder

FUnrealImGuiPanelBuilder is used to build the layout of its window, and the following parameters need to be configured

| property          | describe                                                                                   |
|-------------------|--------------------------------------------------------------------------------------------|
| DockSpaceName     | the name of the layout system                                                              |

After configuring the description information of the layout system, call the following methods to draw the panel

| method            | describe                                                      |
|-------------------|---------------------------------------------------------------|
| Register          | Register the layout system and call it when it is created     |
| Unregister        | Unregister the layout system and call it when it is destroyed |
| DrawPanels        | Draw the panels under the layout system                       |
| LoadDefaultLayout | Reload the activated layout                                   |

### Add layout

Inherit the layout base class types supported under FUnrealImGuiPanelBuilder. For example, ImGuiWorldDebugger extended layout inherits UImGuiWorldDebuggerLayoutBase

* Configure LayoutName. Unnamed layouts will not be displayed
* implement LoadDefaultLayout to declare the default layout structure
* implement ShouldCreateLayout to declare support owner

#### ImGuiWorldDebugger default layout example

The default layout divides the viewport into four areas

``` cpp
UCLASS()
class UImGuiWorldDebuggerDefaultLayout : public UImGuiWorldDebuggerLayoutBase
{
	GENERATED_BODY()
public:
	// Declare the DockId as the configuration when the panel registers the Dock
	enum EDockId
	{
		Viewport,
		Outliner,
		Details,
		Utils,
	};
	UImGuiWorldDebuggerDefaultLayout();
	void LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder) override;
};
```

``` cpp
UImGuiWorldDebuggerDefaultLayout::UImGuiWorldDebuggerDefaultLayout()
{
	// Set layout name
	LayoutName = LOCTEXT("Default", "Default");
}

void UImGuiWorldDebuggerDefaultLayout::LoadDefaultLayout(UObject* Owner, const FUnrealImGuiPanelBuilder& LayoutBuilder)
{
	const ImGuiID DockId = ImGui::DockBuilderAddNode(DockSpaceId, ImGuiDockNodeFlags_None);

	// Call DockBuilderSplitNode to divide the layout
	ImGuiID RemainAreaId;
	ImGuiID ViewportId = ImGui::DockBuilderSplitNode(DockSpaceId, ImGuiDir_Left, 0.7f, nullptr, &RemainAreaId);
	const ImGuiID UtilsId = ImGui::DockBuilderSplitNode(ViewportId, ImGuiDir_Down, 0.3f, nullptr, &ViewportId);
	const ImGuiID OutlinerId = ImGui::DockBuilderSplitNode(RemainAreaId, ImGuiDir_Up, 0.3f, nullptr, &RemainAreaId);
	const ImGuiID DetailsId = ImGui::DockBuilderSplitNode(RemainAreaId, ImGuiDir_Down, 0.7f, nullptr, &RemainAreaId);

	// Add a mapping relationship between the declared DockId and the actual ID of ImGui
	const TMap<int32, ImGuiID> DockIdMap
	{
		{ Viewport, ViewportId },
		{ Outliner, OutlinerId },
		{ Details, DetailsId },
		{ Utils, UtilsId },
	};
	// Subpanel application layout information
	ApplyPanelDockSettings(LayoutBuilder, DockIdMap, EDockId::Utils);

	ImGui::DockBuilderFinish(DockId);
}
```

### Add panel

Inherit the panel base class types supported under FUnrealImGuiPanelBuilder. For example, the ImGuiWorldDebugger extended panel inherits UImGuiWorldDebuggerPanelBase

* Configure Title. Unnamed panels will not be registered
* Configure DefaultDockSpace to add the position of the panel in the layout
* Implement Draw to realize panel drawing
* Implement ShouldCreatePanel to declare support owner (Optional)

#### ImGuiWorldDebuggerViewportPanel panel example

``` cpp
UImGuiWorldDebuggerViewportPanel::UImGuiWorldDebuggerViewportPanel()
{
	// Declares that the menu bar needs to be displayed
	ImGuiWindowFlags = ImGuiWindowFlags_MenuBar;
	// Set panel name
	Title = LOCTEXT("Viewport", "Viewport");
	// The default position in the ImGuiWorldDebuggerDefaultLayout layout is Viewport
	DefaultDockSpace =
	{
		{ UImGuiWorldDebuggerDefaultLayout::StaticClass()->GetFName(), UImGuiWorldDebuggerDefaultLayout::EDockId::Viewport }
	};
}
```

## Viewport Extent

* Inherit UUnrealImGuiViewportBase to declare viewport panel
* Inherit UUnrealImGuiViewportExtentBase to declare viewport extent draw
  * Implement DrawViewportMenu draw menu
  * Implement DrawViewportContent draw viewport content element
  * Implement ShouldCreateExtent to declare support viewport type (optional)

## Bubbling message prompt

[imgui-notify](https://github.com/patrickcjk/imgui-notify)

Call **ImGui::InsertNotification** to use the global bubbling message prompt

![Bubbling message](Docs/Notification.gif)  

## Record drawing data and playback (experimental)

### Recording method

Start recording:

* Menu->ImGui_WS->Start Record
* Console input **ImGui.WS.StartRecord**

To end recording:

* Menu->ImGui_WS->Stop Record
* Console input **ImGui.WS.StopRecord**

### Play back recorded data

Menu->ImGui_WS->Load Record, select the recorded file for review

## Credits

* [ImGui](https://github.com/ocornut/imgui)
  ImGui's repository, which contains ImGui's Wiki
* [imgui-ws](https://github.com/ggerganov/imgui-ws)
  Implemented ImGui web page drawing
