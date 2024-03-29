// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiFontAtlas.h"

#include "imgui.h"
#include "imgui_notify.h"
#include "ImGuiSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"

ImFontAtlas& UnrealImGui::GetDefaultFontAtlas()
{
	static ImFontAtlas DefaultFontAtlas = []
	{
		constexpr float DPIScale = 1.f;
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT(UE_PLUGIN_NAME));
		const FString PluginResourcesPath = Plugin->GetBaseDir() / TEXT("Resources");

		ImFontAtlas FontAtlas;
		const UImGuiSettings* Settings = GetDefault<UImGuiSettings>();
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

		// Initialize notify
		ImGui::MergeIconsWithLatestFont(FontAtlas, 12.f * DPIScale, false);

		return FontAtlas;
	}();
	return DefaultFontAtlas;
}
