/*! \file imgui-ws.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "imgui-ws.h"
#include "imgui-draw-data-compressor.h"

// #include "common.h"

#include <atomic>
#include <sstream>
#include <cstring>
#include <shared_mutex>

#include "imgui.h"
#include "Incppect.h"
#include "UnrealImGui_Log.h"

struct ImGuiWS::FImpl
{
    struct FData
    {
        TMap<int32, FTextureId> TextureIdMap;
        TMap<FTextureId, FTexture> Textures;
    };

    FImpl()
        : DrawInfo()
        , CompressorDrawData(new ImDrawDataCompressor::XorRlePerDrawListWithVtxOffset())
    {
    }

    std::atomic<int32> NumConnected = 0;

    TMap<int32, FTextureId> TextureIdMap;
    TMap<FTextureId, FTexture> Textures;

    ImDrawDataCompressor::Interface::DrawLists DrawLists;
    FDrawInfo DrawInfo;

    TQueue<FEvent> Events;

    FIncppect Incpp;

    THandler HandlerConnect;
    THandler HandlerDisconnect;

    std::unique_ptr<ImDrawDataCompressor::Interface> CompressorDrawData;

    using FAsyncTask = TFunction<void(FImpl&)>;
    TQueue<FAsyncTask> AsyncTasks;
};

ImGuiWS::ImGuiWS()
    : Impl(new FImpl())
{}

ImGuiWS::~ImGuiWS()
{
    Impl->Incpp.Stop();
}

void ImGuiWS::AddVar(const TPath& Path, TGetter&& Getter)
{
    Impl->Incpp.Var(Path, std::move(Getter));
}

void ImGuiWS::AddServerEvent(int32 ClientId, int32 EventId, TArray<uint8>&& Payload)
{
    Impl->AsyncTasks.Enqueue([ClientId, EventId, Payload = MoveTemp(Payload)](FImpl& ImplRef) mutable
    {
        ImplRef.Incpp.ServerEvent(ClientId, EventId, MoveTemp(Payload));
    });
}

bool ImGuiWS::Init(int32 PortListen, const FString& PathOnDisk)
{
    // start the http/websocket server
    FIncppect::FParameters Parameters;
    Parameters.PortListen = PortListen;
    Parameters.tLastRequestTimeout_ms = -1;
    Parameters.HttpRoot = TEXT("/");
    Parameters.PathOnDisk = PathOnDisk;
    Impl->Incpp.Init(Parameters);

    Impl->Incpp.Var(TEXT("my_id[%d]"), [](const auto& idxs)
    {
        static int32 id;
        id = idxs[0];
        return FIncppect::view(id);
    });

    // number of textures available
    Impl->Incpp.Var(TEXT("imgui.n_textures"), [this](const auto& )
    {
        return FIncppect::view(Impl->Textures.Num());
    });

    // sync mouse cursor
    Impl->Incpp.Var(TEXT("imgui.mouse_cursor"), [this](const auto& )
    {
        return FIncppect::view(Impl->DrawInfo.MouseCursor);
    });

    // current control_id
    Impl->Incpp.Var(TEXT("control_id"), [this](const auto& )
    {
       return FIncppect::view(Impl->DrawInfo.ControlId);
    });

    // current control IP
    Impl->Incpp.Var(TEXT("control_ip"), [this](const auto& )
    {
       return FIncppect::view(Impl->DrawInfo.ControlIp);
    });

    Impl->Incpp.Var(TEXT("imgui.want_input_text"), [this](const auto& )
    {
        return FIncppect::view(Impl->DrawInfo.bWantTextInput);
    });

    // sync to uncontrol mouse position
    Impl->Incpp.Var(TEXT("imgui.mouse_pos"), [this](const auto& )
    {
        static FVector2f MousePos;
        MousePos = { Impl->DrawInfo.MousePosX, Impl->DrawInfo.MousePosY };
        return FIncppect::view(MousePos);
    });

    // sync to uncontrol viewport size
    Impl->Incpp.Var(TEXT("imgui.viewport_size"), [this](const auto& )
    {
        static FVector2f ViewportSize;
        ViewportSize = { Impl->DrawInfo.ViewportSizeX, Impl->DrawInfo.ViewportSizeY };
        return FIncppect::view(ViewportSize);
    });

    // texture ids
    Impl->Incpp.Var(TEXT("imgui.texture_id[%d]"), [this](const auto& idxs)
    {
        if (const FTextureId* Idx = Impl->TextureIdMap.Find(idxs[0]))
        {
            return FIncppect::view(*Idx);
        }
        return std::string_view{ };
    });

    // texture revision
    Impl->Incpp.Var(TEXT("imgui.texture_revision[%d]"), [this](const auto& idxs)
    {
        if (const FTexture* Texture = Impl->Textures.Find(idxs[0]))
        {
            return FIncppect::view(Texture->Revision);
        }
        return std::string_view { };
    });

    // get texture by id
    Impl->Incpp.Var(TEXT("imgui.texture_data[%d]"), [this](const auto& idxs)
    {
        const auto TextureId = idxs[0];
        if (const FTexture* Texture = Impl->Textures.Find(TextureId))
        {
            static TArray<uint8> Data;
            Data = Texture->Data;
            return std::string_view { reinterpret_cast<char*>(Data.GetData()), (uint32)Data.Num() };
        }
        return std::string_view { nullptr, 0 };
    });

    // get imgui's draw data
    Impl->Incpp.Var(TEXT("imgui.n_draw_lists"), [this](const auto& )
    {
        return FIncppect::view(Impl->DrawLists.size());
    });

    Impl->Incpp.Var(TEXT("imgui.draw_list[%d]"), [this](const auto& idxs)
    {
        static std::vector<char> data;
        {
            if (idxs[0] >= (int32) Impl->DrawLists.size())
            {
                return std::string_view { nullptr, 0 };
            }

            data.clear();
            std::copy(Impl->DrawLists[idxs[0]].data(),
                      Impl->DrawLists[idxs[0]].data() + Impl->DrawLists[idxs[0]].size(),
                      std::back_inserter(data));
        }

        return std::string_view { data.data(), data.size() };
    });

    Impl->Incpp.SetHandler([&](int32 ClientId, FIncppect::EventType EventType, TArrayView<const uint8> Data)
    {
        FEvent Event;

        Event.ClientId = ClientId;

        switch (EventType)
        {
            case FIncppect::Connect:
                {
                    Impl->NumConnected += 1;
                    Event.Type = FEvent::Connected;
                    Event.Ip = Data[0] + (Data[1] << 8) + (Data[2] << 16) + (Data[3] << 24);
                    if (Impl->HandlerConnect)
                    {
                        Impl->HandlerConnect();
                    }
                }
                break;
            case FIncppect::Disconnect:
                {
                    Impl->NumConnected -= 1;
                    Event.Type = FEvent::Disconnected;
                    if (Impl->HandlerDisconnect)
                    {
                        Impl->HandlerDisconnect();
                    }
                }
                break;
            case FIncppect::Custom:
                {
                    std::stringstream ss;
                    ss << reinterpret_cast<const char*>(Data.GetData());

                    int32 Type = -1;
                    ss >> Type;

                    Event.Type = FEvent::EType{ Type };
                    switch (Event.Type)
                    {
                        case FEvent::MouseMove:
                            {
                                ss >> Event.MouseX >> Event.MouseY;
                            }
                            break;
                        case FEvent::MouseDown:
                            {
                                ss >> Event.MouseBtn >> Event.MouseX >> Event.MouseY;
                            }
                            break;
                        case FEvent::MouseUp:
                            {
                                ss >> Event.MouseBtn >> Event.MouseX >> Event.MouseY;
                            }
                            break;
                        case FEvent::MouseWheel:
                            {
                                ss >> Event.WheelX >> Event.WheelY;
                            }
                            break;
                        case FEvent::KeyPress:
                            {
                                ss >> Event.Key;
                            }
                            break;
                        case FEvent::KeyDown:
                            {
                                ss >> Event.Key;
                            }
                            break;
                        case FEvent::KeyUp:
                            {
                                ss >> Event.Key;
                            }
                            break;
                        case FEvent::Resize:
                            {
                                ss >> Event.ClientWidth >> Event.ClientHeight;
                            }
                            break;
                        case FEvent::TakeControl:
                            {
                                // take control
                            }
                            break;
                        case FEvent::PasteClipboard:
                            {
                                ss >> Event.ClipboardText;
                            }
                            break;
                        case FEvent::InputText:
                            {
                                ss >> Event.InputtedText;
                            }
                            break;
                        default:
                            {
                                Event.Type = FEvent::Unknown;
                                ensure(false);
                                UE_LOG(LogImGui, Warning, TEXT("Unknown input received from client: id = %d, type = %d"), ClientId, Type);
                            }
                            break;
                    }
                }
                break;
        }

        Impl->Events.Enqueue(MoveTemp(Event));
    });

    return true;
}

bool ImGuiWS::Init(int32 PortListen, const FString& PathOnDisk, THandler&& HandlerConnect, THandler&& HandlerDisconnect)
{
    Impl->HandlerConnect = MoveTemp(HandlerConnect);
    Impl->HandlerDisconnect = MoveTemp(HandlerDisconnect);

    return Init(PortListen, PathOnDisk);
}

void ImGuiWS::Tick()
{
    while (Impl->AsyncTasks.IsEmpty() == false)
    {
        FImpl::FAsyncTask Task;
        Impl->AsyncTasks.Dequeue(Task);
        Task(*Impl);
    }
    Impl->Incpp.Tick();
}

bool ImGuiWS::SetTexture(FTextureId TextureId, FTexture::Type TextureType, int32 Width, int32 Height, const uint8* Data)
{
    int32 bpp = 1; // bytes per pixel
    switch (TextureType)
    {
        case FTexture::Type::Alpha8: bpp = 1; break;
        case FTexture::Type::Gray8:  bpp = 1; break;
        case FTexture::Type::RGB24:  bpp = 3; break;
        case FTexture::Type::RGBA32: bpp = 4; break;
    }
    TArray<uint8> TextureData;
    TextureData.SetNumUninitialized(sizeof(FTextureId) + sizeof(FTexture::Type) + 3*sizeof(int32) + bpp*Width*Height);

    size_t Offset = 0;
    FMemory::Memcpy(TextureData.GetData() + Offset, &TextureId, sizeof(TextureId)); Offset += sizeof(TextureId);
    FMemory::Memcpy(TextureData.GetData() + Offset, &TextureType, sizeof(TextureType)); Offset += sizeof(TextureType);
    FMemory::Memcpy(TextureData.GetData() + Offset, &Width, sizeof(Width)); Offset += sizeof(Width);
    FMemory::Memcpy(TextureData.GetData() + Offset, &Height, sizeof(Height)); Offset += sizeof(Height);
    const int32 RevisionOffset = Offset; Offset += sizeof(int32);
    FMemory::Memcpy(TextureData.GetData() + Offset, Data, bpp*Width*Height);

    Impl->AsyncTasks.Enqueue([TextureId, TextureData = MoveTemp(TextureData), RevisionOffset](FImpl& ImplRef) mutable
    {
        FTexture& Texture = ImplRef.Textures.FindOrAdd(TextureId);
        if (Texture.Revision == 0)
        {
            ImplRef.TextureIdMap.Empty();
            int32 Idx = 0;
            for (const auto& [Id, _] : ImplRef.Textures)
            {
                ImplRef.TextureIdMap.Add(Idx, Id);
                Idx += 1;
            }
        }
        Texture.Revision++;
        const int32 Revision = Texture.Revision;

        Texture.Data = MoveTemp(TextureData);
        FMemory::Memcpy(Texture.Data.GetData() + RevisionOffset, &Revision, sizeof(Revision));
    });

    return true;
}

bool ImGuiWS::SetDrawData(const ImDrawData* DrawData)
{
    bool Result = true;

    Result &= Impl->CompressorDrawData->setDrawData(DrawData);

    auto& DrawLists = Impl->CompressorDrawData->getDrawLists();

    // make the draw lists available to incppect clients
    Impl->DrawLists = MoveTemp(DrawLists);

    return Result;
}

bool ImGuiWS::SetDrawInfo(FDrawInfo&& DrawInfo)
{
    Impl->DrawInfo = MoveTemp(DrawInfo);
    return true;
}

int32 ImGuiWS::NumConnected() const
{
    return Impl->NumConnected;
}

TQueue<ImGuiWS::FEvent>& ImGuiWS::TakeEvents()
{
    return Impl->Events;
}
