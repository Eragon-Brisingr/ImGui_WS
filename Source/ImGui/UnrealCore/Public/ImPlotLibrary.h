// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiLibraryBase.h"
#include "implot.h"
#include "PlatformInfo.h"
#include "UnrealImGuiTexture.h"
#include "UnrealImGuiTypes.h"
#include "UnrealImPlotTypes.h"
#include "Containers/Utf8String.h"
#include "ImPlotLibrary.generated.h"

UCLASS(meta = (FunctionPrefixOverride = ImPlot))
class IMGUI_API UImPlotLibrary : public UImGuiLibraryBase
{
	GENERATED_BODY()

	struct FUtf8Labels
	{
		TArray<const char*> RawLabels;
		TArray<FUtf8String> Utf8Labels;
		FUtf8Labels(const TArray<FName>& Labels)
		{
			if (Labels.Num() == 0)
			{
				return;
			}
			RawLabels.SetNum(Labels.Num());
			Utf8Labels.SetNum(Labels.Num());
			for (int32 Idx = 0; Idx < Labels.Num(); ++Idx)
			{
				Utf8Labels[Idx] = FUtf8String{ Labels[Idx].ToString() };
				RawLabels[Idx] = (const char*)Utf8Labels[Idx].GetCharArray().GetData();
			}
		}
		const char* const* GetData() const { return RawLabels.Num() > 0 ? RawLabels.GetData() : nullptr; }
	};
public:
	UFUNCTION(BlueprintCallable, Category="ImPlot", meta = (ImGuiScopeExit = EndPlot, DisplayName = Plot, Name = "Untitle"), BlueprintInternalUseOnly)
	static bool BeginPlot(FName Title, FVector2D Size = FVector2D(-1, 0), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::BeginPlot(TCHAR_TO_UTF8(*Title.ToString()), ImVec2{ Size }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot", BlueprintInternalUseOnly)
	static void EndPlot()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::EndPlot();
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Layout", meta = (ImGuiScopeExit = EndSubplots, DisplayName = Subplots, Name = "Untitle"), BlueprintInternalUseOnly)
	static bool BeginSubplots(FName Title, int32 Rows, int32 Columns, FVector2D Size = FVector2D(-1, 0), UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotSubplotFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::BeginSubplots(TCHAR_TO_UTF8(*Title.ToString()), Rows, Columns, ImVec2{ Size }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Layout", BlueprintInternalUseOnly)
	static void EndSubplots()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::EndSubplots();
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxis(EImAxis Axis, const FString& Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotAxisFlags"))int32 Flags=0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImPlot::SetupAxis(*Axis, TCHAR_TO_UTF8(*Label), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisLimits(EImAxis Axis, double Min, double Max, EImPlotCond Cond = EImPlotCond::Once)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImPlot::SetupAxisLimits(*Axis, Min, Max, *Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisLinks(EImAxis Axis, double& LinkMin, double& LinkMax)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImPlot::SetupAxisLinks(*Axis, &LinkMin, &LinkMax);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisFormat(EImAxis Axis, const FString& Fmt)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		return ImPlot::SetupAxisFormat(*Axis, TCHAR_TO_UTF8(*Fmt));
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisTicks(EImAxis Axis, const TArray<double>& TickValues, const TArray<FName>& Labels, bool bKeepDefault = false)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		const FUtf8Labels Utf8Labels{ Labels };
		ImPlot::SetupAxisTicks(*Axis, TickValues.GetData(), TickValues.Num(), Utf8Labels.GetData(), bKeepDefault);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction, DisplayName = "SetupAxisTicks (Range)", ScriptName = SetupAxisTicks), BlueprintInternalUseOnly)
	static void SetupAxisTicksRange(EImAxis Axis, double Min, double Max, const TArray<FName>& Labels, bool bKeepDefault = false)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		const FUtf8Labels Utf8Labels{ Labels };
		ImPlot::SetupAxisTicks(*Axis, Min, Max, Labels.Num(), Utf8Labels.GetData(), bKeepDefault);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisScale(EImAxis Axis, EImPlotScale Scale)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupAxisScale(*Axis, (ImPlotScale)Scale);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisLimitsConstraints(EImAxis Axis, double Min, double Max)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupAxisLimitsConstraints(*Axis, Min, Max);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxisZoomConstraints(EImAxis Axis, double Min, double Max)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupAxisZoomConstraints(*Axis, Min, Max);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxes(const FString& LabelX, const FString& LabelY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotAxisFlags"))int32 FlagsX = 0, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotAxisFlags"))int32 FlagsY = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupAxes(TCHAR_TO_UTF8(*LabelX), TCHAR_TO_UTF8(*LabelY), FlagsX, FlagsY);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupAxesLimits(double MinX, double MaxX, double MinY, double MaxY, EImPlotCond Cond = EImPlotCond::Once)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupAxesLimits(MinX, MaxX, MinY, MaxY, *Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupLegend(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotLocation"))int32 Location, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotLegendFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupLegend(Location, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupMouseText(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotLocation"))int32 Location, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotMouseTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupMouseText(Location, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Setup", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetupFinish()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetupFinish();
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|SetNext", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextAxisLimits(EImAxis Axis, double Min, double Max, EImPlotCond Cond = EImPlotCond::Once)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextAxisLimits(*Axis, Min, Max, *Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|SetNext", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextAxisLinks(EImAxis Axis, double& LinkMin, double& LinkMax)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextAxisLinks(*Axis, &LinkMin, &LinkMax);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|SetNext", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextAxisToFit(EImAxis Axis)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextAxisToFit(*Axis);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|SetNext", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextAxesLimits(double MinX, double MaxX, double MinY, double MaxY, EImPlotCond Cond = EImPlotCond::Once)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextAxesLimits(MinX, MaxX, MinY, MaxY, *Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|SetNext", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextAxesToFit()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextAxesToFit();
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotLine(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotLineFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotLine(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotScatter(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotScatterFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotScatter(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotStairs(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotStairsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotStairs(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotShaded(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotShadedFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotShaded(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction, DisplayName = "PlotShaded (Y2)", ScriptName = PlotShaded), BlueprintInternalUseOnly)
	static void PlotShadedEx(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, const TArray<float>& ValuesY2, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotShadedFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotShaded(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesY2.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotBars(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, double BarSize, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotBarsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotBars(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), BarSize, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotBarGroups(const TArray<FName>& Labels, const TArray<float>& Values, int32 GroupCount, double GroupSize = 0.67, double Shift = 0, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotBarGroupsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		const FUtf8Labels Utf8Labels{ Labels };
		ImPlot::PlotBarGroups(Utf8Labels.GetData(), Values.GetData(), Values.Num(), GroupCount, GroupSize, Shift, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotErrorBars(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, const TArray<float>& Errors, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotErrorBarsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotErrorBars(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), Errors.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction, DisplayName = "PlotErrorBars (Neg & Pos)", ScriptName = PlotErrorBars), BlueprintInternalUseOnly)
	static void PlotErrorBarsEx(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, const TArray<float>& Neg, const TArray<float>& Pos, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotErrorBarsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotErrorBars(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), Neg.GetData(), Pos.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotStems(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, double Ref = 0, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotStemsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotStems(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Ref, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotInfLines(FName Label, const TArray<float>& Values, int32 Count, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotInfLinesFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotInfLines(TCHAR_TO_UTF8(*Label.ToString()), Values.GetData(), Count, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotPieChart(const TArray<FName>& Labels, const TArray<float>& Values, double X, double Y, double Radius, const FString& Fmt = TEXT("%.1f"), double Angle0 = 90, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotPieChartFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		const FUtf8Labels Utf8Labels{ Labels };
		ImPlot::PlotPieChart(Utf8Labels.GetData(), Values.GetData(), Values.Num(), X, Y, Radius, TCHAR_TO_UTF8(*Fmt), Angle0, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotHistogram(FName Label, const TArray<float>& Values, EImPlotBin Bins = EImPlotBin::Sturges, double BarScale = 1.0, FVector2D Range = FVector2D::ZeroVector, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotHistogramFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotHistogram(TCHAR_TO_UTF8(*Label.ToString()), Values.GetData(), Values.Num(), *Bins, BarScale, ImPlotRange{ Range.X, Range.Y }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotHistogram2D(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, EImPlotBin BinsX = EImPlotBin::Sturges, EImPlotBin BinsY = EImPlotBin::Sturges, FVector2D RectMin = FVector2D::ZeroVector, FVector2D RectMax = FVector2D::ZeroVector, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotHistogramFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotHistogram2D(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), *BinsX, *BinsY, ImPlotRect{ RectMin.X, RectMax.X, RectMin.Y, RectMax.Y }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotDigital(FName Label, const TArray<float>& ValuesX, const TArray<float>& ValuesY, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotStemsFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotDigital(TCHAR_TO_UTF8(*Label.ToString()), ValuesX.GetData(), ValuesY.GetData(), ValuesX.Num(), Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotImage(FName Label, const FImGuiTextureHandle& TextureHandle, const FVector2D& BoundsMin, const FVector2D& BoundsMax, const FVector2D& UV0 = FVector2D::ZeroVector, const FVector2D& UV1 = FVector2D::UnitVector, const FLinearColor& TintColor = FLinearColor::White, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotImageFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotImage(TCHAR_TO_UTF8(*Label.ToString()), TextureHandle, ImPlotPoint{ BoundsMin.X, BoundsMin.Y }, ImPlotPoint{ BoundsMax.X, BoundsMax.Y }, ImVec2{ UV0 }, ImVec2{ UV1 }, ImVec4{ TintColor }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotText(const FString& Text, double X, double Y, const FVector2D& PixOffset = FVector2D::ZeroVector, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotTextFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotText(TCHAR_TO_UTF8(*Text), X, Y, ImVec2{ PixOffset }, Flags);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Items", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void PlotDummy(FName Label, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImPlotDummyFlags"))int32 Flags = 0)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PlotDummy(TCHAR_TO_UTF8(*Label.ToString()), Flags);
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Tools", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void Annotation(double X, double Y, const FString& Annotation, const FLinearColor& Color = FLinearColor::White, const FVector2D& PixOffset = FVector2D::ZeroVector, bool bClamp = false)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::Annotation(X, Y, ImVec4{ Color }, ImVec2{ PixOffset }, bClamp, "%s", TCHAR_TO_UTF8(*Annotation));
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Tools", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TagX(double X, const FString& Tag, const FLinearColor& Color = FLinearColor::White)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::TagX(X, ImVec4{ Color }, "%s", TCHAR_TO_UTF8(*Tag));
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Tools", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void TagY(double Y, const FString& Tag, const FLinearColor& Color = FLinearColor::White)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::TagY(Y, ImVec4{ Color }, "%s", TCHAR_TO_UTF8(*Tag));
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetAxis(EImAxis Axis)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetAxis(*Axis);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetAxes(EImAxis AxisX, EImAxis AxisY)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetAxes(*AxisX, *AxisY);
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D PixelsToPlot(const FVector2D& Pix, EImAxis AxisX = EImAxis::Auto, EImAxis AxisY = EImAxis::Auto)
	{
		if (!CheckImGuiContextThrowError()) { return FVector2D::ZeroVector; }
		const auto Point = ImPlot::PixelsToPlot(Pix.X, Pix.Y, *AxisX, *AxisY);
		return { Point.x, Point.y };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D PlotToPixels(const FVector2D& PlotPos, EImAxis AxisX = EImAxis::Auto, EImAxis AxisY = EImAxis::Auto)
	{
		if (!CheckImGuiContextThrowError()) { return FVector2D::ZeroVector; }
		const auto Point = ImPlot::PlotToPixels(PlotPos.X, PlotPos.Y, *AxisX, *AxisY);
		return { Point.x, Point.y };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetPlotPos()
	{
		if (!CheckImGuiContextThrowError()) { return FVector2D::ZeroVector; }
		return FVector2D{ ImPlot::GetPlotPos() };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetPlotSize()
	{
		if (!CheckImGuiContextThrowError()) { return FVector2D::ZeroVector; }
		return FVector2D{ ImPlot::GetPlotSize() };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FVector2D GetPlotMousePos(EImAxis AxisX = EImAxis::Auto, EImAxis AxisY = EImAxis::Auto)
	{
		if (!CheckImGuiContextThrowError()) { return FVector2D::ZeroVector; }
		const auto Point = ImPlot::GetPlotMousePos(*AxisX, *AxisY);
		return { Point.x, Point.y };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FBox2D GetPlotLimits(EImAxis AxisX = EImAxis::Auto, EImAxis AxisY = EImAxis::Auto)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		const auto Rect = ImPlot::GetPlotLimits(*AxisX, *AxisY);
		return { { Rect.X.Min, Rect.Y.Min }, { Rect.X.Max, Rect.Y.Max } };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsPlotHovered()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::IsPlotHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsAxisHovered(EImAxis Axis)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::IsAxisHovered(*Axis);
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsSubplotsHovered()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::IsSubplotsHovered();
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsPlotSelected()
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::IsPlotSelected();
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FBox2D GetPlotSelection(EImAxis AxisX = EImAxis::Auto, EImAxis AxisY = EImAxis::Auto)
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		const auto Rect = ImPlot::GetPlotSelection(*AxisX, *AxisY);
		return { { Rect.X.Min, Rect.Y.Min }, { Rect.X.Max, Rect.Y.Max } };
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void CancelPlotSelection()
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::CancelPlotSelection();
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void HideNextItem(bool bHidden = true, EImPlotCond Cond = EImPlotCond::Once)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::HideNextItem(bHidden, *Cond);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", meta = (ImGuiScopeExit = EndAlignedPlots, DisplayName = AlignedPlots, Name = "Untitle"), BlueprintInternalUseOnly)
	static bool BeginAlignedPlots(FName Group, bool bVertical = true)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::BeginAlignedPlots(TCHAR_TO_UTF8(*Group.ToString()), bVertical);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Utils", BlueprintInternalUseOnly)
	static void EndAlignedPlots()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::EndAlignedPlots();
	}
	
	UFUNCTION(BlueprintCallable, Category="ImPlot|Legend", meta = (ImGuiScopeExit = EndLegendPopup, DisplayName = LegendPopup, Name = "Untitle"), BlueprintInternalUseOnly)
	static bool BeginLegendPopup(FName Label, EImGuiMouseButton MouseButton = EImGuiMouseButton::Left)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::BeginLegendPopup(TCHAR_TO_UTF8(*Label.ToString()), *MouseButton);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Legend", BlueprintInternalUseOnly)
	static void EndLegendPopup()
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::EndLegendPopup();
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Utils", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static bool IsLegendEntryHovered(FName Label)
	{
		if (!CheckImGuiContextThrowError()) { return false; }
		return ImPlot::IsLegendEntryHovered(TCHAR_TO_UTF8(*Label.ToString()));
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiScopeExit = PopStyleColor, DisplayName = StyleColor), BlueprintInternalUseOnly)
	static void PushStyleColor(EImPlotCol PlotCol, FLinearColor Color)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PushStyleColor(*PlotCol, ImVec4{ Color });
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", BlueprintInternalUseOnly)
	static void PopStyleColor(int32 Count = 1)
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::PopStyleColor(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiScopeExit = PopStyleVar, DisplayName = "StyleVar (float)", ScriptName = PushStyleVar), BlueprintInternalUseOnly)
	static void PushStyleVarFloat(EImPlotStyleVar PlotCol, float Val)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PushStyleVar(*PlotCol, Val);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiScopeExit = PopStyleVar, DisplayName = "StyleVar (Int32)", ScriptName = PushStyleVar), BlueprintInternalUseOnly)
	static void PushStyleVarInt32(EImPlotStyleVar PlotCol, int32 Val)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PushStyleVar(*PlotCol, Val);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiScopeExit = PopStyleVar, DisplayName = "StyleVar (Vector2)", ScriptName = PushStyleVar), BlueprintInternalUseOnly)
	static void PushStyleVarVector2(EImPlotStyleVar PlotCol, FVector2D Val)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PushStyleVar(*PlotCol, ImVec2{ Val });
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", BlueprintInternalUseOnly)
	static void PopStyleVar(int32 Count = 1)
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::PopStyleVar(Count);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextLineStyle(const FLinearColor Color = FLinearColor(0, 0, 0, -1), float Weight = -1.f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextLineStyle(ImVec4{ Color }, Weight);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextFillStyle(const FLinearColor Color = FLinearColor(0, 0, 0, -1), float AlphaMod = -1.f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextFillStyle(ImVec4{ Color }, AlphaMod);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextMarkerStyle(EImPlotMarker Marker = EImPlotMarker::None, float Size = -1.f, const FLinearColor& Fill = FLinearColor(0, 0, 0, -1), float Weight = -1.f, const FLinearColor& Outline = FLinearColor(0, 0, 0, -1))
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextMarkerStyle(*Marker, Size, ImVec4{ Fill }, Weight, ImVec4{ Outline });
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static void SetNextErrorBarStyle(const FLinearColor Color = FLinearColor(0, 0, 0, -1), float Size = -1.f, float Weight = -1.f)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::SetNextErrorBarStyle(ImVec4{ Color }, Size, Weight);
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FLinearColor GetLastItemColor()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FLinearColor{ ImPlot::GetLastItemColor() };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FName GetStyleColorName(EImPlotCol PlotCol)
	{
		if (!CheckImGuiContextThrowError()) { return NAME_None; }
		const auto Name = ImPlot::GetStyleColorName(*PlotCol);
		if (Name == nullptr)
		{
			return NAME_None;
		}
		return { UTF8_TO_TCHAR(Name) };
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Styling", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FName GetMarkerName(EImPlotMarker Marker)
	{
		if (!CheckImGuiContextThrowError()) { return NAME_None; }
		const auto Name = ImPlot::GetMarkerName(*Marker);
		if (Name == nullptr)
		{
			return NAME_None;
		}
		return { UTF8_TO_TCHAR(Name) };
	}

	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", meta = (ImGuiScopeExit = PopColormap), BlueprintInternalUseOnly)
	static void PushColormap(EImPlotColormap Colormap)
	{
		if (!CheckImGuiContextThrowError()) { return; }
		ImPlot::PushColormap(*Colormap);
	}
	UFUNCTION(BlueprintCallable, Category="ImPlot|Styling", BlueprintInternalUseOnly)
	static void PopColormap(int32 Count = 1)
	{
		if (!ImGui::GetCurrentContext()) { return; }
		ImPlot::PopColormap(Count);
	}
	UFUNCTION(BlueprintPure, Category="ImPlot|Colormaps", meta = (ImGuiFunction), BlueprintInternalUseOnly)
	static FLinearColor NextColormapColor()
	{
		if (!CheckImGuiContextThrowError()) { return {}; }
		return FLinearColor{ ImPlot::NextColormapColor() };
	}
};
