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
	FrameBg      = ImPlotCol_FrameBg,       // plot frame background color (defaults to ImGuiCol_FrameBg)
	PlotBg       = ImPlotCol_PlotBg,        // plot area background color (defaults to ImGuiCol_WindowBg)
	PlotBorder   = ImPlotCol_PlotBorder,    // plot area border color (defaults to ImGuiCol_Border)
	LegendBg     = ImPlotCol_LegendBg,      // legend background color (defaults to ImGuiCol_PopupBg)
	LegendBorder = ImPlotCol_LegendBorder,  // legend border color (defaults to ImPlotCol_PlotBorder)
	LegendText   = ImPlotCol_LegendText,    // legend text color (defaults to ImPlotCol_InlayText)
	TitleText    = ImPlotCol_TitleText,     // plot title text color (defaults to ImGuiCol_Text)
	InlayText    = ImPlotCol_InlayText,     // color of text appearing inside of plots (defaults to ImGuiCol_Text)
	AxisText     = ImPlotCol_AxisText,      // axis label and tick labels color (defaults to ImGuiCol_Text)
	AxisGrid     = ImPlotCol_AxisGrid,      // axis grid color (defaults to 25% ImPlotCol_AxisText)
	AxisTick     = ImPlotCol_AxisTick,      // axis tick color (defaults to AxisGrid)
	AxisBg       = ImPlotCol_AxisBg,        // background color of axis hover region (defaults to transparent)
	AxisBgHovered= ImPlotCol_AxisBgHovered, // axis hover color (defaults to ImGuiCol_ButtonHovered)
	AxisBgActive = ImPlotCol_AxisBgActive,  // axis active color (defaults to ImGuiCol_ButtonActive)
	Selection    = ImPlotCol_Selection,     // box-selection color (defaults to yellow)
	Crosshairs   = ImPlotCol_Crosshairs,    // crosshairs color (defaults to ImPlotCol_PlotBorder)
};
FORCEINLINE constexpr ImPlotCol_ operator*(EImPlotCol PlotCol) { return (ImPlotCol_)PlotCol; }

UENUM()
enum class EImPlotStyleVar : uint8
{
	PlotDefaultSize    = ImPlotStyleVar_PlotDefaultSize,    // ImVec2, default size used when ImVec2(0,0) is passed to BeginPlot
	PlotMinSize        = ImPlotStyleVar_PlotMinSize,        // ImVec2, minimum size plot frame can be when shrunk
	PlotBorderSize     = ImPlotStyleVar_PlotBorderSize,     // float,  thickness of border around plot area
	MinorAlpha         = ImPlotStyleVar_MinorAlpha,         // float,  alpha multiplier applied to minor axis grid lines
	MajorTickLen       = ImPlotStyleVar_MajorTickLen,       // ImVec2, major tick lengths for X and Y axes
	MinorTickLen       = ImPlotStyleVar_MinorTickLen,       // ImVec2, minor tick lengths for X and Y axes
	MajorTickSize      = ImPlotStyleVar_MajorTickSize,      // ImVec2, line thickness of major ticks
	MinorTickSize      = ImPlotStyleVar_MinorTickSize,      // ImVec2, line thickness of minor ticks
	MajorGridSize      = ImPlotStyleVar_MajorGridSize,      // ImVec2, line thickness of major grid lines
	MinorGridSize      = ImPlotStyleVar_MinorGridSize,      // ImVec2, line thickness of minor grid lines
	PlotPadding        = ImPlotStyleVar_PlotPadding,        // ImVec2, padding between widget frame and plot area, labels, or outside legends (i.e. main padding)
	LabelPadding       = ImPlotStyleVar_LabelPadding,       // ImVec2, padding between axes labels, tick labels, and plot edge
	LegendPadding      = ImPlotStyleVar_LegendPadding,      // ImVec2, legend padding from plot edges
	LegendInnerPadding = ImPlotStyleVar_LegendInnerPadding, // ImVec2, legend inner padding from legend edges
	LegendSpacing      = ImPlotStyleVar_LegendSpacing,      // ImVec2, spacing between legend entries
	MousePosPadding    = ImPlotStyleVar_MousePosPadding,    // ImVec2, padding between plot edge and interior info text
	AnnotationPadding  = ImPlotStyleVar_AnnotationPadding,  // ImVec2, text padding around annotation labels
	FitPadding         = ImPlotStyleVar_FitPadding,         // ImVec2, additional fit padding as a percentage of the fit extents (e.g. ImVec2(0.1f,0.1f) adds 10% to the fit extents of X and Y)
	DigitalPadding     = ImPlotStyleVar_DigitalPadding,     // float,  digital plot padding from bottom in pixels
	DigitalSpacing     = ImPlotStyleVar_DigitalSpacing,     // float,  digital plot spacing gap in pixels
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
	Auto = (uint8)ImPlotMarker_Auto, // use the next unused marker
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

UENUM()
enum class EImPlotProp : uint8
{
	LineColor = ImPlotProp_LineColor,
	LineColors = ImPlotProp_LineColors,
	LineWeight = ImPlotProp_LineWeight,
	FillColor = ImPlotProp_FillColor,
	FillColors = ImPlotProp_FillColors,
	FillAlpha = ImPlotProp_FillAlpha,
	Marker = ImPlotProp_Marker,
	MarkerSize = ImPlotProp_MarkerSize,
	MarkerSizes = ImPlotProp_MarkerSizes,
	MarkerLineColor = ImPlotProp_MarkerLineColor,
	MarkerLineColors = ImPlotProp_MarkerLineColors,
	MarkerFillColor = ImPlotProp_MarkerFillColor,
	MarkerFillColors = ImPlotProp_MarkerFillColors,
	Size = ImPlotProp_Size,
	Offset = ImPlotProp_Offset,
	Stride = ImPlotProp_Stride,
	Flags = ImPlotProp_Flags
};
FORCEINLINE constexpr ImPlotProp_ operator*(EImPlotProp Prop) { return (ImPlotProp_)Prop; }

USTRUCT(BlueprintType)
struct FImPlotSpec
{
	GENERATED_BODY()
	
	static constexpr FLinearColor AUTO_COL{ 0, 0, 0, -1 };

	// line color (applies to lines, bar edges); AUTO_COL will use next Colormap color or current item color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	FLinearColor LineColor = AUTO_COL;
	
	// line weight in pixels (applies to lines, bar edges, marker edges)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	float LineWeight      = 1.0f;

	// array of colors for each line; if empty, use LineColor for all lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	TArray<FLinearColor> LineColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	FLinearColor FillColor = AUTO_COL;

	// array of colors for each fill; if empty, use FillColor for all fills
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	TArray<FLinearColor> FillColors;
	
	// alpha multiplier (applies to FillColor, FillColors, MarkerFillColor, and MarkerFillColors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	float FillAlpha = 1.0f;
	
	// marker type; specify ImPlotMarker_Auto to use the next unused marker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	EImPlotMarker Marker = EImPlotMarker::None;
	
	// size of markers (radius) *in pixels*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	float MarkerSize = 4;

	// array of sizes for each marker; if empty, use MarkerSize for all markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	TArray<float> MarkerSizes;
	
	// marker edge color; IMPLOT_AUTO_COL will use LineColor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	FLinearColor MarkerLineColor = AUTO_COL;

	// array of colors for each marker edge; if empty, use MarkerLineColor for all markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	TArray<FLinearColor> MarkerLineColors;
	
	// marker face color; IMPLOT_AUTO_COL will use LineColor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	FLinearColor MarkerFillColor = AUTO_COL;

	// array of colors for each marker face; if empty, use MarkerFillColor for all markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	TArray<FLinearColor> MarkerFillColors;
	
	// size of error bar whiskers (width or height), and digital bars (height) *in pixels*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	float Size = 4;
	
	// data index offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	int32 Offset = 0;
	
	// data stride in bytes; IMPLOT_AUTO will result in sizeof(T) where T is the type passed to PlotX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImPlot)
	int32 Stride = IMPLOT_AUTO;
	
	// optional item flags; can be composed from common ImPlotItemFlags and/or specialized ImPlotXFlags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotItemFlags"), Category = ImPlot)
	int32 Flags = ImPlotItemFlags_None;

	bool SetProp(EImPlotProp Prop, double Value)
	{
		switch (*Prop)
		{
		case ImPlotProp_LineColor:
			LineColor = FLinearColor{ ImGui::ColorConvertU32ToFloat4((ImU32)Value) };
			return true;
		case ImPlotProp_LineWeight:
			LineWeight = (float)Value;
			return true;
		case ImPlotProp_FillColor:
			FillColor = FLinearColor{ ImGui::ColorConvertU32ToFloat4((ImU32)Value) };
			return true;
		case ImPlotProp_FillAlpha:
			FillAlpha = (float)Value;
			return true;
		case ImPlotProp_Marker:
			Marker = (EImPlotMarker)(uint8)(int8)FMath::RoundToInt(Value);
			return true;
		case ImPlotProp_MarkerSize:
			MarkerSize = (float)Value;
			return true;
		case ImPlotProp_MarkerLineColor:
			MarkerLineColor = FLinearColor{ ImGui::ColorConvertU32ToFloat4((ImU32)Value) };
			return true;
		case ImPlotProp_MarkerFillColor:
			MarkerFillColor = FLinearColor{ ImGui::ColorConvertU32ToFloat4((ImU32)Value) };
			return true;
		case ImPlotProp_Size:
			Size = (float)Value;
			return true;
		case ImPlotProp_Offset:
			Offset = (int32)FMath::RoundToInt(Value);
			return true;
		case ImPlotProp_Stride:
			Stride = (int32)FMath::RoundToInt(Value);
			return true;
		case ImPlotProp_Flags:
			Flags = (int32)FMath::RoundToInt(Value);
			return true;
		default:
			return false;
		}
	}

	bool SetProp(EImPlotProp Prop, const FLinearColor& Value)
	{
		switch (*Prop)
		{
		case ImPlotProp_LineColor:
			LineColor = Value;
			return true;
		case ImPlotProp_FillColor:
			FillColor = Value;
			return true;
		case ImPlotProp_MarkerLineColor:
			MarkerLineColor = Value;
			return true;
		case ImPlotProp_MarkerFillColor:
			MarkerFillColor = Value;
			return true;
		default:
			return false;
		}
	}

	bool SetProp(EImPlotProp Prop, const TArray<FLinearColor>& Values)
	{
		switch (*Prop)
		{
		case ImPlotProp_LineColors:
			LineColors = Values;
			return true;
		case ImPlotProp_FillColors:
			FillColors = Values;
			return true;
		case ImPlotProp_MarkerLineColors:
			MarkerLineColors = Values;
			return true;
		case ImPlotProp_MarkerFillColors:
			MarkerFillColors = Values;
			return true;
		default:
			return false;
		}
	}

	bool SetProp(EImPlotProp Prop, const TArray<float>& Values)
	{
		switch (*Prop)
		{
		case ImPlotProp_MarkerSizes:
			MarkerSizes = Values;
			return true;
		default:
			return false;
		}
	}

	operator ImPlotSpec() const
	{
		ImPlotSpec Spec;
		Spec.LineColor = ImVec4{ LineColor };
		Spec.LineColors = BuildColorBuffer(LineColors, CachedLineColors);
		Spec.LineWeight = LineWeight;
		Spec.FillColor = ImVec4{ FillColor };
		Spec.FillColors = BuildColorBuffer(FillColors, CachedFillColors);
		Spec.FillAlpha = FillAlpha;
		Spec.Marker = (ImPlotMarker)Marker;
		Spec.MarkerSize = MarkerSize;
		Spec.MarkerSizes = MarkerSizes.Num() == 0 ? nullptr : const_cast<float*>(MarkerSizes.GetData());
		Spec.MarkerLineColor = ImVec4{ MarkerLineColor };
		Spec.MarkerLineColors = BuildColorBuffer(MarkerLineColors, CachedMarkerLineColors);
		Spec.MarkerFillColor = ImVec4{ MarkerFillColor };
		Spec.MarkerFillColors = BuildColorBuffer(MarkerFillColors, CachedMarkerFillColors);
		Spec.Size = Size;
		Spec.Offset = Offset;
		Spec.Stride = Stride;
		Spec.Flags = Flags;

		return Spec;
	}

private:
	static ImU32 ToImU32(const FLinearColor& Color)
	{
		return ImGui::ColorConvertFloat4ToU32(ImVec4{ Color });
	}

	static ImU32* BuildColorBuffer(const TArray<FLinearColor>& Source, TArray<ImU32>& Buffer)
	{
		if (Source.Num() == 0)
		{
			return nullptr;
		}
		Buffer.Reset(Source.Num());
		Buffer.Reserve(Source.Num());
		for (const FLinearColor& Color : Source)
		{
			Buffer.Add(ToImU32(Color));
		}
		return Buffer.GetData();
	}

	mutable TArray<ImU32> CachedLineColors;
	mutable TArray<ImU32> CachedFillColors;
	mutable TArray<ImU32> CachedMarkerLineColors;
	mutable TArray<ImU32> CachedMarkerFillColors;
};
