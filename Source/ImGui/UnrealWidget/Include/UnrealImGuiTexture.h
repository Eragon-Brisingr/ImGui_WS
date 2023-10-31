// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace UnrealImGui
{
	struct FImGuiTextureHandle
	{
		FImGuiTextureHandle(const UTexture* Texture)
			: ImTextureID{ reinterpret_cast<intptr_t>(Texture) }
		{}
		operator void*() const
		{
			return reinterpret_cast<void*>(ImTextureID);
		}
		operator intptr_t() const
		{
			return ImTextureID;
		}
	private:
		intptr_t ImTextureID;
	};

	enum class ETextureFormat : uint8
	{
		Alpha8 = 0,
		Gray8 = 1,
		RGB8 = 2,
		RGBA8 = 3,
	};

	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, uint8* Data);
	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTexture2D* Texture2D);
	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTextureRenderTarget2D* RenderTarget2D);

	namespace Private
	{
		using FUpdateTextureData_WS = TFunction<void(FImGuiTextureHandle, ETextureFormat, int32, int32, uint8*)>;
		IMGUI_API extern FUpdateTextureData_WS UpdateTextureData_WS;
	}
}