// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiTexture.h"

#include "ImageUtils.h"
#include "imgui.h"
#include "RenderingThread.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

namespace UnrealImGui
{
	Private::FUpdateTextureData_WS Private::UpdateTextureData_WS;

	namespace TextureIdManager
	{
		// note: font texture use id 0
		uint32 HandleIdCounter = 0;
		TMap<TWeakObjectPtr<const UTexture>, uint32> HandleIdMap;
		TMap<uint32, TWeakObjectPtr<const UTexture>> IdTextureMap;
	}

	UTextureRenderTarget2D* CreateTexture(FImGuiTextureHandle& Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, UObject* Outer, const FName& Name)
	{
		UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(Outer, Name);
		check(RT);
		RT->RenderTargetFormat = RTF_RGBA8;
		RT->ClearColor = FLinearColor::Black;
		RT->bAutoGenerateMips = false;
		RT->InitAutoFormat(Width, Height);
		RT->LODGroup = TEXTUREGROUP_Pixels2D;
		RT->UpdateResourceImmediate(false);
		Handle = FImGuiTextureHandle{ RT };
		return RT;
	}

	void UpdateTextureDataToWS(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, const uint8* Data)
	{
		if (Private::UpdateTextureData_WS)
		{
			Private::UpdateTextureData_WS(Handle, TextureFormat, Width, Height, Data);
		}
	}

	void UpdateTextureData(FImGuiTextureHandle Handle, ETextureFormat TextureFormat, int32 Width, int32 Height, const uint8* Data, UTextureRenderTarget2D* Texture)
	{
		UpdateTextureDataToWS(Handle, TextureFormat, Width, Height, Data);
		if (Texture)
		{
			int32 BytesPerPixel;
			switch (TextureFormat)
			{
			case ETextureFormat::Alpha8:
			case ETextureFormat::Gray8:
				BytesPerPixel = 1;
				break;
			case ETextureFormat::RGB8:
				BytesPerPixel = 3;
				break;
			case ETextureFormat::RGBA8:
				BytesPerPixel = 4;
				break;
			default:
				checkNoEntry();
				BytesPerPixel = 0;
			}
			ENQUEUE_RENDER_COMMAND(ImGuiFontAtlas)(
				[TextureDataRaw = TArray<uint8>{ Data, Width * Height * BytesPerPixel },
				TextureFormat,
				RenderTargetPtr = TWeakObjectPtr<UTextureRenderTarget2D>(Texture)]
				(FRHICommandListImmediate& RHICmdList)
				{
					UTextureRenderTarget2D* RT = RenderTargetPtr.Get();
					if (!RT)
					{
						return;
					}
					const FTextureResource* RenderTargetResource = RT->GetResource();
					if (RenderTargetResource == nullptr)
					{
						return;
					}

					TArray<uint8> FontAtlasTextureData;
					FontAtlasTextureData.SetNumUninitialized(TextureDataRaw.Num() * 4);
					{
						const uint8* Src = TextureDataRaw.GetData();
						uint32* Dst = reinterpret_cast<uint32*>(FontAtlasTextureData.GetData());
						switch (TextureFormat)
						{
						case ETextureFormat::Alpha8:
							for (int32 Idx = RT->SizeX * RT->SizeY; Idx > 0; --Idx)
							{
								*Dst++ = IM_COL32(255, 255, 255, *Src++);
							}
							break;
						case ETextureFormat::Gray8:
							for (int32 Idx = RT->SizeX * RT->SizeY; Idx > 0; --Idx)
							{
								const uint8 V = *Src++;
								*Dst++ = IM_COL32(V, V, V, 255);
							}
							break;
						case ETextureFormat::RGB8:
							for (int32 Idx = RT->SizeX * RT->SizeY; Idx > 0; --Idx)
							{
								const uint8 R = *Src++;
								const uint8 G = *Src++;
								const uint8 B = *Src++;
								*Dst++ = IM_COL32(R, G, B, 255);
							}
							break;
						case ETextureFormat::RGBA8:
							for (int32 Idx = RT->SizeX * RT->SizeY; Idx > 0; --Idx)
							{
								const uint8 R = *Src++;
								const uint8 G = *Src++;
								const uint8 B = *Src++;
								*Dst++ = IM_COL32(R, G, B, *Src++);
							}
							break;
						}
					}

					constexpr uint32 SrcBpp = sizeof(uint32);
					const uint32 SrcPitch = RT->SizeX * SrcBpp;
					FRHITexture* Texture = RenderTargetResource->GetTexture2DRHI();
					const FUpdateTextureRegion2D Region{ 0, 0, 0, 0, uint32(RT->SizeX), uint32(RT->SizeY) };
					RHIUpdateTexture2D(
						Texture,
						0,
						Region,
						SrcPitch,
						FontAtlasTextureData.GetData());
				});
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
			UpdateTextureDataToWS(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Image.AsG8().GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
					UpdateTextureDataToWS(Handle, TextureFormat, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat == ETextureFormat::Alpha8 ? TextureFormat : ETextureFormat::Gray8, Image.GetWidth(), Image.GetHeight(), Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
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
				UpdateTextureDataToWS(Handle, TextureFormat, RenderTarget2D->SizeX, RenderTarget2D->SizeY, Data.GetData());
			}
			break;
		default:
			ensure(false);
		}
	}

	FImGuiTextureHandle FindOrAddTexture(ETextureFormat TextureFormat, UTexture* Texture)
	{
		if (Texture == nullptr)
		{
			return {};
		}

		bool bCreated;
		const FImGuiTextureHandle Handle = FImGuiTextureHandle::FindOrCreateHandle(Texture, bCreated);
		if (bCreated)
		{
			if (UTexture2D* Texture2D = Cast<UTexture2D>(Texture))
			{
				UpdateTextureData(Handle, TextureFormat, Texture2D);
			}
			else if (UTextureRenderTarget2D* RT = Cast<UTextureRenderTarget2D>(Texture))
			{
				UpdateTextureData(Handle, TextureFormat, RT);
			}
			else
			{
				ensure(false);
			}
		}
		return Handle;
	}

	const UTexture* FindTexture(uint32 ImTextureId)
	{
		using namespace TextureIdManager;
		return IdTextureMap.FindRef(ImTextureId).Get();
	}
}

FImGuiTextureHandle::FImGuiTextureHandle(const UTexture* Texture)
{
	using namespace UnrealImGui::TextureIdManager;
	bool bCreated;
	ImTextureId = FindOrCreateHandle(Texture, bCreated).ImTextureId;
}

FImGuiTextureHandle FImGuiTextureHandle::MakeUnique()
{
	using namespace UnrealImGui::TextureIdManager;
	HandleIdCounter += 1;
	return { HandleIdCounter };
}

FImGuiTextureHandle FImGuiTextureHandle::FindOrCreateHandle(const UTexture* Texture, bool& bCreated)
{
	using namespace UnrealImGui::TextureIdManager;
	uint32& Id = HandleIdMap.FindOrAdd(Texture);
	bCreated = Id == 0;
	if (bCreated)
	{
		HandleIdCounter += 1;
		Id = HandleIdCounter;
		IdTextureMap.Add(Id, Texture);
	}
	return FImGuiTextureHandle{ Id };
}
