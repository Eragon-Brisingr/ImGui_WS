// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "implot.h"
#include "UnrealImPlotTypes.generated.h"

UENUM()
enum class EImAxis : uint8
{
	// horizontal axes
	X1 = ImAxis_X1,		// enabled by default
	X2 = ImAxis_X2,     // disabled by default
	X3 = ImAxis_X3,     // disabled by default
	// vertical axes
	Y1 = ImAxis_Y1,     // enabled by default
	Y2 = ImAxis_Y2,     // disabled by default
	Y3 = ImAxis_Y3,     // disabled by default

	Auto = (uint8)IMPLOT_AUTO
};
FORCEINLINE constexpr ImAxis_ operator*(EImAxis Axis) { return (ImAxis_)static_cast<int8>(Axis); }

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotFlags : int32
{
	None          = ImPlotFlags_None,       // default
	NoTitle       = ImPlotFlags_NoTitle,  // the plot title will not be displayed (titles are also hidden if preceeded by double hashes, e.g. "##MyPlot")
	NoLegend      = ImPlotFlags_NoLegend,  // the legend will not be displayed
	NoMouseText   = ImPlotFlags_NoMouseText,  // the mouse position, in plot coordinates, will not be displayed inside of the plot
	NoInputs      = ImPlotFlags_NoInputs,  // the user will not be able to interact with the plot
	NoMenus       = ImPlotFlags_NoMenus,  // the user will not be able to open context menus
	NoBoxSelect   = ImPlotFlags_NoBoxSelect,  // the user will not be able to box-select
	NoFrame       = ImPlotFlags_NoFrame,  // the ImGui frame will not be rendered
	Equal         = ImPlotFlags_Equal,  // x and y axes pairs will be constrained to have the same units/pixel
	Crosshairs    = ImPlotFlags_Crosshairs,  // the default mouse cursor will be replaced with a crosshair when hovered
	CanvasOnly    = ImPlotFlags_CanvasOnly
};
ENUM_CLASS_FLAGS(EImPlotFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotAxisFlags : int32
{
    None          = ImPlotAxisFlags_None,       // default
    NoLabel       = ImPlotAxisFlags_NoLabel,  // the axis label will not be displayed (axis labels are also hidden if the supplied string name is nullptr)
    NoGridLines   = ImPlotAxisFlags_NoGridLines,  // no grid lines will be displayed
    NoTickMarks   = ImPlotAxisFlags_NoTickMarks,  // no tick marks will be displayed
    NoTickLabels  = ImPlotAxisFlags_NoTickLabels,  // no text labels will be displayed
    NoInitialFit  = ImPlotAxisFlags_NoInitialFit,  // axis will not be initially fit to data extents on the first rendered frame
    NoMenus       = ImPlotAxisFlags_NoMenus,  // the user will not be able to open context menus with right-click
    NoSideSwitch  = ImPlotAxisFlags_NoSideSwitch,  // the user will not be able to switch the axis side by dragging it
    NoHighlight   = ImPlotAxisFlags_NoHighlight,  // the axis will not have its background highlighted when hovered or held
    Opposite      = ImPlotAxisFlags_Opposite,  // axis ticks and labels will be rendered on the conventionally opposite side (i.e, right or top)
    Foreground    = ImPlotAxisFlags_Foreground,  // grid lines will be displayed in the foreground (i.e. on top of data) instead of the background
    Invert        = ImPlotAxisFlags_Invert, // the axis will be inverted
    AutoFit       = ImPlotAxisFlags_AutoFit, // axis will be auto-fitting to data extents
    RangeFit      = ImPlotAxisFlags_RangeFit, // axis will only fit points if the point is in the visible range of the **orthogonal** axis
    PanStretch    = ImPlotAxisFlags_PanStretch, // panning in a locked or constrained state will cause the axis to stretch if possible
    LockMin       = ImPlotAxisFlags_LockMin, // the axis minimum value will be locked when panning/zooming
    LockMax       = ImPlotAxisFlags_LockMax, // the axis maximum value will be locked when panning/zooming
    Lock          = ImPlotAxisFlags_Lock,
    NoDecorations = ImPlotAxisFlags_NoDecorations,
    AuxDefault    = ImPlotAxisFlags_AuxDefault
};
ENUM_CLASS_FLAGS(EImPlotAxisFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotSubplotFlags : int32
{
	None        = ImPlotSubplotFlags_None,       // default
	NoTitle     = ImPlotSubplotFlags_NoTitle,  // the subplot title will not be displayed (titles are also hidden if preceeded by double hashes, e.g. "##MySubplot")
	NoLegend    = ImPlotSubplotFlags_NoLegend,  // the legend will not be displayed (only applicable if ImPlotSubplotFlags_ShareItems is enabled)
	NoMenus     = ImPlotSubplotFlags_NoMenus,  // the user will not be able to open context menus with right-click
	NoResize    = ImPlotSubplotFlags_NoResize,  // resize splitters between subplot cells will be not be provided
	NoAlign     = ImPlotSubplotFlags_NoAlign,  // subplot edges will not be aligned vertically or horizontally
	ShareItems  = ImPlotSubplotFlags_ShareItems,  // items across all subplots will be shared and rendered into a single legend entry
	LinkRows    = ImPlotSubplotFlags_LinkRows,  // link the y-axis limits of all plots in each row (does not apply to auxiliary axes)
	LinkCols    = ImPlotSubplotFlags_LinkCols,  // link the x-axis limits of all plots in each column (does not apply to auxiliary axes)
	LinkAllX    = ImPlotSubplotFlags_LinkAllX,  // link the x-axis limits in every plot in the subplot (does not apply to auxiliary axes)
	LinkAllY    = ImPlotSubplotFlags_LinkAllY,  // link the y-axis limits in every plot in the subplot (does not apply to auxiliary axes)
	ColMajor    = ImPlotSubplotFlags_ColMajor  // subplots are added in column major order instead of the default row major order
};
ENUM_CLASS_FLAGS(EImPlotSubplotFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotLegendFlags : int32
{
	None            = ImPlotLegendFlags_None,      // default
	NoButtons       = ImPlotLegendFlags_NoButtons, // legend icons will not function as hide/show buttons
	NoHighlightItem = ImPlotLegendFlags_NoHighlightItem, // plot items will not be highlighted when their legend entry is hovered
	NoHighlightAxis = ImPlotLegendFlags_NoHighlightAxis, // axes will not be highlighted when legend entries are hovered (only relevant if x/y-axis count > 1)
	NoMenus         = ImPlotLegendFlags_NoMenus, // the user will not be able to open context menus with right-click
	Outside         = ImPlotLegendFlags_Outside, // legend will be rendered outside of the plot area
	Horizontal      = ImPlotLegendFlags_Horizontal, // legend entries will be displayed horizontally
	Sort            = ImPlotLegendFlags_Sort, // legend entries will be displayed in alphabetical order
};
ENUM_CLASS_FLAGS(EImPlotLegendFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotMouseTextFlags : int32
{
	None        = ImPlotMouseTextFlags_None,      // default
	NoAuxAxes   = ImPlotMouseTextFlags_NoAuxAxes, // only show the mouse position for primary axes
	NoFormat    = ImPlotMouseTextFlags_NoFormat, // axes label formatters won't be used to render text
	ShowAlways  = ImPlotMouseTextFlags_ShowAlways, // always display mouse position even if plot not hovered
};
ENUM_CLASS_FLAGS(EImPlotMouseTextFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotDragToolFlags : int32
{
	None      = ImPlotDragToolFlags_None,      // default
	NoCursors = ImPlotDragToolFlags_NoCursors, // drag tools won't change cursor icons when hovered or held
	NoFit     = ImPlotDragToolFlags_NoFit, // the drag tool won't be considered for plot fits
	NoInputs  = ImPlotDragToolFlags_NoInputs, // lock the tool from user inputs
	Delayed   = ImPlotDragToolFlags_Delayed, // tool rendering will be delayed one frame; useful when applying position-constraints
};
ENUM_CLASS_FLAGS(EImPlotDragToolFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotColormapScaleFlags : int32
{
	None     = ImPlotColormapScaleFlags_None,      // default
	NoLabel  = ImPlotColormapScaleFlags_NoLabel, // the colormap axis label will not be displayed
	Opposite = ImPlotColormapScaleFlags_Opposite, // render the colormap label and tick labels on the opposite side
	Invert   = ImPlotColormapScaleFlags_Invert, // invert the colormap bar and axis scale (this only affects rendering; if you only want to reverse the scale mapping, make scale_min > scale_max)
};
ENUM_CLASS_FLAGS(EImPlotColormapScaleFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotItemFlags : int32
{
	None     = ImPlotItemFlags_None,
	NoLegend = ImPlotItemFlags_NoLegend, // the item won't have a legend entry displayed
	NoFit    = ImPlotItemFlags_NoFit, // the item won't be considered for plot fits
};
ENUM_CLASS_FLAGS(EImPlotItemFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotLineFlags : int32
{
	None        = ImPlotLineFlags_None,       // default
	Segments    = ImPlotLineFlags_Segments, // a line segment will be rendered from every two consecutive points
	Loop        = ImPlotLineFlags_Loop, // the last and first point will be connected to form a closed loop
	SkipNaN     = ImPlotLineFlags_SkipNaN, // NaNs values will be skipped instead of rendered as missing data
	NoClip      = ImPlotLineFlags_NoClip, // markers (if displayed) on the edge of a plot will not be clipped
	Shaded      = ImPlotLineFlags_Shaded, // a filled region between the line and horizontal origin will be rendered; use PlotShaded for more advanced cases
};
ENUM_CLASS_FLAGS(EImPlotLineFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotScatterFlags : int32
{
	None   = ImPlotScatterFlags_None,       // default
	NoClip = ImPlotScatterFlags_NoClip, // markers on the edge of a plot will not be clipped
};
ENUM_CLASS_FLAGS(EImPlotScatterFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotStairsFlags : int32
{
	None     = ImPlotStairsFlags_None,       // default
	PreStep  = ImPlotStairsFlags_PreStep, // the y value is continued constantly to the left from every x position, i.e. the interval (x[i-1], x[i]] has the value y[i]
	Shaded   = ImPlotStairsFlags_Shaded  // a filled region between the stairs and horizontal origin will be rendered; use PlotShaded for more advanced cases
};
ENUM_CLASS_FLAGS(EImPlotStairsFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotShadedFlags : int32
{
	None  = ImPlotShadedFlags_None // default
};
ENUM_CLASS_FLAGS(EImPlotShadedFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotBarsFlags : int32
{
	None         = ImPlotBarsFlags_None,       // default
	Horizontal   = ImPlotBarsFlags_Horizontal, // bars will be rendered horizontally on the current y-axis
};
ENUM_CLASS_FLAGS(EImPlotBarsFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotBarGroupsFlags : int32
{
	None        = ImPlotBarGroupsFlags_None,       // default
	Horizontal  = ImPlotBarGroupsFlags_Horizontal, // bar groups will be rendered horizontally on the current y-axis
	Stacked     = ImPlotBarGroupsFlags_Stacked, // items in a group will be stacked on top of each other
};
ENUM_CLASS_FLAGS(EImPlotBarGroupsFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotErrorBarsFlags : int32
{
	None       = ImPlotErrorBarsFlags_None,       // default
	Horizontal = ImPlotErrorBarsFlags_Horizontal, // error bars will be rendered horizontally on the current y-axis
};
ENUM_CLASS_FLAGS(EImPlotErrorBarsFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotStemsFlags : int32
{
	None       = ImPlotStemsFlags_None,       // default
	Horizontal = ImPlotStemsFlags_Horizontal, // stems will be rendered horizontally on the current y-axis
};
ENUM_CLASS_FLAGS(EImPlotStemsFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotInfLinesFlags : int32
{
	None       = ImPlotInfLinesFlags_None,      // default
	Horizontal = ImPlotInfLinesFlags_Horizontal // lines will be rendered horizontally on the current y-axis
};
ENUM_CLASS_FLAGS(EImPlotInfLinesFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotPieChartFlags : int32
{
	None         = ImPlotPieChartFlags_None,       // default
	Normalize    = ImPlotPieChartFlags_Normalize, // force normalization of pie chart values (i.e. always make a full circle if sum < 0)
	IgnoreHidden = ImPlotPieChartFlags_IgnoreHidden, // ignore hidden slices when drawing the pie chart (as if they were not there)
	Exploding    = ImPlotPieChartFlags_Exploding  // Explode legend-hovered slice
};
ENUM_CLASS_FLAGS(EImPlotPieChartFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotHeatmapFlags : int32
{
	None     = ImPlotHeatmapFlags_None,       // default
	ColMajor = ImPlotHeatmapFlags_ColMajor, // data will be read in column major order
};
ENUM_CLASS_FLAGS(EImPlotHeatmapFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotHistogramFlags : int32
{
	ImPlotHistogramFlags_None       = ImPlotHistogramFlags_None,       // default
	ImPlotHistogramFlags_Horizontal = ImPlotHistogramFlags_Horizontal, // histogram bars will be rendered horizontally (not supported by PlotHistogram2D)
	ImPlotHistogramFlags_Cumulative = ImPlotHistogramFlags_Cumulative, // each bin will contain its count plus the counts of all previous bins (not supported by PlotHistogram2D)
	ImPlotHistogramFlags_Density    = ImPlotHistogramFlags_Density, // counts will be normalized, i.e. the PDF will be visualized, or the CDF will be visualized if Cumulative is also set
	ImPlotHistogramFlags_NoOutliers = ImPlotHistogramFlags_NoOutliers, // exclude values outside the specifed histogram range from the count toward normalizing and cumulative counts
	ImPlotHistogramFlags_ColMajor   = ImPlotHistogramFlags_ColMajor  // data will be read in column major order (not supported by PlotHistogram)
};
ENUM_CLASS_FLAGS(EImPlotHistogramFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotDigitalFlags : int32
{
	None = ImPlotDigitalFlags_None // default
};
ENUM_CLASS_FLAGS(EImPlotDigitalFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotImageFlags : int32
{
	None = ImPlotImageFlags_None // default
};
ENUM_CLASS_FLAGS(EImPlotImageFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotTextFlags : int32
{
	None     = ImPlotTextFlags_None,       // default
	Vertical = ImPlotTextFlags_Vertical  // text will be rendered vertically
};
ENUM_CLASS_FLAGS(EImPlotTextFlags);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotDummyFlags : int32
{
	ImPlotDummyFlags_None = ImPlotDummyFlags_None // default
};
ENUM_CLASS_FLAGS(EImPlotDummyFlags);

UENUM()
enum class EImPlotCond : uint8
{
	None   = ImPlotCond_None,    // No condition (always set the variable), same as _Always
	Always = ImPlotCond_Always,  // No condition (always set the variable)
	Once   = ImPlotCond_Once,    // Set the variable once per runtime session (only the first call will succeed)
};
FORCEINLINE constexpr ImPlotCond_ operator*(EImPlotCond Cond) { return (ImPlotCond_)static_cast<int8>(Cond); }

UENUM()
enum class EImPlotCol : uint8
{
	Line = ImPlotCol_Line,          // plot line/outline color (defaults to next unused color in current colormap)
	Fill = ImPlotCol_Fill,          // plot fill color for bars (defaults to the current line color)
	MarkerOutline = ImPlotCol_MarkerOutline, // marker outline color (defaults to the current line color)
	MarkerFill = ImPlotCol_MarkerFill,    // marker fill color (defaults to the current line color)
	ErrorBar = ImPlotCol_ErrorBar,      // error bar color (defaults to ImGuiCol_Text)
	FrameBg = ImPlotCol_FrameBg,       // plot frame background color (defaults to ImGuiCol_FrameBg)
	PlotBg = ImPlotCol_PlotBg,        // plot area background color (defaults to ImGuiCol_WindowBg)
	PlotBorder = ImPlotCol_PlotBorder,    // plot area border color (defaults to ImGuiCol_Border)
	LegendBg = ImPlotCol_LegendBg,      // legend background color (defaults to ImGuiCol_PopupBg)
	LegendBorder = ImPlotCol_LegendBorder,  // legend border color (defaults to ImPlotCol_PlotBorder)
	LegendText = ImPlotCol_LegendText,    // legend text color (defaults to ImPlotCol_InlayText)
	TitleText = ImPlotCol_TitleText,     // plot title text color (defaults to ImGuiCol_Text)
	InlayText = ImPlotCol_InlayText,     // color of text appearing inside of plots (defaults to ImGuiCol_Text)
	AxisText = ImPlotCol_AxisText,      // axis label and tick lables color (defaults to ImGuiCol_Text)
	AxisGrid = ImPlotCol_AxisGrid,      // axis grid color (defaults to 25% ImPlotCol_AxisText)
	AxisTick = ImPlotCol_AxisTick,      // axis tick color (defaults to AxisGrid)
	AxisBg = ImPlotCol_AxisBg,        // background color of axis hover region (defaults to transparent)
	AxisBgHovered = ImPlotCol_AxisBgHovered, // axis hover color (defaults to ImGuiCol_ButtonHovered)
	AxisBgActive = ImPlotCol_AxisBgActive,  // axis active color (defaults to ImGuiCol_ButtonActive)
	Selection = ImPlotCol_Selection,     // box-selection color (defaults to yellow)
	Crosshairs = ImPlotCol_Crosshairs,    // crosshairs color (defaults to ImPlotCol_PlotBorder)
};
FORCEINLINE constexpr ImPlotCol_ operator*(EImPlotCol PlotCol) { return (ImPlotCol_)PlotCol; }

UENUM()
enum class EImPlotStyleVar : uint8
{
    LineWeight = ImPlotStyleVar_LineWeight,         // float,  plot item line weight in pixels
    Marker = ImPlotStyleVar_Marker,             // int,    marker specification
    MarkerSize = ImPlotStyleVar_MarkerSize,         // float,  marker size in pixels (roughly the marker's "radius")
    MarkerWeight = ImPlotStyleVar_MarkerWeight,       // float,  plot outline weight of markers in pixels
    FillAlpha = ImPlotStyleVar_FillAlpha,          // float,  alpha modifier applied to all plot item fills
    ErrorBarSize = ImPlotStyleVar_ErrorBarSize,       // float,  error bar whisker width in pixels
    ErrorBarWeight = ImPlotStyleVar_ErrorBarWeight,     // float,  error bar whisker weight in pixels
    DigitalBitHeight = ImPlotStyleVar_DigitalBitHeight,   // float,  digital channels bit height (at 1) in pixels
    DigitalBitGap = ImPlotStyleVar_DigitalBitGap,      // float,  digital channels bit padding gap in pixels
    PlotBorderSize = ImPlotStyleVar_PlotBorderSize,     // float,  thickness of border around plot area
    MinorAlpha = ImPlotStyleVar_MinorAlpha,         // float,  alpha multiplier applied to minor axis grid lines
    MajorTickLen = ImPlotStyleVar_MajorTickLen,       // ImVec2, major tick lengths for X and Y axes
    MinorTickLen = ImPlotStyleVar_MinorTickLen,       // ImVec2, minor tick lengths for X and Y axes
    MajorTickSize = ImPlotStyleVar_MajorTickSize,      // ImVec2, line thickness of major ticks
    MinorTickSize = ImPlotStyleVar_MinorTickSize,      // ImVec2, line thickness of minor ticks
    MajorGridSize = ImPlotStyleVar_MajorGridSize,      // ImVec2, line thickness of major grid lines
    MinorGridSize = ImPlotStyleVar_MinorGridSize,      // ImVec2, line thickness of minor grid lines
    PlotPadding = ImPlotStyleVar_PlotPadding,        // ImVec2, padding between widget frame and plot area, labels, or outside legends (i.e. main padding)
    LabelPadding = ImPlotStyleVar_LabelPadding,       // ImVec2, padding between axes labels, tick labels, and plot edge
    LegendPadding = ImPlotStyleVar_LegendPadding,      // ImVec2, legend padding from plot edges
    LegendInnerPadding = ImPlotStyleVar_LegendInnerPadding, // ImVec2, legend inner padding from legend edges
    LegendSpacing = ImPlotStyleVar_LegendSpacing,      // ImVec2, spacing between legend entries
    MousePosPadding = ImPlotStyleVar_MousePosPadding,    // ImVec2, padding between plot edge and interior info text
    AnnotationPadding = ImPlotStyleVar_AnnotationPadding,  // ImVec2, text padding around annotation labels
    FitPadding = ImPlotStyleVar_FitPadding,         // ImVec2, additional fit padding as a percentage of the fit extents (e.g. ImVec2(0.1f,0.1f) adds 10% to the fit extents of X and Y)
    PlotDefaultSize = ImPlotStyleVar_PlotDefaultSize,    // ImVec2, default size used when ImVec2(0,0) is passed to BeginPlot
    PlotMinSize = ImPlotStyleVar_PlotMinSize,        // ImVec2, minimum size plot frame can be when shrunk
};
FORCEINLINE constexpr ImPlotStyleVar_ operator*(EImPlotStyleVar PlotCol) { return (ImPlotStyleVar_)PlotCol; }

UENUM()
enum class EImPlotScale : uint8
{
	Linear = ImPlotScale_Linear, // default linear scale
	Time = ImPlotScale_Time,       // date/time scale
	Log10 = ImPlotScale_Log10,      // base 10 logartithmic scale
	SymLog = ImPlotScale_SymLog,     // symmetric log scale
};

UENUM()
enum class EImPlotMarker : uint8
{
	None = (uint8)ImPlotMarker_None, // no marker
	Circle = ImPlotMarker_Circle,    // a circle marker (default)
	Square = ImPlotMarker_Square,    // a square maker
	Diamond = ImPlotMarker_Diamond,   // a diamond marker
	Up = ImPlotMarker_Up,        // an upward-pointing triangle marker
	Down = ImPlotMarker_Down,      // an downward-pointing triangle marker
	Left = ImPlotMarker_Left,      // an leftward-pointing triangle marker
	Right = ImPlotMarker_Right,     // an rightward-pointing triangle marker
	Cross = ImPlotMarker_Cross,     // a cross marker (not fillable)
	Plus = ImPlotMarker_Plus,      // a plus marker (not fillable)
	Asterisk = ImPlotMarker_Asterisk,  // a asterisk marker (not fillable)
};
FORCEINLINE constexpr ImPlotMarker_ operator*(EImPlotMarker Bin) { return (ImPlotMarker_)static_cast<int8>(Bin); }

UENUM()
enum class EImPlotColormap : uint8
{
	Deep     = ImPlotColormap_Deep,   // a.k.a. seaborn deep             (qual=true,  n=10) (default)
	Dark     = ImPlotColormap_Dark,   // a.k.a. matplotlib "Set1"        (qual=true,  n=9 )
	Pastel   = ImPlotColormap_Pastel,   // a.k.a. matplotlib "Pastel1"     (qual=true,  n=9 )
	Paired   = ImPlotColormap_Paired,   // a.k.a. matplotlib "Paired"      (qual=true,  n=12)
	Viridis  = ImPlotColormap_Viridis,   // a.k.a. matplotlib "viridis"     (qual=false, n=11)
	Plasma   = ImPlotColormap_Plasma,   // a.k.a. matplotlib "plasma"      (qual=false, n=11)
	Hot      = ImPlotColormap_Hot,   // a.k.a. matplotlib/MATLAB "hot"  (qual=false, n=11)
	Cool     = ImPlotColormap_Cool,   // a.k.a. matplotlib/MATLAB "cool" (qual=false, n=11)
	Pink     = ImPlotColormap_Pink,   // a.k.a. matplotlib/MATLAB "pink" (qual=false, n=11)
	Jet      = ImPlotColormap_Jet,   // a.k.a. MATLAB "jet"             (qual=false, n=11)
	Twilight = ImPlotColormap_Twilight,  // a.k.a. matplotlib "twilight"    (qual=false, n=11)
	RdBu     = ImPlotColormap_RdBu,  // red/blue, Color Brewer          (qual=false, n=11)
	BrBG     = ImPlotColormap_BrBG,  // brown/blue-green, Color Brewer  (qual=false, n=11)
	PiYG     = ImPlotColormap_PiYG,  // pink/yellow-green, Color Brewer (qual=false, n=11)
	Spectral = ImPlotColormap_Spectral,  // color spectrum, Color Brewer    (qual=false, n=11)
	Greys    = ImPlotColormap_Greys,  // white/black                     (qual=false, n=2 )
};
FORCEINLINE constexpr ImPlotColormap_ operator*(EImPlotColormap Colormap) { return (ImPlotColormap_)Colormap; }

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImPlotLocation : int32
{
	Center    = ImPlotLocation_Center,                                          // center-center
	North     = ImPlotLocation_North,                                     // top-center
	South     = ImPlotLocation_South,                                     // bottom-center
	West      = ImPlotLocation_West,                                     // center-left
	East      = ImPlotLocation_East,                                     // center-right
	NorthWest = ImPlotLocation_NorthWest, // top-left
	NorthEast = ImPlotLocation_NorthEast, // top-right
	SouthWest = ImPlotLocation_SouthWest, // bottom-left
	SouthEast = ImPlotLocation_SouthEast  // bottom-right
};
ENUM_CLASS_FLAGS(EImPlotLocation);

UENUM()
enum class EImPlotBin : uint8
{
	Sqrt    = -ImPlotBin_Sqrt, // k = sqrt(n)
	Sturges = -ImPlotBin_Sturges, // k = 1 + log2(n)
	Rice    = -ImPlotBin_Rice, // k = 2 * cbrt(n)
	Scott   = -ImPlotBin_Scott, // w = 3.49 * sigma / cbrt(n)
};
FORCEINLINE constexpr ImPlotBin_ operator*(EImPlotBin Bin) { return (ImPlotBin_)-static_cast<int32>(Bin); }
