// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiTexture.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;
class UTexture;

USTRUCT(BlueprintType)
struct IMGUI_API FImGuiTextureHandle
{
	GENERATED_BODY()

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
	bool IsValid() const { return ImTextureId != 0; }
private:
	FImGuiTextureHandle(uint32 Id)
		: ImTextureId(Id)
	{}
	uint32 ImTextureId;
};

namespace UnrealImGui
{
	enum class ETextureFormat : uint8
	{
		Alpha8 = 0,
		Gray8 = 1,
		RGB8 = 2,
		RGBA8 = 3,
	};

	IMGUI_API UTextureRenderTarget2D* CreateTexture(FImGuiTextureHandle& Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, UObject* Outer, const FName& Name = NAME_None);
	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, const uint8* Data, UTextureRenderTarget2D* Texture);
	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTexture2D* Texture2D);
	IMGUI_API void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTextureRenderTarget2D* RenderTarget2D);

	IMGUI_API FImGuiTextureHandle FindOrAddTexture(ETextureFormat TextureFormat, UTexture* Texture);
	IMGUI_API const UTexture* FindTexture(uint32 ImTextureId);

	namespace Private
	{
		using FUpdateTextureData_WS = TFunction<void(FImGuiTextureHandle, ETextureFormat, int32, int32, const uint8*)>;
		IMGUI_API extern FUpdateTextureData_WS UpdateTextureData_WS;
	}
}
