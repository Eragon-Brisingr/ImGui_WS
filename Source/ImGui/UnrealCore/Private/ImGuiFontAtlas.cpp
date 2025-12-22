// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiFontAtlas.h"

#include "fa-solid-900.h"
#include "IconsFontAwesome.h"
#include "imgui.h"
#include "ImGuiSettings.h"
#include "Containers/Utf8String.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"

void UnrealImGui::ConfigDefaultFontAtlas(ImFontAtlas& FontAtlas)
{
	const UImGuiSettings* Settings = GetDefault<UImGuiSettings>();

	FontAtlas.RefCount = 1;
	ImFontConfig FontConfig;
	FontConfig.FontDataOwnedByAtlas = false;
	FontConfig.RasterizerDensity = GetGlobalDPIScale();
	static TArray<uint8> FontBin = [&]
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT(UE_PLUGIN_NAME));
		const FString PluginResourcesPath = Plugin->GetBaseDir() / TEXT("Resources");
		const FString FontPath = PluginResourcesPath / Settings->FontFileName;
		TArray<uint8> Bin;
		ensure(FFileHelper::LoadFileToArray(Bin, *FontPath));
		return Bin;
	}();
	FUtf8String FontName{ Settings->FontName };
	check(Settings->FontName.Len() < UE_ARRAY_COUNT(FontConfig.Name));
	FCStringAnsi::Strncpy(FontConfig.Name, reinterpret_cast<const char*>(*FontName), UE_ARRAY_COUNT(FontConfig.Name));
	FontAtlas.AddFontFromMemoryTTF(FontBin.GetData(), FontBin.Num(), Settings->FontSize, &FontConfig);

	// FontAwesome Icon
	const int32 IconFontSize = Settings->FontSize;
	static constexpr ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig IconsConfig;
	IconsConfig.MergeMode = true;
	IconsConfig.PixelSnapH = true;
	IconsConfig.GlyphMinAdvanceX = IconFontSize;
	FontAtlas.AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, IconFontSize, &IconsConfig, iconsRanges);
}

float UnrealImGui::GetSystemDPIScale()
{
	static const float SystemDPIScale = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(0, 0);
	return SystemDPIScale;
}

float UnrealImGui::GetGlobalDPIScale()
{
	const UImGuiPerUserSettings* Settings = GetDefault<UImGuiPerUserSettings>();
	static const bool bIsDedicatedServer = !GIsClient && GIsServer;
	if (Settings->bOverrideDPIScale || bIsDedicatedServer)
	{
		return Settings->OverrideDPIScale;
	}
	return GetSystemDPIScale();
}

void UnrealImGui::ShowGlobalDPISettings(float& OutFontScaleDpi)
{
	auto Settings = GetMutableDefault<UImGuiPerUserSettings>();
	ImGui::SetNextItemWidth(ImGui::GetFontSize());
	if (ImGui::Checkbox("##Override DPI Scale", &Settings->bOverrideDPIScale))
	{
		OutFontScaleDpi = UnrealImGui::GetGlobalDPIScale();
		Settings->SaveConfig();
	}
	if (ImGui::BeginItemTooltip())
	{
		ImGui::Text("Override DPI Scale");
		ImGui::EndTooltip();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 6.f);
	ImGui::BeginDisabled(!Settings->bOverrideDPIScale);
	if (ImGui::InputFloat("DPI Scale", &Settings->OverrideDPIScale, 0.01f))
	{
		Settings->OverrideDPIScale = FMath::Clamp(Settings->OverrideDPIScale, 0.5f, 3.f);
		OutFontScaleDpi = UnrealImGui::GetGlobalDPIScale();
		Settings->SaveConfig();
	}
	ImGui::EndDisabled();
}
