// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UTexture2D;
class UTextureRenderTarget2D;
class UTexture;

namespace UnrealImGui
{
	struct IMGUI_API FImGuiTextureHandle
	{
		FImGuiTextureHandle()
			: ImTextureId(0)
		{}
		FImGuiTextureHandle(const UTexture* Texture);
		static FImGuiTextureHandle MakeUnique();
		static FImGuiTextureHandle FindOrCreateHandle(const UTexture* Texture, bool& bCreated);

		explicit operator bool() const
		{
			return ImTextureId != 0;
		}
		operator uint32() const
		{
			return ImTextureId;
		}
	private:
		FImGuiTextureHandle(uint32 Id)
			: ImTextureId(Id)
		{}
		uint32 ImTextureId;
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

	IMGUI_API FImGuiTextureHandle FindOrAddTexture(ETextureFormat TextureFormat, UTexture* Texture);
	IMGUI_API const UTexture* FindTexture(uint32 ImTextureId);

	namespace Private
	{
		using FUpdateTextureData_WS = TFunction<void(FImGuiTextureHandle, ETextureFormat, int32, int32, uint8*)>;
		IMGUI_API extern FUpdateTextureData_WS UpdateTextureData_WS;
	}
}
