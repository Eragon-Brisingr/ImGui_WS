// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiFontAtlas.h"

#include "imgui.h"
#include "ImGuiSettings.h"
#include "imgui_notify.h"
#include "UnrealImGuiString.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"

ImFontAtlas& UnrealImGui::GetDefaultFontAtlas()
{
	static ImFontAtlas DefaultFontAtlas = []
	{
		const UImGuiSettings* Settings = GetDefault<UImGuiSettings>();

		float DPIScale = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(0, 0);
		if (DPIScale == 1.f)
		{
			DPIScale = Settings->DefaultDPIScale;
		}
		
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT(UE_PLUGIN_NAME));
		const FString PluginResourcesPath = Plugin->GetBaseDir() / TEXT("Resources");

		ImFontAtlas FontAtlas;
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
		const FString FontPath = PluginResourcesPath / Settings->FontFileName;

		TArray<uint8> Bin;
		ensure(FFileHelper::LoadFileToArray(Bin, *FontPath));
		FUTF8String FontName{ Settings->FontName };
		FCStringAnsi::Strcpy(FontConfig.Name, FontName.Len() + 1, FontName.GetData());
		FontAtlas.AddFontFromMemoryTTF(Bin.GetData(), Bin.Num(), Settings->FontSize * DPIScale, &FontConfig);

		// Initialize notify
		ImGui::MergeIconsWithLatestFont(FontAtlas, Settings->FontSize * DPIScale, false);

		// build font
		FontAtlas.Build();

		return FontAtlas;
	}();
	return DefaultFontAtlas;
}
