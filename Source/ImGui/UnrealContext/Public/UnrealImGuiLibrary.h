// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "InputCoreTypes.h"
#include "UnrealImGuiKeyUtils.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiTexture.h"
#include "UnrealImGuiWrapper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealImGuiLibrary.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiWindowFlags : int32
{
	None                      = ImGuiWindowFlags_None                     ,
    NoTitleBar                = ImGuiWindowFlags_NoTitleBar               ,   // Disable title-bar
    NoResize                  = ImGuiWindowFlags_NoResize                 ,   // Disable user resizing with the lower-right grip
    NoMove                    = ImGuiWindowFlags_NoMove                   ,   // Disable user moving the window
    NoScrollbar               = ImGuiWindowFlags_NoScrollbar              ,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    NoScrollWithMouse         = ImGuiWindowFlags_NoScrollWithMouse        ,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    NoCollapse                = ImGuiWindowFlags_NoCollapse               ,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
    AlwaysAutoResize          = ImGuiWindowFlags_AlwaysAutoResize         ,   // Resize every window to its content every frame
    NoBackground              = ImGuiWindowFlags_NoBackground             ,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    NoSavedSettings           = ImGuiWindowFlags_NoSavedSettings          ,   // Never load/save settings in .ini file
    NoMouseInputs             = ImGuiWindowFlags_NoMouseInputs            ,   // Disable catching mouse, hovering test with pass through.
    MenuBar                   = ImGuiWindowFlags_MenuBar                  ,  // Has a menu-bar
    HorizontalScrollbar       = ImGuiWindowFlags_HorizontalScrollbar      ,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    NoFocusOnAppearing        = ImGuiWindowFlags_NoFocusOnAppearing       ,  // Disable taking focus when transitioning from hidden to visible state
    NoBringToFrontOnFocus     = ImGuiWindowFlags_NoBringToFrontOnFocus    ,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    AlwaysVerticalScrollbar   = ImGuiWindowFlags_AlwaysVerticalScrollbar  ,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    NoNavInputs               = ImGuiWindowFlags_NoNavInputs              ,  // No gamepad/keyboard navigation within the window
    NoNavFocus                = ImGuiWindowFlags_NoNavFocus               ,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    UnsavedDocument           = ImGuiWindowFlags_UnsavedDocument          ,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    NoDocking                 = ImGuiWindowFlags_NoDocking                ,  // Disable docking of this window
    NoNav                     = ImGuiWindowFlags_NoNav                    ,
    NoDecoration              = ImGuiWindowFlags_NoDecoration             ,
    NoInputs                  = ImGuiWindowFlags_NoInputs                 ,
};
ENUM_CLASS_FLAGS(EImGuiWindowFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiChildFlags : int32
{
	None                   = ImGuiChildFlags_None                    ,
	Border                 = ImGuiChildFlags_Border                  ,   // Show an outer border and enable WindowPadding. (IMPORTANT: this is always == 1 == true for legacy reason)
	AlwaysUseWindowPadding = ImGuiChildFlags_AlwaysUseWindowPadding  ,   // Pad with style.WindowPadding even if no border are drawn (no padding by default for non-bordered child windows because it makes more sense)
	ResizeX                = ImGuiChildFlags_ResizeX                 ,   // Allow resize from right border (layout direction). Enable .ini saving (unless ImGuiWindowFlags_NoSavedSettings passed to window flags)
	ResizeY                = ImGuiChildFlags_ResizeY                 ,   // Allow resize from bottom border (layout direction). "
	AutoResizeX            = ImGuiChildFlags_AutoResizeX             ,   // Enable auto-resizing width. Read "IMPORTANT: Size measurement" details above.
	AutoResizeY            = ImGuiChildFlags_AutoResizeY             ,   // Enable auto-resizing height. Read "IMPORTANT: Size measurement" details above.
	AlwaysAutoResize       = ImGuiChildFlags_AlwaysAutoResize        ,   // Combined with AutoResizeX/AutoResizeY. Always measure size even when child is hidden, always return true, always disable clipping optimization! NOT RECOMMENDED.
	FrameStyle             = ImGuiChildFlags_FrameStyle              ,   // Style the child window like a framed item: use FrameBg, FrameRounding, FrameBorderSize, FramePadding instead of ChildBg, ChildRounding, ChildBorderSize, WindowPadding.
};
ENUM_CLASS_FLAGS(EImGuiChildFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiFocusedFlags : int32
{
	None                = ImGuiFocusedFlags_None                     ,
	ChildWindows        = ImGuiFocusedFlags_ChildWindows             ,   // Return true if any children of the window is focused
	RootWindow          = ImGuiFocusedFlags_RootWindow               ,   // Test from root window (top most parent of the current hierarchy)
	AnyWindow           = ImGuiFocusedFlags_AnyWindow                ,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
	NoPopupHierarchy    = ImGuiFocusedFlags_NoPopupHierarchy         ,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
	DockHierarchy       = ImGuiFocusedFlags_DockHierarchy            ,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
	RootAndChildWindows = ImGuiFocusedFlags_RootAndChildWindows      ,
};
ENUM_CLASS_FLAGS(EImGuiFocusedFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiHoveredFlags : int32
{
    None                          = ImGuiHoveredFlags_None                          ,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    ChildWindows                  = ImGuiHoveredFlags_ChildWindows                  ,   // IsWindowHovered() only: Return true if any children of the window is hovered
    RootWindow                    = ImGuiHoveredFlags_RootWindow                    ,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    AnyWindow                     = ImGuiHoveredFlags_AnyWindow                     ,   // IsWindowHovered() only: Return true if any window is hovered
    NoPopupHierarchy              = ImGuiHoveredFlags_NoPopupHierarchy              ,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    DockHierarchy                 = ImGuiHoveredFlags_DockHierarchy                 ,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    AllowWhenBlockedByPopup       = ImGuiHoveredFlags_AllowWhenBlockedByPopup       ,   // Return true even if a popup window is normally blocking access to this item/window
    AllowWhenBlockedByActiveItem  = ImGuiHoveredFlags_AllowWhenBlockedByActiveItem  ,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    AllowWhenOverlappedByItem     = ImGuiHoveredFlags_AllowWhenOverlappedByItem     ,   // IsItemHovered() only: Return true even if the item uses AllowOverlap mode and is overlapped by another hoverable item.
    AllowWhenOverlappedByWindow   = ImGuiHoveredFlags_AllowWhenOverlappedByWindow   ,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window.
    AllowWhenDisabled             = ImGuiHoveredFlags_AllowWhenDisabled             ,  // IsItemHovered() only: Return true even if the item is disabled
    NoNavOverride                 = ImGuiHoveredFlags_NoNavOverride                 ,  // IsItemHovered() only: Disable using gamepad/keyboard navigation state when active, always query mouse
    AllowWhenOverlapped           = ImGuiHoveredFlags_AllowWhenOverlapped           ,
    RectOnly                      = ImGuiHoveredFlags_RectOnly                      ,
    RootAndChildWindows           = ImGuiHoveredFlags_RootAndChildWindows           ,

    // Tooltips mode
    // - typically used in IsItemHovered() + SetTooltip() sequence.
    // - this is a shortcut to pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' where you can reconfigure desired behavior.
    //   e.g. 'TooltipHoveredFlagsForMouse' defaults to 'ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort'.
    // - for frequently actioned or hovered items providing a tooltip, you want may to use ImGuiHoveredFlags_ForTooltip (stationary + delay) so the tooltip doesn't show too often.
    // - for items which main purpose is to be hovered, or items with low affordance, or in less consistent apps, prefer no delay or shorter delay.
    ForTooltip = ImGuiHoveredFlags_ForTooltip                    ,  // Shortcut for standard flags when using IsItemHovered() + SetTooltip() sequence.

    // (Advanced) Mouse Hovering delays.
    // - generally you can use ImGuiHoveredFlags_ForTooltip to use application-standardized flags.
    // - use those if you need specific overrides.
    Stationary    = ImGuiHoveredFlags_Stationary                    ,  // Require mouse to be stationary for style.HoverStationaryDelay (~0.15 sec) _at least one time_. After this, can move on same item/window. Using the stationary test tends to reduces the need for a long delay.
    DelayNone     = ImGuiHoveredFlags_DelayNone                     ,  // IsItemHovered() only: Return true immediately (default). As this is the default you generally ignore this.
    DelayShort    = ImGuiHoveredFlags_DelayShort                    ,  // IsItemHovered() only: Return true after style.HoverDelayShort elapsed (~0.15 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    DelayNormal   = ImGuiHoveredFlags_DelayNormal                   ,  // IsItemHovered() only: Return true after style.HoverDelayNormal elapsed (~0.40 sec) (shared between items) + requires mouse to be stationary for style.HoverStationaryDelay (once per item).
    NoSharedDelay = ImGuiHoveredFlags_NoSharedDelay                 ,  // IsItemHovered() only: Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};
ENUM_CLASS_FLAGS(EImGuiHoveredFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiCond : int32
{
	None         = ImGuiCond_None          ,        // No condition (always set the variable), same as _Always
	Always       = ImGuiCond_Always        ,   // No condition (always set the variable), same as _None
	Once         = ImGuiCond_Once          ,   // Set the variable once per runtime session (only the first call will succeed)
	FirstUseEver = ImGuiCond_FirstUseEver  ,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
	Appearing    = ImGuiCond_Appearing     ,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};
ENUM_CLASS_FLAGS(EImGuiCond);

UENUM()
enum class EImGuiCol : uint8
{
    Text = ImGuiCol_Text,
    TextDisabled = ImGuiCol_TextDisabled,
    WindowBg = ImGuiCol_WindowBg,              // Background of normal windows
    ChildBg = ImGuiCol_ChildBg,               // Background of child windows
    PopupBg = ImGuiCol_PopupBg,               // Background of popups, menus, tooltips windows
    Border = ImGuiCol_Border,
    BorderShadow = ImGuiCol_BorderShadow,
    FrameBg = ImGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    FrameBgHovered = ImGuiCol_FrameBgHovered,
    FrameBgActive = ImGuiCol_FrameBgActive,
    TitleBg = ImGuiCol_TitleBg,               // Title bar
    TitleBgActive = ImGuiCol_TitleBgActive,         // Title bar when focused
    TitleBgCollapsed = ImGuiCol_TitleBgCollapsed,      // Title bar when collapsed
    MenuBarBg = ImGuiCol_MenuBarBg,
    ScrollbarBg = ImGuiCol_ScrollbarBg,
    ScrollbarGrab = ImGuiCol_ScrollbarGrab,
    ScrollbarGrabHovered = ImGuiCol_ScrollbarGrabHovered,
    ScrollbarGrabActive = ImGuiCol_ScrollbarGrabActive,
    CheckMark = ImGuiCol_CheckMark,             // Checkbox tick and RadioButton circle
    SliderGrab = ImGuiCol_SliderGrab,
    SliderGrabActive = ImGuiCol_SliderGrabActive,
    Button = ImGuiCol_Button,
    ButtonHovered = ImGuiCol_ButtonHovered,
    ButtonActive = ImGuiCol_ButtonActive,
    Header = ImGuiCol_Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    HeaderHovered = ImGuiCol_HeaderHovered,
    HeaderActive = ImGuiCol_HeaderActive,
    Separator = ImGuiCol_Separator,
    SeparatorHovered = ImGuiCol_SeparatorHovered,
    SeparatorActive = ImGuiCol_SeparatorActive,
    ResizeGrip = ImGuiCol_ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
    ResizeGripHovered = ImGuiCol_ResizeGripHovered,
    ResizeGripActive = ImGuiCol_ResizeGripActive,
    Tab = ImGuiCol_Tab,                   // TabItem in a TabBar
    TabHovered = ImGuiCol_TabHovered,
    TabActive = ImGuiCol_TabActive,
    TabUnfocused = ImGuiCol_TabUnfocused,
    TabUnfocusedActive = ImGuiCol_TabUnfocusedActive,
    DockingPreview = ImGuiCol_DockingPreview,        // Preview overlay color when about to docking something
    DockingEmptyBg = ImGuiCol_DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
    PlotLines = ImGuiCol_PlotLines,
    PlotLinesHovered = ImGuiCol_PlotLinesHovered,
    PlotHistogram = ImGuiCol_PlotHistogram,
    PlotHistogramHovered = ImGuiCol_PlotHistogramHovered,
    TableHeaderBg = ImGuiCol_TableHeaderBg,         // Table header background
    TableBorderStrong = ImGuiCol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    TableBorderLight = ImGuiCol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    TableRowBg = ImGuiCol_TableRowBg,            // Table row background (even rows)
    TableRowBgAlt = ImGuiCol_TableRowBgAlt,         // Table row background (odd rows)
    TextSelectedBg = ImGuiCol_TextSelectedBg,
    DragDropTarget = ImGuiCol_DragDropTarget,        // Rectangle highlighting a drop target
    NavHighlight = ImGuiCol_NavHighlight,          // Gamepad/keyboard: current highlighted item
    NavWindowingHighlight = ImGuiCol_NavWindowingHighlight, // Highlight window when using CTRL+TAB
    NavWindowingDimBg = ImGuiCol_NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    ModalWindowDimBg = ImGuiCol_ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
};

UENUM()
enum class EImGuiStyleVar : uint8
{
    Alpha = ImGuiStyleVar_Alpha,               // float     Alpha
    DisabledAlpha = ImGuiStyleVar_DisabledAlpha,       // float     DisabledAlpha
    WindowPadding = ImGuiStyleVar_WindowPadding,       // ImVec2    WindowPadding
    WindowRounding = ImGuiStyleVar_WindowRounding,      // float     WindowRounding
    WindowBorderSize = ImGuiStyleVar_WindowBorderSize,    // float     WindowBorderSize
    WindowMinSize = ImGuiStyleVar_WindowMinSize,       // ImVec2    WindowMinSize
    WindowTitleAlign = ImGuiStyleVar_WindowTitleAlign,    // ImVec2    WindowTitleAlign
    ChildRounding = ImGuiStyleVar_ChildRounding,       // float     ChildRounding
    ChildBorderSize = ImGuiStyleVar_ChildBorderSize,     // float     ChildBorderSize
    PopupRounding = ImGuiStyleVar_PopupRounding,       // float     PopupRounding
    PopupBorderSize = ImGuiStyleVar_PopupBorderSize,     // float     PopupBorderSize
    FramePadding = ImGuiStyleVar_FramePadding,        // ImVec2    FramePadding
    FrameRounding = ImGuiStyleVar_FrameRounding,       // float     FrameRounding
    FrameBorderSize = ImGuiStyleVar_FrameBorderSize,     // float     FrameBorderSize
    ItemSpacing = ImGuiStyleVar_ItemSpacing,         // ImVec2    ItemSpacing
    ItemInnerSpacing = ImGuiStyleVar_ItemInnerSpacing,    // ImVec2    ItemInnerSpacing
    IndentSpacing = ImGuiStyleVar_IndentSpacing,       // float     IndentSpacing
    CellPadding = ImGuiStyleVar_CellPadding,         // ImVec2    CellPadding
    ScrollbarSize = ImGuiStyleVar_ScrollbarSize,       // float     ScrollbarSize
    ScrollbarRounding = ImGuiStyleVar_ScrollbarRounding,   // float     ScrollbarRounding
    GrabMinSize = ImGuiStyleVar_GrabMinSize,         // float     GrabMinSize
    GrabRounding = ImGuiStyleVar_GrabRounding,        // float     GrabRounding
    TabRounding = ImGuiStyleVar_TabRounding,         // float     TabRounding
    TabBarBorderSize = ImGuiStyleVar_TabBarBorderSize,    // float     TabBarBorderSize
    ButtonTextAlign = ImGuiStyleVar_ButtonTextAlign,     // ImVec2    ButtonTextAlign
    SelectableTextAlign = ImGuiStyleVar_SelectableTextAlign, // ImVec2    SelectableTextAlign
    SeparatorTextBorderSize = ImGuiStyleVar_SeparatorTextBorderSize,// float  SeparatorTextBorderSize
    SeparatorTextAlign = ImGuiStyleVar_SeparatorTextAlign,  // ImVec2    SeparatorTextAlign
    SeparatorTextPadding = ImGuiStyleVar_SeparatorTextPadding,// ImVec2    SeparatorTextPadding
    DockingSeparatorSize = ImGuiStyleVar_DockingSeparatorSize,// float     DockingSeparatorSize
};

UENUM()
enum class EImGuiDir : uint8
{
	Left   = ImGuiDir_Left,
	Right  = ImGuiDir_Right,
	Up     = ImGuiDir_Up,
	Down   = ImGuiDir_Down,
};

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiComboFlags : int32
{
	None            = ImGuiComboFlags_None                    ,
	PopupAlignLeft  = ImGuiComboFlags_PopupAlignLeft          ,   // Align the popup toward the left by default
	HeightSmall     = ImGuiComboFlags_HeightSmall             ,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
	HeightRegular   = ImGuiComboFlags_HeightRegular           ,   // Max ~8 items visible (default)
	HeightLarge     = ImGuiComboFlags_HeightLarge             ,   // Max ~20 items visible
	HeightLargest   = ImGuiComboFlags_HeightLargest           ,   // As many fitting items as possible
	NoArrowButton   = ImGuiComboFlags_NoArrowButton           ,   // Display on the preview box without the square arrow button
	NoPreview       = ImGuiComboFlags_NoPreview               ,   // Display only a square arrow button
	WidthFitPreview = ImGuiComboFlags_WidthFitPreview         ,   // Width dynamically calculated from preview contents
};
ENUM_CLASS_FLAGS(EImGuiComboFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiSliderFlags : int32
{
	None            = ImGuiSliderFlags_None                   ,
	AlwaysClamp     = ImGuiSliderFlags_AlwaysClamp            ,       // Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
	Logarithmic     = ImGuiSliderFlags_Logarithmic            ,       // Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
	NoRoundToFormat = ImGuiSliderFlags_NoRoundToFormat        ,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)
	NoInput         = ImGuiSliderFlags_NoInput                ,       // Disable CTRL+Click or Enter key allowing to input text directly into the widget
};
ENUM_CLASS_FLAGS(EImGuiSliderFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiInputTextFlags : int32
{
    None                = ImGuiInputTextFlags_None                ,
    CharsDecimal        = ImGuiInputTextFlags_CharsDecimal        ,   // Allow 0123456789.+-*/
    CharsHexadecimal    = ImGuiInputTextFlags_CharsHexadecimal    ,   // Allow 0123456789ABCDEFabcdef
    CharsUppercase      = ImGuiInputTextFlags_CharsUppercase      ,   // Turn a..z into A..Z
    CharsNoBlank        = ImGuiInputTextFlags_CharsNoBlank        ,   // Filter out spaces, tabs
    AutoSelectAll       = ImGuiInputTextFlags_AutoSelectAll       ,   // Select entire text when first taking mouse focus
    EnterReturnsTrue    = ImGuiInputTextFlags_EnterReturnsTrue    ,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
    CallbackCompletion  = ImGuiInputTextFlags_CallbackCompletion  ,   // Callback on pressing TAB (for completion handling)
    CallbackHistory     = ImGuiInputTextFlags_CallbackHistory     ,   // Callback on pressing Up/Down arrows (for history handling)
    CallbackAlways      = ImGuiInputTextFlags_CallbackAlways      ,   // Callback on each iteration. User code may query cursor position, modify text buffer.
    CallbackCharFilter  = ImGuiInputTextFlags_CallbackCharFilter  ,   // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    AllowTabInput       = ImGuiInputTextFlags_AllowTabInput       ,  // Pressing TAB input a '\t' character into the text field
    CtrlEnterForNewLine = ImGuiInputTextFlags_CtrlEnterForNewLine ,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
    NoHorizontalScroll  = ImGuiInputTextFlags_NoHorizontalScroll  ,  // Disable following the cursor horizontally
    AlwaysOverwrite     = ImGuiInputTextFlags_AlwaysOverwrite     ,  // Overwrite mode
    ReadOnly            = ImGuiInputTextFlags_ReadOnly            ,  // Read-only mode
    Password            = ImGuiInputTextFlags_Password            ,  // Password mode, display all characters as '*'
    NoUndoRedo          = ImGuiInputTextFlags_NoUndoRedo          ,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
    CharsScientific     = ImGuiInputTextFlags_CharsScientific     ,  // Allow 0123456789.+-*/eE (Scientific notation input)
    CallbackResize      = ImGuiInputTextFlags_CallbackResize      ,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
    CallbackEdit        = ImGuiInputTextFlags_CallbackEdit        ,  // Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
    EscapeClearsAll     = ImGuiInputTextFlags_EscapeClearsAll     ,  // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)
};
ENUM_CLASS_FLAGS(EImGuiInputTextFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiColorEditFlags : int32
{
    None            = ImGuiColorEditFlags_None            ,
    NoAlpha         = ImGuiColorEditFlags_NoAlpha         ,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    NoPicker        = ImGuiColorEditFlags_NoPicker        ,   //              // ColorEdit: disable picker when clicking on color square.
    NoOptions       = ImGuiColorEditFlags_NoOptions       ,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    NoSmallPreview  = ImGuiColorEditFlags_NoSmallPreview  ,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    NoInputs        = ImGuiColorEditFlags_NoInputs        ,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    NoTooltip       = ImGuiColorEditFlags_NoTooltip       ,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    NoLabel         = ImGuiColorEditFlags_NoLabel         ,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    NoSidePreview   = ImGuiColorEditFlags_NoSidePreview   ,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    NoDragDrop      = ImGuiColorEditFlags_NoDragDrop      ,   //              // ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.
    NoBorder        = ImGuiColorEditFlags_NoBorder        ,  //              // ColorButton: disable border (which is enforced by default)

    // User Options (right-click on widget to change some of them).
    AlphaBar         = ImGuiColorEditFlags_AlphaBar        ,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    AlphaPreview     = ImGuiColorEditFlags_AlphaPreview    ,  //              // ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.
    AlphaPreviewHalf = ImGuiColorEditFlags_AlphaPreviewHalf,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.
    HDR              = ImGuiColorEditFlags_HDR             ,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).
    DisplayRGB       = ImGuiColorEditFlags_DisplayRGB      ,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    DisplayHSV       = ImGuiColorEditFlags_DisplayHSV      ,  // [Display]    // "
    DisplayHex       = ImGuiColorEditFlags_DisplayHex      ,  // [Display]    // "
    Uint8            = ImGuiColorEditFlags_Uint8           ,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    Float            = ImGuiColorEditFlags_Float           ,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    PickerHueBar     = ImGuiColorEditFlags_PickerHueBar    ,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    PickerHueWheel   = ImGuiColorEditFlags_PickerHueWheel  ,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    InputRGB         = ImGuiColorEditFlags_InputRGB        ,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    InputHSV         = ImGuiColorEditFlags_InputHSV        ,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.
};
ENUM_CLASS_FLAGS(EImGuiColorEditFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTreeNodeFlags : int32
{
    None                 = ImGuiTreeNodeFlags_None                 ,
    Selected             = ImGuiTreeNodeFlags_Selected             ,   // Draw as selected
    Framed               = ImGuiTreeNodeFlags_Framed               ,   // Draw frame with background (e.g. for CollapsingHeader)
    AllowOverlap         = ImGuiTreeNodeFlags_AllowOverlap         ,   // Hit testing to allow subsequent widgets to overlap this one
    NoTreePushOnOpen     = ImGuiTreeNodeFlags_NoTreePushOnOpen     ,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    NoAutoOpenOnLog      = ImGuiTreeNodeFlags_NoAutoOpenOnLog      ,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    DefaultOpen          = ImGuiTreeNodeFlags_DefaultOpen          ,   // Default node to be open
    OpenOnDoubleClick    = ImGuiTreeNodeFlags_OpenOnDoubleClick    ,   // Need double-click to open node
    OpenOnArrow          = ImGuiTreeNodeFlags_OpenOnArrow          ,   // Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
    Leaf                 = ImGuiTreeNodeFlags_Leaf                 ,   // No collapsing, no arrow (use as a convenience for leaf nodes).
    Bullet               = ImGuiTreeNodeFlags_Bullet               ,   // Display a bullet instead of arrow. IMPORTANT: node can still be marked open/close if you don't set the _Leaf flag!
    FramePadding         = ImGuiTreeNodeFlags_FramePadding         ,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
    SpanAvailWidth       = ImGuiTreeNodeFlags_SpanAvailWidth       ,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
    SpanFullWidth        = ImGuiTreeNodeFlags_SpanFullWidth        ,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
    SpanAllColumns       = ImGuiTreeNodeFlags_SpanAllColumns       ,  // Frame will span all columns of its container table (text will still fit in current column)
    NavLeftJumpsBackHere = ImGuiTreeNodeFlags_NavLeftJumpsBackHere ,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)
    CollapsingHeader     = ImGuiTreeNodeFlags_CollapsingHeader     ,
};
ENUM_CLASS_FLAGS(EImGuiTreeNodeFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiSelectableFlags : int32
{
	None             = ImGuiSelectableFlags_None               ,
	DontClosePopups  = ImGuiSelectableFlags_DontClosePopups    ,   // Clicking this doesn't close parent popup window
	SpanAllColumns   = ImGuiSelectableFlags_SpanAllColumns     ,   // Frame will span all columns of its container table (text will still fit in current column)
	AllowDoubleClick = ImGuiSelectableFlags_AllowDoubleClick   ,   // Generate press events on double clicks too
	Disabled         = ImGuiSelectableFlags_Disabled           ,   // Cannot be selected, display grayed out text
	AllowOverlap     = ImGuiSelectableFlags_AllowOverlap       ,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
};
ENUM_CLASS_FLAGS(EImGuiSelectableFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiPopupFlags : int32
{
	None                    = ImGuiPopupFlags_None                    ,
	MouseButtonLeft         = ImGuiPopupFlags_MouseButtonLeft         ,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGuiMouseButton_Left)
	MouseButtonRight        = ImGuiPopupFlags_MouseButtonRight        ,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGuiMouseButton_Right)
	MouseButtonMiddle       = ImGuiPopupFlags_MouseButtonMiddle       ,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGuiMouseButton_Middle)
	MouseButtonMask_        = ImGuiPopupFlags_MouseButtonMask_        ,
	MouseButtonDefault_     = ImGuiPopupFlags_MouseButtonDefault_     ,
	NoReopen                = ImGuiPopupFlags_NoReopen                ,   // For OpenPopup*(), BeginPopupContext*(): don't reopen same popup if already open (won't reposition, won't reinitialize navigation)
	NoOpenOverExistingPopup = ImGuiPopupFlags_NoOpenOverExistingPopup ,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
	NoOpenOverItems         = ImGuiPopupFlags_NoOpenOverItems         ,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
	AnyPopupId              = ImGuiPopupFlags_AnyPopupId              ,  // For IsPopupOpen(): ignore the ImGuiID parameter and test for any popup.
	AnyPopupLevel           = ImGuiPopupFlags_AnyPopupLevel           ,  // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
	AnyPopup                = ImGuiPopupFlags_AnyPopup                ,
};
ENUM_CLASS_FLAGS(EImGuiPopupFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTableFlags : int32
{
    // Features
    None                       = ImGuiTableFlags_None                       ,
    Resizable                  = ImGuiTableFlags_Resizable                  ,   // Enable resizing columns.
    Reorderable                = ImGuiTableFlags_Reorderable                ,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
    Hideable                   = ImGuiTableFlags_Hideable                   ,   // Enable hiding/disabling columns in context menu.
    Sortable                   = ImGuiTableFlags_Sortable                   ,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see ImGuiTableFlags_SortMulti and ImGuiTableFlags_SortTristate.
    NoSavedSettings            = ImGuiTableFlags_NoSavedSettings            ,   // Disable persisting columns order, width and sort settings in the .ini file.
    ContextMenuInBody          = ImGuiTableFlags_ContextMenuInBody          ,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
    // Decorations
    RowBg                      = ImGuiTableFlags_RowBg                      ,   // Set each RowBg color with ImGuiCol_TableRowBg or ImGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)
    BordersInnerH              = ImGuiTableFlags_BordersInnerH              ,   // Draw horizontal borders between rows.
    BordersOuterH              = ImGuiTableFlags_BordersOuterH              ,   // Draw horizontal borders at the top and bottom.
    BordersInnerV              = ImGuiTableFlags_BordersInnerV              ,   // Draw vertical borders between columns.
    BordersOuterV              = ImGuiTableFlags_BordersOuterV              ,  // Draw vertical borders on the left and right sides.
    BordersH                   = ImGuiTableFlags_BordersH                   , // Draw horizontal borders.
    BordersV                   = ImGuiTableFlags_BordersV                   , // Draw vertical borders.
    BordersInner               = ImGuiTableFlags_BordersInner               , // Draw inner borders.
    BordersOuter               = ImGuiTableFlags_BordersOuter               , // Draw outer borders.
    Borders                    = ImGuiTableFlags_Borders                    ,   // Draw all borders.
    NoBordersInBody            = ImGuiTableFlags_NoBordersInBody            ,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
    NoBordersInBodyUntilResize = ImGuiTableFlags_NoBordersInBodyUntilResize ,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
    // Sizing Policy (read above for defaults)
    SizingFixedFit             = ImGuiTableFlags_SizingFixedFit             ,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    SizingFixedSame            = ImGuiTableFlags_SizingFixedSame            ,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGuiTableFlags_NoKeepColumnsVisible.
    SizingStretchProp          = ImGuiTableFlags_SizingStretchProp          ,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    SizingStretchSame          = ImGuiTableFlags_SizingStretchSame          ,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
    // Sizing Extra Options
    NoHostExtendX              = ImGuiTableFlags_NoHostExtendX              ,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    NoHostExtendY              = ImGuiTableFlags_NoHostExtendY              ,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    NoKeepColumnsVisible       = ImGuiTableFlags_NoKeepColumnsVisible       ,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    PreciseWidths              = ImGuiTableFlags_PreciseWidths              ,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
    // Clipping
    NoClip                     = ImGuiTableFlags_NoClip                     ,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
    // Padding
    PadOuterX                  = ImGuiTableFlags_PadOuterX                  ,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
    NoPadOuterX                = ImGuiTableFlags_NoPadOuterX                ,  // Default if BordersOuterV is off. Disable outermost padding.
    NoPadInnerX                = ImGuiTableFlags_NoPadInnerX                ,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
    // Scrolling
    ScrollX                    = ImGuiTableFlags_ScrollX                    ,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
    ScrollY                    = ImGuiTableFlags_ScrollY                    ,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
    // Sorting
    SortMulti                  = ImGuiTableFlags_SortMulti                  ,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    SortTristate               = ImGuiTableFlags_SortTristate               ,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).
    // Miscellaneous
    HighlightHoveredColumn     = ImGuiTableFlags_HighlightHoveredColumn     ,  // Highlight column headers when hovered (may evolve into a fuller highlight)
};
ENUM_CLASS_FLAGS(EImGuiTableFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTableRowFlags : int32
{
	None    = ImGuiTableRowFlags_None                     ,
	Headers = ImGuiTableRowFlags_Headers                  ,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};
ENUM_CLASS_FLAGS(EImGuiTableRowFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTableColumnFlags : int32
{
    // Input configuration flags
    None                 = ImGuiTableColumnFlags_None                  ,
    Disabled             = ImGuiTableColumnFlags_Disabled              ,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    DefaultHide          = ImGuiTableColumnFlags_DefaultHide           ,   // Default as a hidden/disabled column.
    DefaultSort          = ImGuiTableColumnFlags_DefaultSort           ,   // Default as a sorting column.
    WidthStretch         = ImGuiTableColumnFlags_WidthStretch          ,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    WidthFixed           = ImGuiTableColumnFlags_WidthFixed            ,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    NoResize             = ImGuiTableColumnFlags_NoResize              ,   // Disable manual resizing.
    NoReorder            = ImGuiTableColumnFlags_NoReorder             ,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    NoHide               = ImGuiTableColumnFlags_NoHide                ,   // Disable ability to hide/disable this column.
    NoClip               = ImGuiTableColumnFlags_NoClip                ,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    NoSort               = ImGuiTableColumnFlags_NoSort                ,   // Disable ability to sort on this field (even if ImGuiTableFlags_Sortable is set on the table).
    NoSortAscending      = ImGuiTableColumnFlags_NoSortAscending       ,  // Disable ability to sort in the ascending direction.
    NoSortDescending     = ImGuiTableColumnFlags_NoSortDescending      ,  // Disable ability to sort in the descending direction.
    NoHeaderLabel        = ImGuiTableColumnFlags_NoHeaderLabel         ,  // TableHeadersRow() will not submit horizontal label for this column. Convenient for some small columns. Name will still appear in context menu or in angled headers.
    NoHeaderWidth        = ImGuiTableColumnFlags_NoHeaderWidth         ,  // Disable header text width contribution to automatic column width.
    PreferSortAscending  = ImGuiTableColumnFlags_PreferSortAscending   ,  // Make the initial sort direction Ascending when first sorting on this column (default).
    PreferSortDescending = ImGuiTableColumnFlags_PreferSortDescending  ,  // Make the initial sort direction Descending when first sorting on this column.
    IndentEnable         = ImGuiTableColumnFlags_IndentEnable          ,  // Use current Indent value when entering cell (default for column 0).
    IndentDisable        = ImGuiTableColumnFlags_IndentDisable         ,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.
    AngledHeader         = ImGuiTableColumnFlags_AngledHeader          ,  // TableHeadersRow() will submit an angled header row for this column. Note this will add an extra row.

    // Output status flags, read-only via TableGetColumnFlags()
    IsEnabled            = ImGuiTableColumnFlags_IsEnabled             ,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    IsVisible            = ImGuiTableColumnFlags_IsVisible             ,  // Status: is visible == is enabled AND not clipped by scrolling.
    IsSorted             = ImGuiTableColumnFlags_IsSorted              ,  // Status: is currently part of the sort specs
    IsHovered            = ImGuiTableColumnFlags_IsHovered             ,  // Status: is hovered by mouse
};
ENUM_CLASS_FLAGS(EImGuiTableColumnFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTabBarFlags : int32
{
	None                         = ImGuiTabBarFlags_None                           ,
	Reorderable                  = ImGuiTabBarFlags_Reorderable                    ,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
	AutoSelectNewTabs            = ImGuiTabBarFlags_AutoSelectNewTabs              ,   // Automatically select new tabs when they appear
	TabListPopupButton           = ImGuiTabBarFlags_TabListPopupButton             ,   // Disable buttons to open the tab list popup
	NoCloseWithMiddleMouseButton = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton   ,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
	NoTabListScrollingButtons    = ImGuiTabBarFlags_NoTabListScrollingButtons      ,   // Disable scrolling buttons (apply when fitting policy is ImGuiTabBarFlags_FittingPolicyScroll)
	NoTooltip                    = ImGuiTabBarFlags_NoTooltip                      ,   // Disable tooltips when hovering a tab
	FittingPolicyResizeDown      = ImGuiTabBarFlags_FittingPolicyResizeDown        ,   // Resize tabs when they don't fit
	FittingPolicyScroll          = ImGuiTabBarFlags_FittingPolicyScroll            ,   // Add scroll buttons when tabs don't fit
};
ENUM_CLASS_FLAGS(EImGuiTabBarFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTableBgTarget : int32
{
	ImGuiTableBgTarget_None                     = 0,
	ImGuiTableBgTarget_RowBg0                   = 1,        // Set row background color 0 (generally used for background, automatically set when ImGuiTableFlags_RowBg is used)
	ImGuiTableBgTarget_RowBg1                   = 2,        // Set row background color 1 (generally used for selection marking)
	ImGuiTableBgTarget_CellBg                   = 3,        // Set cell background color (top-most color)
};
ENUM_CLASS_FLAGS(EImGuiTableBgTarget);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiTabItemFlags : int32
{
	ImGuiTabItemFlags_None                          ,
	ImGuiTabItemFlags_UnsavedDocument               ,   // Display a dot next to the title + set ImGuiTabItemFlags_NoAssumedClosure.
	ImGuiTabItemFlags_SetSelected                   ,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
	ImGuiTabItemFlags_NoCloseWithMiddleMouseButton  ,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You may handle this behavior manually on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
	ImGuiTabItemFlags_NoPushId                      ,   // Don't call PushID()/PopID() on BeginTabItem()/EndTabItem()
	ImGuiTabItemFlags_NoTooltip                     ,   // Disable tooltip for the given tab
	ImGuiTabItemFlags_NoReorder                     ,   // Disable reordering this tab or having another tab cross over this tab
	ImGuiTabItemFlags_Leading                       ,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
	ImGuiTabItemFlags_Trailing                      ,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
	ImGuiTabItemFlags_NoAssumedClosure              ,   // Tab is selected when trying to close + closure is not immediately assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
};
ENUM_CLASS_FLAGS(EImGuiTabItemFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiModKey
{
	None     = ImGuiMod_None                   ,
	Ctrl     = ImGuiMod_Ctrl                   , // Ctrl
	Shift    = ImGuiMod_Shift                  , // Shift
	Alt      = ImGuiMod_Alt                    , // Option/Menu
	Super    = ImGuiMod_Super                  , // Cmd/Super/Windows
	Shortcut = ImGuiMod_Shortcut               , // Alias for Ctrl (non-macOS) _or_ Super (macOS).
};
ENUM_CLASS_FLAGS(EImGuiModKey);

UENUM()
enum class EImGuiMouseButton : uint8
{
	Left  = ImGuiMouseButton_Left,
	Right = ImGuiMouseButton_Right,
	Middle = ImGuiMouseButton_Middle,
};

UENUM()
enum class EImGuiMouseCursor : uint8
{
	Arrow = ImGuiMouseCursor_Arrow,
	TextInput = ImGuiMouseCursor_TextInput,         // When hovering over InputText, etc.
	ResizeAll = ImGuiMouseCursor_ResizeAll,         // (Unused by Dear ImGui functions)
	ResizeNS  = ImGuiMouseCursor_ResizeNS,          // When hovering over a horizontal border
	ResizeEW  = ImGuiMouseCursor_ResizeEW,          // When hovering over a vertical border or a column
	ResizeNESW = ImGuiMouseCursor_ResizeNESW,        // When hovering over the bottom-left corner of a window
	ResizeNWSE = ImGuiMouseCursor_ResizeNWSE,        // When hovering over the bottom-right corner of a window
	Hand      = ImGuiMouseCursor_Hand,              // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
	NotAllowed = ImGuiMouseCursor_NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
};

USTRUCT(BlueprintType)
struct FImGuiId
{
	GENERATED_BODY()

	FImGuiId() = default;
	FImGuiId(uint32 Id)
		: Id(Id)
	{}
	operator uint32() const
	{
		return Id;
	}
private:
	UPROPERTY()
	uint32 Id = 0;
};

UENUM()
enum class EImGuiTextureFormat : uint8
{
	Alpha8 = UnrealImGui::ETextureFormat::Alpha8,
	Gray8 = UnrealImGui::ETextureFormat::Gray8,
	RGB8 = UnrealImGui::ETextureFormat::RGB8,
	RGBA8 = UnrealImGui::ETextureFormat::RGBA8,
};

static_assert(std::is_same_v<FVector2D::FReal, double>);

UCLASS()
class IMGUI_API UImGui : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", meta = (ImGuiScopeExit = End, ImGuiAlwaysExit, Name = "NSLOCTEXT(\"WS\", \"BP_DefaultTitle\", \"Untitle\")", DisplayName = "Window", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool Begin(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags)
	{
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", meta = (ImGuiScopeExit = End, ImGuiAlwaysExit, Name = "NSLOCTEXT(\"WS\", \"BP_DefaultTitle\", \"Untitle\")", DisplayName = "Window (Open State)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginWithOpenState(FText Name, UPARAM(Ref)bool& OpenState, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags)
	{
		return ImGui::Begin(TCHAR_TO_UTF8(*Name.ToString()), &OpenState, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Windows", BlueprintInternalUseOnly)
	static void End()
	{
		ImGui::End();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Child Windows", meta = (ImGuiScopeExit = EndChild, ImGuiAlwaysExit, DisplayName = "Child Window", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginChild(FText Name, FVector2D Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiChildFlags"))int32 ChildFlags, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 WindowFlags)
	{
		return ImGui::BeginChild(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Size }, ChildFlags, WindowFlags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Child Windows", BlueprintInternalUseOnly)
	static void EndChild()
	{
		ImGui::EndChild();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsWindowAppearing()
	{
		return ImGui::IsWindowAppearing();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsWindowCollapsed()
	{
		return ImGui::IsWindowCollapsed();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsWindowFocused(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiFocusedFlags"))int32 Flags = 0)
	{
		return ImGui::IsWindowFocused(Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsWindowHovered(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiHoveredFlags")) int32 Flags = 0)
	{
		return ImGui::IsWindowHovered(Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowDpiScale()
	{
		return ImGui::GetWindowDpiScale();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowPos()
	{
		return FVector2D{ ImGui::GetWindowPos() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowSize()
	{
		return FVector2D{ ImGui::GetWindowSize() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowWidth()
	{
		return ImGui::GetWindowWidth();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Windows Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetWindowHeight()
	{
		return ImGui::GetWindowHeight();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetNextWindowPos(const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0, const FVector2D& Pivot = FVector2D::ZeroVector)
	{
		ImGui::SetNextWindowPos(ImVec2{ Pos }, Cond, ImVec2{ Pivot });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowSize(const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetNextWindowSize(ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowContentSize(const FVector2D& Size)
	{
		ImGui::SetNextWindowSize(ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowCollapsed(bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetNextWindowCollapsed(Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowFocus()
	{
		ImGui::SetNextWindowFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowScroll(const FVector2D& Scroll)
	{
		ImGui::SetNextWindowScroll(ImVec2{ Scroll });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowBgAlpha(float Alpha)
	{
		ImGui::SetNextWindowBgAlpha(Alpha);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextWindowViewport(int32 ViewportId)
	{
		ImGui::SetNextWindowViewport(ViewportId);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowPos(const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowPos(ImVec2{ Pos }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowSize(const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowSize(ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void SetWindowCollapsed(bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowCollapsed(Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFocus()
	{
		ImGui::SetWindowFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFontScale(float Scale)
	{
		ImGui::SetWindowFontScale(Scale);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void SetWindowPosByName(FText Name, const FVector2D& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowPos(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Pos }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowSizeByName(FText Name, const FVector2D& Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowSize(TCHAR_TO_UTF8(*Name.ToString()), ImVec2{ Size }, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowCollapsedByName(FText Name, bool Collapsed, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiCond"))int32 Cond = 0)
	{
		ImGui::SetWindowCollapsed(TCHAR_TO_UTF8(*Name.ToString()), Collapsed, Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Window manipulation", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetWindowFocusByName(FText Name)
	{
		ImGui::SetWindowFocus(TCHAR_TO_UTF8(*Name.ToString()));
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetContentRegionAvail()
	{
		return FVector2D{ ImGui::GetContentRegionAvail() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetContentRegionMax()
	{
		return FVector2D{ ImGui::GetContentRegionMax() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowContentRegionMin()
	{
		return FVector2D{ ImGui::GetWindowContentRegionMin() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetWindowContentRegionMax()
	{
		return FVector2D{ ImGui::GetWindowContentRegionMax() };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollX()
	{
		return ImGui::GetScrollX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollY()
	{
		return ImGui::GetScrollY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollX(float ScrollX)
	{
		ImGui::SetScrollX(ScrollX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollY(float ScrollY)
	{
		ImGui::SetScrollX(ScrollY);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollMaxX()
	{
		return ImGui::GetScrollMaxX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetScrollMaxY()
	{
		return ImGui::GetScrollMaxY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollHereX(float CenterRatioX = 0.5f)
	{
		ImGui::SetScrollHereX(CenterRatioX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollHereY(float CenterRatioY = 0.5f)
	{
		ImGui::SetScrollHereY(CenterRatioY);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollFromPosX(float LocalX, float CenterRatioX = 0.5f)
	{
		ImGui::SetScrollFromPosX(LocalX, CenterRatioX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Content region", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetScrollFromPosY(float LocalY, float CenterRatioY = 0.5f)
	{
		ImGui::SetScrollFromPosY(LocalY, CenterRatioY);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleColor, DisplayName = "Style Color"), BlueprintInternalUseOnly)
	static void PushStyleColor(EImGuiCol ColorType, const FColor& Color)
	{
		ImGui::PushStyleColor((ImGuiCol)ColorType, IM_COL32(Color.R, Color.G, Color.B, Color.A));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopStyleColor(int32 Count = 1)
	{
		ImGui::PopStyleColor(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleVar, DisplayName = "Style Var (float)"), BlueprintInternalUseOnly)
	static void PushStyleVarFloat(EImGuiStyleVar StyleVar, float Value)
	{
		ImGui::PushStyleVar((ImGuiStyleVar)StyleVar, Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopStyleVar, DisplayName = "Style Var (Vector2)"), BlueprintInternalUseOnly)
	static void PushStyleVarVector2(EImGuiStyleVar StyleVar, const FVector2D& Value)
	{
		ImGui::PushStyleVar((ImGuiStyleVar)StyleVar, ImVec2{ Value });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopStyleVar(int32 Count = 1)
	{
		ImGui::PopStyleVar(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopTabStop, DisplayName = "Tab Stop"), BlueprintInternalUseOnly)
	static void PushTabStop(bool TabStop)
	{
		ImGui::PushTabStop(TabStop);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopTabStop()
	{
		ImGui::PopTabStop();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopButtonRepeat, DisplayName = "Button Repeat"), BlueprintInternalUseOnly)
	static void PushButtonRepeat(bool Repeat)
	{
		ImGui::PushButtonRepeat(Repeat);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopButtonRepeat()
	{
		ImGui::PopButtonRepeat();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopItemWidth, DisplayName = "Item Width"), BlueprintInternalUseOnly)
	static void PushItemWidth(float ItemWidth)
	{
		ImGui::PushItemWidth(ItemWidth);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopItemWidth()
	{
		ImGui::PopItemWidth();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemWidth(float ItemWidth)
	{
		ImGui::SetNextItemWidth(ItemWidth);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Parameters stacks", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float CalcItemWidth()
	{
		return ImGui::CalcItemWidth();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", meta=(ImGuiScopeExit = PopTextWrapPos, DisplayName = "Text Wrap Pos"), BlueprintInternalUseOnly)
	static void PushTextWrapPos(float WrapLocalPosX = 0.0f)
	{
		ImGui::PushTextWrapPos(WrapLocalPosX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Parameters stacks", BlueprintInternalUseOnly)
	static void PopTextWrapPos()
	{
		ImGui::PopTextWrapPos();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFontSize()
	{
		return ImGui::GetFontSize();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetFontTexUvWhitePixel()
	{
		return FVector2D{ ImGui::GetFontTexUvWhitePixel() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Style read access", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FColor GetStyleColor(EImGuiCol ColorType, float AlphaMul = 1.0f)
	{
		const ImU32 Color{ ImGui::GetColorU32((ImGuiCol)ColorType, AlphaMul) };
		return FColor{ uint8(Color >> IM_COL32_R_SHIFT), uint8(Color >> IM_COL32_G_SHIFT), uint8(Color >> IM_COL32_B_SHIFT), uint8(Color >> IM_COL32_A_SHIFT) };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorScreenPos()
	{
		return FVector2D{ ImGui::GetCursorScreenPos() };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorScreenPos(const FVector2D& Pos)
	{
		ImGui::SetCursorScreenPos(ImVec2{ Pos });
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorPos()
	{
		return FVector2D{ ImGui::GetCursorPos() };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetCursorPosX()
	{
		return ImGui::GetCursorPosX();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetCursorPosY()
	{
		return ImGui::GetCursorPosY();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPos(const FVector2D& LocalPos)
	{
		ImGui::SetCursorPos(ImVec2{ LocalPos });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPosX(float LocalX)
	{
		ImGui::SetCursorPosX(LocalX);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetCursorPosY(float LocalY)
	{
		ImGui::SetCursorPosY(LocalY);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Cursor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetCursorStartPos()
	{
		return FVector2D{ ImGui::GetCursorStartPos() };
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Separator()
	{
		ImGui::Separator();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void SameLine(float OffsetFromStartX = 0.f, float Spacing = -1.f)
	{
		ImGui::SameLine(OffsetFromStartX, Spacing);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void NewLine()
	{
		ImGui::NewLine();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Spacing()
	{
		ImGui::Spacing();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Dummy(const FVector2D& Size)
	{
		ImGui::Dummy(ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void Indent(float IndentW = 0.0f)
	{
		ImGui::Indent(IndentW);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Unindent(float IndentW = 0.0f)
	{
		ImGui::Unindent(IndentW);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiScopeExit = PopTextWrapPos, DisplayName = "Group"), BlueprintInternalUseOnly)
	static void BeginGroup()
	{
		ImGui::BeginGroup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", BlueprintInternalUseOnly)
	static void EndGroup()
	{
		ImGui::EndGroup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void AlignTextToFramePadding()
	{
		ImGui::AlignTextToFramePadding();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTextLineHeight()
	{
		return ImGui::GetTextLineHeight();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTextLineHeightWithSpacing()
	{
		return ImGui::GetTextLineHeightWithSpacing();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFrameHeight()
	{
		return ImGui::GetFrameHeight();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Layout", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetFrameHeightWithSpacing()
	{
		return ImGui::GetFrameHeightWithSpacing();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiScopeExit = PopID, DisplayName = "Set Id"), BlueprintInternalUseOnly)
	static void PushID(FText Name)
	{
		return ImGui::PushID(TCHAR_TO_UTF8(*Name.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiScopeExit = PopID, DisplayName = "Set Id (Integer)"), BlueprintInternalUseOnly)
	static void PushIDByInteger(int32 Integer)
	{
		return ImGui::PushID(Integer);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", BlueprintInternalUseOnly)
	static void PopID()
	{
		ImGui::PopID();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|ID stack", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiId GetID(FText Name)
	{
		return ImGui::GetID(TCHAR_TO_UTF8(*Name.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Text(FText Text)
	{
		ImGui::Text(TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextColored(FLinearColor Color, FText Text)
	{
		ImGui::TextColored(ImVec4{ Color }, TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextDisabled(FText Text)
	{
		ImGui::TextDisabled(TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TextWrapped(FText Text)
	{
		ImGui::TextWrapped(TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void LabelText(FText Label, FText Text)
	{
		ImGui::LabelText(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void BulletText(FText Text)
	{
		ImGui::BulletText(TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Text", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SeparatorText(FText Text)
	{
		ImGui::SeparatorText(TCHAR_TO_UTF8(*Text.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool Button(FText Label, FVector2D Size)
	{
		return ImGui::Button(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool SmallButton(FText Label)
	{
		return ImGui::SmallButton(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool ArrowButton(FText Label, EImGuiDir Dir)
	{
		return ImGui::ArrowButton(TCHAR_TO_UTF8(*Label.ToString()), (ImGuiDir)Dir);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool InvisibleButton(FText Label, FVector2D Size)
	{
		return ImGui::InvisibleButton(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool CheckBox(FText Label, UPARAM(Ref)bool& Value)
	{
		return ImGui::Checkbox(TCHAR_TO_UTF8(*Label.ToString()), &Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool CheckboxFlags(FText Label, UPARAM(Ref)int32& Flags, int32 FlagsValue)
	{
		return ImGui::CheckboxFlags(TCHAR_TO_UTF8(*Label.ToString()), &Flags, FlagsValue);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool RadioButton(FText Label, bool bActive)
	{
		return ImGui::RadioButton(TCHAR_TO_UTF8(*Label.ToString()), bActive);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiFunction, SizeArg = "(X=-1, Y=0)", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void ProgressBar(float Fraction, FVector2D SizeArg, FText Overlay)
	{
		ImGui::ProgressBar(Fraction, ImVec2{ SizeArg }, TCHAR_TO_UTF8(*Overlay.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Main", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Bullet()
	{
		ImGui::Bullet();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Images", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	void Image(UTexture* Texture, FVector2D ImageSize, EImGuiTextureFormat Format = EImGuiTextureFormat::RGB8, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		const UnrealImGui::FImGuiTextureHandle Handle = UnrealImGui::FindOrAddTexture((UnrealImGui::ETextureFormat)Format, Texture);
		ImGui::Image(Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Images", meta = (ImGuiTrigger, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	bool ImageButton(FText Label, UTexture* Texture, FVector2D ImageSize, EImGuiTextureFormat Format = EImGuiTextureFormat::RGB8, FVector2D UV0 = FVector2D::ZeroVector, FVector2D UV1 = FVector2D::UnitVector, FLinearColor TintColor = FLinearColor::White, FLinearColor BorderColor = FLinearColor::Transparent)
	{
		const UnrealImGui::FImGuiTextureHandle Handle = UnrealImGui::FindOrAddTexture((UnrealImGui::ETextureFormat)Format, Texture);
		return ImGui::ImageButton(TCHAR_TO_UTF8(*Label.ToString()), Handle, ImVec2{ ImageSize }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, ImVec4{ BorderColor });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", meta = (ImGuiScopeExit = EndCombo, DisplayName = "Combo (Scope)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginCombo(FText Label, FText PreviewValue, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiComboFlags"))int32 Flags = 0)
	{
		return ImGui::BeginCombo(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*PreviewValue.ToString()), (ImGuiComboFlags)Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", BlueprintInternalUseOnly)
	static void EndCombo()
	{
		ImGui::EndCombo();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Combo Box (Dropdown)", meta = (ImGuiTrigger, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	static bool Combo(FText Label, UPARAM(Ref)int32& CurrentItem, const TArray<FText>& Items, int32 PopupMaxHeightInItems = -1)
	{
		struct FUserData
		{
			const TArray<FText>& Items;
		};
		FUserData UserData{ Items };
		return ImGui::Combo(TCHAR_TO_UTF8(*Label.ToString()), &CurrentItem, [](void* UserData, int32 Idx)
		{
			const TArray<FText>& Items = static_cast<FUserData*>(UserData)->Items;
			const ANSICHAR* String = TCHAR_TO_UTF8(*Items[Idx].ToString());
			return String;
		}, &UserData, Items.Num(), PopupMaxHeightInItems);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat(FText Label, UPARAM(Ref)float& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat2(FText Label, UPARAM(Ref)FVector2f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat3(FText Label, UPARAM(Ref)FVector3f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloat4(FText Label, UPARAM(Ref)FVector4f& Value, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragFloatRange2(FText Label, UPARAM(Ref)float& CurrentMin, UPARAM(Ref)float& CurrentMax, float Speed = 1.0f, float Min = 0.0f, float Max = 0.0f, FString FormatMin = TEXT("%.3f"), FString FormatMax = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragFloatRange2(TCHAR_TO_UTF8(*Label.ToString()), &CurrentMin, &CurrentMax, Speed, Min, Max, TCHAR_TO_UTF8(*FormatMin), TCHAR_TO_UTF8(*FormatMax), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt(FText Label, UPARAM(Ref)int32& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt2(FText Label, UPARAM(Ref)FIntPoint& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt3(FText Label, UPARAM(Ref)FIntVector& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt4(FText Label, UPARAM(Ref)FIntVector4& Value, float Speed = 1.0f, int32 Min = 0, int32 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Speed, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragIntRange2(FText Label, UPARAM(Ref)int32& CurrentMin, UPARAM(Ref)int32& CurrentMax, float Speed = 1.0f, int Min = 0, int Max = 0, FString FormatMin = TEXT("%d"), FString FormatMax = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragIntRange2(TCHAR_TO_UTF8(*Label.ToString()), &CurrentMin, &CurrentMax, Speed, Min, Max, TCHAR_TO_UTF8(*FormatMin), TCHAR_TO_UTF8(*FormatMax), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragDouble(FText Label, UPARAM(Ref)double& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector2(FText Label, UPARAM(Ref)FVector2D& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 2, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector3(FText Label, UPARAM(Ref)FVector& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 3, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragVector4(FText Label, UPARAM(Ref)FVector4& Value, float Speed = 1.0f, double Min = 0.0f, double Max = 0.0f, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 4, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragByte(FText Label, UPARAM(Ref)uint8& Value, float Speed = 1.0f, uint8 Min = 0, uint8 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool DragInt64(FText Label, UPARAM(Ref)int64& Value, float Speed = 1.0f, int64 Min = 0, int64 Max = 0, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::DragScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, Speed, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat(FText Label, UPARAM(Ref)float& Value, float Min, float Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat2(FText Label, UPARAM(Ref)FVector2f& Value, float Min, float Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat3(FText Label, UPARAM(Ref)FVector3f& Value, float Min, float Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderFloat4(FText Label, UPARAM(Ref)FVector4f& Value, float Min, float Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderAngle(FText Label, UPARAM(Ref)float& Rad, float DegreesMin = -360.0f, float DegreesMax = +360.0f, FString Format = TEXT("%.0f deg"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderAngle(TCHAR_TO_UTF8(*Label.ToString()), &Rad, DegreesMin, DegreesMax, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt(FText Label, UPARAM(Ref)int32& Value, int32 Min, int32 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt2(FText Label, UPARAM(Ref)FIntPoint& Value, int32 Min, int32 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt3(FText Label, UPARAM(Ref)FIntVector& Value, int32 Min, int32 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt4(FText Label, UPARAM(Ref)FIntVector4& Value, int32 Min, int32 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderDouble(FText Label, UPARAM(Ref)double& Value, double Min, double Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector2(FText Label, UPARAM(Ref)FVector2D& Value, double Min, double Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 2, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector3(FText Label, UPARAM(Ref)FVector& Value, double Min, double Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 3, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderVector4(FText Label, UPARAM(Ref)FVector4& Value, double Min, double Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value, 4, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderByte(FText Label, UPARAM(Ref)uint8& Value, uint8 Min, uint8 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool SliderInt64(FText Label, UPARAM(Ref)int64& Value, int64 Min, int64 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::SliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 4), BlueprintInternalUseOnly)
	static bool VSliderFloat(FText Label, FVector2D Size, UPARAM(Ref)float& Value, float Min, float Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::VSliderFloat(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderInt(FText Label, FVector2D Size, UPARAM(Ref)int32& Value, int32 Min, int32 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::VSliderInt(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, &Value, Min, Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Regular Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderDouble(FText Label, FVector2D Size, UPARAM(Ref)double& Value, double Min, double Max, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_Double, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderByte(FText Label, FVector2D Size, UPARAM(Ref)uint8& Value, uint8 Min, uint8 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_U8, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Drag Sliders", meta = (ImGuiTrigger, AdvancedDisplay = 5), BlueprintInternalUseOnly)
	static bool VSliderInt64(FText Label, FVector2D Size, UPARAM(Ref)int64& Value, int64 Min, int64 Max, FString Format = TEXT("%d"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSliderFlags"))int32 Flags = 0)
	{
		return ImGui::VSliderScalar(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size }, ImGuiDataType_S64, &Value, &Min, &Max, TCHAR_TO_UTF8(*Format), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputText(FText Label, UPARAM(Ref)FString& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputText(TCHAR_TO_UTF8(*Label.ToString()), UTF8String, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputTextMultiline(FText Label, UPARAM(Ref)FString& Value, FVector2f Size, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputTextMultiline(TCHAR_TO_UTF8(*Label.ToString()), UTF8String, ImVec2{ Size }, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputTextWithHint(FText Label, FText Hint, UPARAM(Ref)FString& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		UnrealImGui::FUTF8String UTF8String{ Value };
		const bool Ret = UnrealImGui::InputTextWithHint(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Hint.ToString()), UTF8String, Flags);
		if (Ret)
		{
			Value = UTF8String.ToString();
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputFloat(FText Label, UPARAM(Ref)float& Value, float Step = 0.f, float StepFast = 0.f, FString Format = TEXT("%.3"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputFloat(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector2f(FText Label, UPARAM(Ref)FVector2f& Value, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputFloat2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector3f(FText Label, UPARAM(Ref)FVector3f& Value, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputFloat3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputFloat4(FText Label, UPARAM(Ref)FVector4f& Value, FString Format = TEXT("%.3f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputFloat4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt(FText Label, UPARAM(Ref)int32& Value, int32 Step = 1, int32 StepFast = 100, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputInt(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt2(FText Label, UPARAM(Ref)FIntPoint& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputInt2(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt3(FText Label, UPARAM(Ref)FIntVector& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputInt3(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt4(FText Label, UPARAM(Ref)FIntVector4& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputInt4(TCHAR_TO_UTF8(*Label.ToString()), &Value.X, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputDouble(FText Label, UPARAM(Ref)double& Value, double Step = 0.f, double StepFast = 0.f, FString Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputDouble(TCHAR_TO_UTF8(*Label.ToString()), &Value, Step, StepFast, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector2(FText Label, UPARAM(Ref)FVector2D& Value, double Step = 0.f, double StepFast = 0.f, FString Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 2, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector3(FText Label, UPARAM(Ref)FVector& Value, double Step = 0.f, double StepFast = 0.f, FString Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 3, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputVector4(FText Label, UPARAM(Ref)FVector4& Value, double Step = 0.f, double StepFast = 0.f, FString Format = TEXT("%.6f"), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputScalarN(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_Double, &Value.X, 4, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, TCHAR_TO_UTF8(*Format), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputByte(FText Label, UPARAM(Ref)uint8& Value, uint8 Step = 1, uint8 StepFast = 10, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_U8, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Input", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool InputInt64(FText Label, UPARAM(Ref)int64& Value, int64 Step = 1, int64 StepFast = 100, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiInputTextFlags"))int32 Flags = 0)
	{
		return ImGui::InputScalar(TCHAR_TO_UTF8(*Label.ToString()), ImGuiDataType_S64, &Value, Step > 0 ? &Step : nullptr, StepFast > 0 ? &StepFast : nullptr, nullptr, Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool LinearColorEdit(FText Label, UPARAM(Ref)FLinearColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		return ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &Value.R, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorEdit(FText Label, UPARAM(Ref)FColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		FLinearColor V = Value;
		const bool Ret = ImGui::ColorEdit4(TCHAR_TO_UTF8(*Label.ToString()), &V.R, Flags);
		if (Ret)
		{
			Value = V.ToFColor(false);
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool LinearColorPicker(FText Label, UPARAM(Ref)FLinearColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		return ImGui::ColorPicker4(TCHAR_TO_UTF8(*Label.ToString()), &Value.R, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorPicker(FText Label, UPARAM(Ref)FColor& Value, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0)
	{
		FLinearColor V = Value;
		const bool Ret = ImGui::ColorPicker4(TCHAR_TO_UTF8(*Label.ToString()), &V.R, Flags);
		if (Ret)
		{
			Value = V.ToFColor(false);
		}
		return Ret;
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool ColorButton(FText Label, FLinearColor Color, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags = 0, FVector2D Size = FVector2D::ZeroVector)
	{
		return ImGui::ColorButton(TCHAR_TO_UTF8(*Label.ToString()), ImVec4{ Color }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Color Editor", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColorEditOptions(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiColorEditFlags"))int32 Flags)
	{
		ImGui::SetColorEditOptions(Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TreeNode(FText Label)
	{
		return ImGui::TreeNode(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TreeNodeEx(FText Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		return ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiScopeExit = TreePop, DisplayName = "Tree Id Scope"), BlueprintInternalUseOnly)
	static void TreePush(FText Label)
	{
		ImGui::TreePush(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", BlueprintInternalUseOnly)
	static void TreePop()
	{
		ImGui::TreePop();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static float GetTreeNodeToLabelSpacing()
	{
		return ImGui::GetTreeNodeToLabelSpacing();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool CollapsingHeaderRegular(FText Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		return ImGui::CollapsingHeader(TCHAR_TO_UTF8(*Label.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool CollapsingHeader(FText Label, UPARAM(Ref)bool& Visible, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTreeNodeFlags"))int32 Flags = 0)
	{
		return ImGui::CollapsingHeader(TCHAR_TO_UTF8(*Label.ToString()), &Visible, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Trees", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemOpen(bool IsOpen, EImGuiCond Cond = EImGuiCond::None)
	{
		ImGui::SetNextItemOpen(IsOpen, (int32)Cond);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Selectables", meta = (ImGuiTrigger, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool Selectable(FText Label, bool bActive, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiSelectableFlags"))int32 Flags = 0, FVector2D Size = FVector2D::ZeroVector)
	{
		return ImGui::Selectable(TCHAR_TO_UTF8(*Label.ToString()), bActive, Flags, ImVec2{ Size });
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", meta = (ImGuiScopeExit = EndListBox, DisplayName = "List Box Scope"), BlueprintInternalUseOnly)
	static bool BeginListBox(FText Label, const FVector2D& Size = FVector2D::ZeroVector)
	{
		return ImGui::BeginListBox(TCHAR_TO_UTF8(*Label.ToString()), ImVec2{ Size });
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", BlueprintInternalUseOnly)
	static void EndListBox()
	{
		ImGui::EndListBox();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|List Boxes", meta = (ImGuiFunction, AdvancedDisplay = 3), BlueprintInternalUseOnly)
	static bool ListBox(FText Label, UPARAM(Ref)int32& CurrentItem, const TArray<FText>& Items, int HeightInItems = -1)
	{
		struct FUserData
		{
			const TArray<FText>& Items;
		};
		FUserData UserData{ Items };
		return ImGui::ListBox(TCHAR_TO_UTF8(*Label.ToString()), &CurrentItem, [](void* UserData, int32 Idx)->const char*
		{
			const TArray<FText>& Items = static_cast<FUserData*>(UserData)->Items;
			const ANSICHAR* String = TCHAR_TO_UTF8(*Items[Idx].ToString());
			return String;
		}, &UserData, Items.Num(), HeightInItems);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueBool(FText Prefix, bool Value)
	{
		ImGui::Value(TCHAR_TO_UTF8(*Prefix.ToString()), Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueInt(FText Prefix, int32 Value)
	{
		ImGui::Value(TCHAR_TO_UTF8(*Prefix.ToString()), Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Value", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void ValueFloat(FText Prefix, float Value, FString Format = TEXT(""))
	{
		ImGui::Value(TCHAR_TO_UTF8(*Prefix.ToString()), Value, Format.Len() > 0 ? TCHAR_TO_UTF8(*Format) : nullptr);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta=(ImGuiScopeExit = EndMenuBar, DisplayName = "Menu Bar"), BlueprintInternalUseOnly)
	static bool BeginMenuBar()
	{
		return ImGui::BeginMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", BlueprintInternalUseOnly)
	static void EndMenuBar()
	{
		ImGui::EndMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta = (ImGuiScopeExit = EndMainMenuBar, DisplayName = "Main Menu Bar"), BlueprintInternalUseOnly)
	static bool BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", BlueprintInternalUseOnly)
	static void EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta=(ImGuiScopeExit = EndMenu, DisplayName = "Menu", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginMenu(FText Label, bool Enabled = true)
	{
		return ImGui::BeginMenu(TCHAR_TO_UTF8(*Label.ToString()), Enabled);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Menus", BlueprintInternalUseOnly)
	static void EndMenu()
	{
		return ImGui::EndMenu();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Menus", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool MenuItem(FText Label, FString Shortcut, bool Selected, bool Enabled = true)
	{
		return ImGui::MenuItem(TCHAR_TO_UTF8(*Label.ToString()), TCHAR_TO_UTF8(*Shortcut), Selected, Enabled);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", meta=(ImGuiScopeExit = EndTooltip, DisplayName = "Tooltip"), BlueprintInternalUseOnly)
	static bool BeginTooltip()
	{
		return ImGui::BeginTooltip();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", BlueprintInternalUseOnly)
	static void EndTooltip()
	{
		return ImGui::EndTooltip();
	}
	UFUNCTION(Blueprintable, Category="ImGui|Tooltip", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetTooltip(FText Text)
	{
		ImGui::SetTooltip(TCHAR_TO_UTF8(*Text.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tooltip", meta=(ImGuiScopeExit = EndTooltip, DisplayName = "Tooltip"), BlueprintInternalUseOnly)
	static bool BeginItemTooltip()
	{
		return ImGui::BeginItemTooltip();
	}
	UFUNCTION(Blueprintable, Category="ImGui|Tooltip", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetItemTooltip(FText Text)
	{
		ImGui::SetItemTooltip(TCHAR_TO_UTF8(*Text.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginPopup(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		return ImGui::BeginPopup(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup Modal", AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool BeginPopupModal(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		return ImGui::BeginPopupModal(TCHAR_TO_UTF8(*Name.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta=(ImGuiScopeExit = EndPopup, DisplayName = "Popup Modal (Open State)", AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static bool BeginPopupModalOpenState(FText Name, bool& Open, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiWindowFlags"))int32 Flags = 0)
	{
		return ImGui::BeginPopupModal(TCHAR_TO_UTF8(*Name.ToString()), &Open, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", BlueprintInternalUseOnly)
	static void EndPopup()
	{
		ImGui::EndPopup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void OpenPopup(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 0)
	{
		ImGui::OpenPopup(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static void OpenPopupOnItemClick(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		ImGui::OpenPopupOnItemClick(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void CloseCurrentPopup()
	{
		ImGui::CloseCurrentPopup();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextItem"), BlueprintInternalUseOnly)
	static bool BeginPopupContextItem(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		return ImGui::BeginPopupContextItem(Name.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextWindow"), BlueprintInternalUseOnly)
	static bool BeginPopupContextWindow(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		return ImGui::BeginPopupContextWindow(Name.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Popups", meta = (ImGuiScopeExit = EndPopup, AdvancedDisplay = 0, DisplayName = "PopupContextVoid"), BlueprintInternalUseOnly)
	static bool BeginPopupContextVoid(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 1)
	{
		return ImGui::BeginPopupContextVoid(Name.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Popups", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsPopupOpen(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiPopupFlags"))int32 Flags = 0)
	{
		return ImGui::IsPopupOpen(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiScopeExit = EndTable, AdvancedDisplay = 2, DisplayName = "Table"), BlueprintInternalUseOnly)
	static bool BeginTable(FText Name, int32 Column, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableFlags"))int32 Flags = 0, FVector2D OuterSize = FVector2D::ZeroVector, float InnerWidth = 0.0f)
	{
		return ImGui::BeginTable(TCHAR_TO_UTF8(*Name.ToString()), Column, Flags, ImVec2{ OuterSize }, InnerWidth);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", BlueprintInternalUseOnly)
	static void EndTable()
	{
		return ImGui::EndTable();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void TableNextRow(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableRowFlags"))int32 Flags = 0, float MinRowHeight = 0.0f)
	{
		ImGui::TableNextRow(Flags, MinRowHeight);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TableNextColumn()
	{
		return ImGui::TableNextColumn();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool TableSetColumnIndex(int32 ColumnN)
	{
		return ImGui::TableSetColumnIndex(ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 1, Flag = 0, InitWidthOrWeight = 0, UserId = "(Id=0)"), BlueprintInternalUseOnly)
	static void TableSetupColumn(FText Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableColumnFlags"))int32 Flags, float InitWidthOrWeight, FImGuiId UserId)
	{
		ImGui::TableSetupColumn(TCHAR_TO_UTF8(*Label.ToString()), Flags, InitWidthOrWeight, UserId);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableSetupScrollFreeze(int32 Columns, int32 Rows)
	{
		ImGui::TableSetupScrollFreeze(Columns, Rows);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableHeader(FText Label)
	{
		ImGui::TableHeader(TCHAR_TO_UTF8(*Label.ToString()));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableHeadersRow()
	{
		ImGui::TableHeadersRow();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableAngledHeadersRow()
	{
		ImGui::TableAngledHeadersRow();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetColumnCount()
	{
		return ImGui::TableGetColumnCount();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetColumnIndex()
	{
		return ImGui::TableGetColumnIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 TableGetRowIndex()
	{
		return ImGui::TableGetRowIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static FText TableGetColumnName(int32 ColumnN = -1)
	{
		return FText::FromString(UTF8_TO_TCHAR(ImGui::TableGetColumnName(ColumnN)));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static int32 TableGetColumnFlags(int32 ColumnN = -1)
	{
		return ImGui::TableGetColumnFlags(ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TableSetColumnEnabled(int32 ColumnN, bool Value)
	{
		ImGui::TableSetColumnEnabled(ColumnN, Value);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 2), BlueprintInternalUseOnly)
	static void TableSetBgColor(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTableBgTarget"))int32 Target, FColor Color, int32 ColumnN = -1)
	{
		ImGui::TableSetBgColor(Target, IM_COL32(Color.R, Color.G, Color.B, Color.A), ColumnN);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void Columns(int32 Count = 1, FText Id = FText::GetEmpty(), bool Border = true)
	{
		ImGui::Columns(Count, Id.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*Id.ToString()), Border);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void NextColumn()
	{
		ImGui::NextColumn();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetColumnIndex()
	{
		return ImGui::GetColumnIndex();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static float GetColumnWidth(int32 ColumnIndex = -1)
	{
		return ImGui::GetColumnWidth(ColumnIndex);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColumnWidth(int32 ColumnIndex, float Width)
	{
		ImGui::SetColumnWidth(ColumnIndex, Width);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static float GetColumnOffset(int32 ColumnIndex = -1)
	{
		return ImGui::GetColumnOffset(ColumnIndex);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetColumnOffset(int32 ColumnIndex, float OffsetX)
	{
		ImGui::SetColumnOffset(ColumnIndex, OffsetX);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Widgets|Tables", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetColumnsCount()
	{
		return ImGui::GetColumnsCount();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabBar, AdvancedDisplay = 1, DisplayName = "Tab Bar"), BlueprintInternalUseOnly)
	static bool BeginTabBar(FText Name, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabBarFlags"))int32 Flags = 0)
	{
		return ImGui::BeginTabBar(TCHAR_TO_UTF8(*Name.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Widgets|Tables", BlueprintInternalUseOnly)
	static void EndTabBar()
	{
		ImGui::EndTabBar();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabItem, AdvancedDisplay = 1, DisplayName = "Tab Item"), BlueprintInternalUseOnly)
	static bool BeginTabItem(FText Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		return ImGui::BeginTabItem(TCHAR_TO_UTF8(*Label.ToString()), nullptr, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiScopeExit = EndTabItem, AdvancedDisplay = 1, DisplayName = "Tab Item (Open State)"), BlueprintInternalUseOnly)
	static bool BeginTabItemOpenState(FText Label, bool& IsOpen, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		return ImGui::BeginTabItem(TCHAR_TO_UTF8(*Label.ToString()), &IsOpen, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", BlueprintInternalUseOnly)
	static void EndTabItem()
	{
		ImGui::EndTabItem();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiTrigger), BlueprintInternalUseOnly)
	static bool TabItemButton(FText Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiTabItemFlags"))int32 Flags = 0)
	{
		return ImGui::TabItemButton(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Tab Bars", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetTabItemClosed(FText Label)
	{
		ImGui::SetTabItemClosed(TCHAR_TO_UTF8(*Label.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Disabling", meta = (ImGuiScopeExit = EndDisabled, AdvancedDisplay = 0, DisplayName = "Disabled Scope"), BlueprintInternalUseOnly)
	static void BeginDisabled(bool Disabled = true)
	{
		ImGui::BeginDisabled(Disabled);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Disabling", BlueprintInternalUseOnly)
	static void EndDisabled()
	{
		ImGui::EndDisabled();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Clipping", meta = (ImGuiScopeExit = PopClipRect, AdvancedDisplay = 0, DisplayName = "Clip Rect Scope"), BlueprintInternalUseOnly)
	static void PushClipRect(const FVector2D& ClipRectMin, const FVector2D& ClipRectMax, bool IntersectWithCurrentClipRect)
	{
		ImGui::PushClipRect(ImVec2{ ClipRectMin }, ImVec2{ ClipRectMax }, IntersectWithCurrentClipRect);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Clipping", BlueprintInternalUseOnly)
	static void PopClipRect()
	{
		ImGui::PopClipRect();
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Focus", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetItemDefaultFocus()
	{
		ImGui::SetItemDefaultFocus();
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Focus", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
	static void SetKeyboardFocusHere(int32 Offset = 0)
	{
		ImGui::SetKeyboardFocusHere(Offset);
	}

	UFUNCTION(BlueprintCallable, Category="ImGui|Overlapping", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextItemAllowOverlap()
	{
		ImGui::SetNextItemAllowOverlap();
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemHovered()
	{
		return ImGui::IsItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemActive()
	{
		return ImGui::IsItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemFocused()
	{
		return ImGui::IsItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemClicked()
	{
		return ImGui::IsItemClicked();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemVisible()
	{
		return ImGui::IsItemVisible();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemEdited()
	{
		return ImGui::IsItemEdited();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemActivated()
	{
		return ImGui::IsItemActivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemDeactivated()
	{
		return ImGui::IsItemDeactivated();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemDeactivatedAfterEdit()
	{
		return ImGui::IsItemDeactivatedAfterEdit();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsItemToggledOpen()
	{
		return ImGui::IsItemToggledOpen();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemHovered()
	{
		return ImGui::IsAnyItemHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemActive()
	{
		return ImGui::IsAnyItemActive();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAnyItemFocused()
	{
		return ImGui::IsAnyItemFocused();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FImGuiId GetItemID()
	{
		return ImGui::GetItemID();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectMin()
	{
		return FVector2D{ ImGui::GetItemRectMin() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectMax()
	{
		return FVector2D{ ImGui::GetItemRectMax() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Item Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetItemRectSize()
	{
		return FVector2D{ ImGui::GetItemRectSize() };
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyDown(FKey Key)
	{
		return ImGui::IsKeyDown(UnrealImGui::ConvertKey(Key));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
	static bool IsKeyPressed(FKey Key, bool Repeat = true)
	{
		return ImGui::IsKeyPressed(UnrealImGui::ConvertKey(Key), Repeat);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyReleased(FKey Key)
	{
		return ImGui::IsKeyReleased(UnrealImGui::ConvertKey(Key));
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsKeyChordPressed(FKey Key, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiModKey"))int32 ModKey)
	{
		return ImGui::IsKeyChordPressed(UnrealImGui::ConvertKey(Key) | ModKey);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static int32 GetKeyPressedAmount(FKey Key, float RepeatDelay, float Rate)
	{
		return ImGui::GetKeyPressedAmount(UnrealImGui::ConvertKey(Key), RepeatDelay, Rate);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FText GetKeyName(FKey Key)
	{
		return FText::FromString(UTF8_TO_TCHAR(ImGui::GetKeyName(UnrealImGui::ConvertKey(Key))));
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextFrameWantCaptureKeyboard(bool WantCaptureKeyboard)
	{
		ImGui::SetNextFrameWantCaptureKeyboard(WantCaptureKeyboard);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsMouseDown(EImGuiMouseButton Button)
	{
		return ImGui::IsMouseDown((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseClicked(EImGuiMouseButton Button, bool Repeat = false)
	{
		return ImGui::IsMouseClicked((ImGuiMouseButton)Button, Repeat);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMouseReleased(EImGuiMouseButton Button)
	{
		return ImGui::IsMouseReleased((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMouseDoubleClicked(EImGuiMouseButton Button)
	{
		return ImGui::IsMouseDoubleClicked((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static int32 GetMouseClickedCount(EImGuiMouseButton Button)
	{
		return ImGui::GetMouseClickedCount((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseHoveringRect(FVector2D Min, FVector2D Max, bool Clip = true)
	{
		return ImGui::IsMouseHoveringRect(ImVec2{ Min }, ImVec2{ Max }, Clip);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsMousePosValid(FVector2D MousePos)
	{
		const ImVec2 Pos{ MousePos };
		return ImGui::IsMousePosValid(&Pos);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static bool IsAnyMouseDown()
	{
		return ImGui::IsAnyMouseDown();
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static FVector2D GetMousePos()
	{
		return FVector2D{ ImGui::GetMousePos() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static FVector2D GetMousePosOnOpeningCurrentPopup()
	{
		return FVector2D{ ImGui::GetMousePosOnOpeningCurrentPopup() };
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 1), BlueprintInternalUseOnly)
    static bool IsMouseDragging(EImGuiMouseButton Button, float LockThreshold = -1.0f)
	{
		return ImGui::IsMouseDragging((ImGuiMouseButton)Button, LockThreshold);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
    static FVector2D GetMouseDragDelta(EImGuiMouseButton Button = EImGuiMouseButton::Left, float LockThreshold = -1.0f)
	{
		return FVector2D{ ImGui::GetMouseDragDelta((ImGuiMouseButton)Button, LockThreshold) };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction, AdvancedDisplay = 0), BlueprintInternalUseOnly)
    static void ResetMouseDragDelta(EImGuiMouseButton Button = EImGuiMouseButton::Left)
	{
		ImGui::ResetMouseDragDelta((ImGuiMouseButton)Button);
	}
	UFUNCTION(BlueprintPure, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static EImGuiMouseCursor GetMouseCursor()
	{
		return EImGuiMouseCursor{ (uint8)ImGui::GetMouseCursor() };
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static void SetMouseCursor(EImGuiMouseCursor CursorType)
	{
		ImGui::SetMouseCursor((ImGuiMouseCursor)CursorType);
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Inputs Utilities", meta = (ImGuiFunction), BlueprintInternalUseOnly)
    static void SetNextFrameWantCaptureMouse(bool WantCaptureKeyboard)
	{
		ImGui::SetNextFrameWantCaptureMouse(WantCaptureKeyboard);
	}

	UFUNCTION(BlueprintPure, Category="ImGui|Clipboard", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FString GetClipboardText()
	{
		return UTF8_TO_TCHAR(ImGui::GetClipboardText());
	}
	UFUNCTION(BlueprintCallable, Category="ImGui|Clipboard", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetClipboardText(const FString& Text)
	{
		ImGui::SetClipboardText(TCHAR_TO_UTF8(*Text));
	}
};
