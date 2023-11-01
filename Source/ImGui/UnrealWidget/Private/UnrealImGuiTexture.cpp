// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiTexture.h"

#include "ImageUtils.h"
#include "Engine/TextureRenderTarget2D.h"

namespace UnrealImGui
{
	Private::FUpdateTextureData_WS Private::UpdateTextureData_WS;

	namespace ImGuiTextureId
	{
		// note: font texture use id 0
		uint32 HandleIdCounter = 0;
		TMap<const void*, uint32> HandleIdMap;
	}

	FImGuiTextureHandle::FImGuiTextureHandle(const UTexture* Texture)
	{
		using namespace ImGuiTextureId;
		uint32& Id = HandleIdMap.FindOrAdd(Texture);
		if (Id == 0)
		{
			HandleIdCounter += 1;
			Id = HandleIdCounter;
		}
		ImTextureId = Id;
	}

	FImGuiTextureHandle FImGuiTextureHandle::MakeUnique()
	{
		using namespace ImGuiTextureId;
		HandleIdCounter += 1;
		return { HandleIdCounter };
	}

	void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, uint8* Data)
	{
		if (ensure(Private::UpdateTextureData_WS))
		{
			Private::UpdateTextureData_WS(Handle, TextureFormat, Width, Height, Data);
		}
	}

	void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTexture2D* Texture2D)
	{
		if (Texture2D == nullptr)
		{
			return;
		}
		FImage Image;
		if (FImageUtils::GetTexture2DSourceImage(Texture2D, Image) == false)
		{
			return;
		}
		switch (Image.Format) {
		case ERawImageFormat::G8:
			UpdateTextureData(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Image.AsG8().GetData());
			break;
		case ERawImageFormat::BGRA8:
		case ERawImageFormat::BGRE8:
			switch (TextureFormat)
			{
			case ETextureFormat::Alpha8:
			case ETextureFormat::Gray8:
				{
					const TArrayView64<FColor> RawData = Image.Format == ERawImageFormat::BGRA8 ? Image.AsBGRA8() : Image.AsBGRE8();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx].A;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGB8:
				{
					const TArrayView64<FColor> RawData = Image.Format == ERawImageFormat::BGRA8 ? Image.AsBGRA8() : Image.AsBGRE8();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 3);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 3] = RawData[Idx].R;
						Data[Idx * 3 + 1] = RawData[Idx].G;
						Data[Idx * 3 + 2] = RawData[Idx].B;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGBA8:
				{
					const TArrayView64<FColor> RawData = Image.Format == ERawImageFormat::BGRA8 ? Image.AsBGRA8() : Image.AsBGRE8();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 4);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 4] = RawData[Idx].R;
						Data[Idx * 4 + 1] = RawData[Idx].G;
						Data[Idx * 4 + 2] = RawData[Idx].B;
						Data[Idx * 4 + 3] = RawData[Idx].A;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			}
			break;
		case ERawImageFormat::RGBA16:
			switch (TextureFormat)
			{
			case ETextureFormat::Alpha8:
			case ETextureFormat::Gray8:
				{
					constexpr float RGBA16Scale = (float)TNumericLimits<uint8>::Max() / TNumericLimits<uint16>::Max();
					const TArrayView64<uint16> RawData = Image.AsRGBA16();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
					for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx * 4 + 3] * RGBA16Scale;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGB8:
				{
					constexpr float RGBA16Scale = (float)TNumericLimits<uint8>::Max() / TNumericLimits<uint16>::Max();
					const TArrayView64<uint16> RawData = Image.AsRGBA16();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 3);
					for (int32 Idx = 0; Idx < Image.GetWidth() * Image.GetHeight(); ++Idx)
					{
						Data[Idx * 3] = RawData[Idx * 4] * RGBA16Scale;
						Data[Idx * 3 + 1] = RawData[Idx * 4 + 1] * RGBA16Scale;
						Data[Idx * 3 + 2] = RawData[Idx * 4 + 2] * RGBA16Scale;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGBA8:
				{
					constexpr float RGBA16Scale = (float)TNumericLimits<uint8>::Max() / TNumericLimits<uint16>::Max();
					const TArrayView64<uint16> RawData = Image.AsRGBA16();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 4);
					for (int32 Idx = 0; Idx < Image.GetWidth() * Image.GetHeight(); ++Idx)
					{
						Data[Idx * 4] = RawData[Idx * 4] * RGBA16Scale;
						Data[Idx * 4 + 1] = RawData[Idx * 4 + 1] * RGBA16Scale;
						Data[Idx * 4 + 2] = RawData[Idx * 4 + 2] * RGBA16Scale;
						Data[Idx * 4 + 3] = RawData[Idx * 4 + 3] * RGBA16Scale;
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			}
			break;
		case ERawImageFormat::RGBA16F:
			switch (TextureFormat)
			{
			case ETextureFormat::Alpha8:
			case ETextureFormat::Gray8:
				{
					const TArrayView64<FFloat16Color> RawData = Image.AsRGBA16F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
					for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx].A * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGB8:
				{
					const TArrayView64<FFloat16Color> RawData = Image.AsRGBA16F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 3);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 3] = RawData[Idx].R * TNumericLimits<uint8>::Max();
						Data[Idx * 3 + 1] = RawData[Idx].G * TNumericLimits<uint8>::Max();
						Data[Idx * 3 + 2] = RawData[Idx].B * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGBA8:
				{
					const TArrayView64<FFloat16Color> RawData = Image.AsRGBA16F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 4);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 4] = RawData[Idx].R * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 1] = RawData[Idx].G * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 2] = RawData[Idx].B * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 3] = RawData[Idx].A * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			}
			break;
		case ERawImageFormat::RGBA32F:
			switch (TextureFormat)
			{
			case ETextureFormat::Alpha8:
			case ETextureFormat::Gray8:
				{
					const TArrayView64<FLinearColor> RawData = Image.AsRGBA32F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
					for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx].A * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGB8:
				{
					const TArrayView64<FLinearColor> RawData = Image.AsRGBA32F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 3);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 3] = RawData[Idx].R * TNumericLimits<uint8>::Max();
						Data[Idx * 3 + 1] = RawData[Idx].G * TNumericLimits<uint8>::Max();
						Data[Idx * 3 + 2] = RawData[Idx].B * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			case ETextureFormat::RGBA8:
				{
					const TArrayView64<FLinearColor> RawData = Image.AsRGBA32F();
					TArray<uint8> Data;
					Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight() * 4);
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx * 4] = RawData[Idx].R * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 1] = RawData[Idx].G * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 2] = RawData[Idx].B * TNumericLimits<uint8>::Max();
						Data[Idx * 4 + 3] = RawData[Idx].A * TNumericLimits<uint8>::Max();
					}
					UpdateTextureData(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
				}
				break;
			}
			break;
		case ERawImageFormat::G16:
			{
				const TArrayView64<uint16> RawData = Image.AsG16();
				TArray<uint8> Data;
				Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
				for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
				{
					Data[Idx] = RawData[Idx] / (float)TNumericLimits<uint16>::Max() * TNumericLimits<uint8>::Max();
				}
				UpdateTextureData(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
			}
			break;
		case ERawImageFormat::R16F:
			{
				const TArrayView64<FFloat16> RawData = Image.AsR16F();
				TArray<uint8> Data;
				Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
				for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
				{
					Data[Idx] = RawData[Idx] * TNumericLimits<uint8>::Max();
				}
				UpdateTextureData(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
			}
			break;
		case ERawImageFormat::R32F:
			{
				const TArrayView64<float> RawData = Image.AsR32F();
				TArray<uint8> Data;
				Data.SetNumUninitialized(Image.GetWidth() * Image.GetHeight());
				for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
				{
					Data[Idx] = RawData[Idx] * TNumericLimits<uint8>::Max();
				}
				UpdateTextureData(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
			}
			break;
		default:
			ensure(false);
			return;
		}
	}

	void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, UTextureRenderTarget2D* RenderTarget2D)
	{
		if (RenderTarget2D == nullptr)
		{
			return;
		}

		FTextureRenderTarget2DResource* RTResource = static_cast<FTextureRenderTarget2DResource*>(RenderTarget2D->GameThread_GetRenderTargetResource());
		if (!ensure(RTResource))
		{
			return;
		}
		TArray<FColor> RawData;
		if (!ensure(RTResource->ReadPixels(RawData, {}, FIntRect{ 0, 0, RenderTarget2D->SizeX, RenderTarget2D->SizeY })))
		{
			return;
		}

		const ETextureRenderTargetFormat Format = RenderTarget2D->RenderTargetFormat;
		const bool bSingleChannel = Format == RTF_R8 || Format == RTF_R16f || Format == RTF_R32f;
		if (bSingleChannel && TextureFormat != ETextureFormat::Alpha8)
		{
			TextureFormat = ETextureFormat::Gray8;
		}

		switch (TextureFormat)
		{
		case ETextureFormat::Alpha8:
		case ETextureFormat::Gray8:
			{
				TArray<uint8> Data;
				Data.SetNumUninitialized(RenderTarget2D->SizeX * RenderTarget2D->SizeY);
				if (bSingleChannel)
				{
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx].R;
					}
				}
				else
				{
					for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
					{
						Data[Idx] = RawData[Idx].A;
					}
				}
				UpdateTextureData(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
			}
			break;
		case ETextureFormat::RGB8:
			{
				TArray<uint8> Data;
				Data.SetNumUninitialized(RenderTarget2D->SizeX * RenderTarget2D->SizeY * 3);
				for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
				{
					Data[Idx * 3] = RawData[Idx].R;
					Data[Idx * 3 + 1] = RawData[Idx].G;
					Data[Idx * 3 + 2] = RawData[Idx].B;
				}
				UpdateTextureData(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
			}
			break;
		case ETextureFormat::RGBA8:
			{
				TArray<uint8> Data;
				Data.SetNumUninitialized(RenderTarget2D->SizeX * RenderTarget2D->SizeY * 4);
				for (int32 Idx = 0; Idx < RawData.Num(); ++Idx)
				{
					Data[Idx * 4] = RawData[Idx].R;
					Data[Idx * 4 + 1] = RawData[Idx].G;
					Data[Idx * 4 + 2] = RawData[Idx].B;
					Data[Idx * 4 + 3] = RawData[Idx].A;
				}
				UpdateTextureData(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
			}
			break;
		default:
			ensure(false);
		}
	}
}
