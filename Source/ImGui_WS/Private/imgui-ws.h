/*! \file imgui-ws.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <string>
#include <memory>

class ImGuiWS
{
public:
    using FTextureId = uint32;

    using THandler = TFunction<void()>;
    using TPath = FName;
    using TIdxs = TArray<int32>;
    using TGetter = TFunction<std::string_view(const TIdxs& idxs)>;

    struct FTexture
    {
        enum class Type : int32
        {
            Alpha8 = 0,
            Gray8  = 1,
            RGB24  = 2,
            RGBA32 = 3,
        };

        int32 Revision = 0;
        TArray<uint8> Data;
    };

    struct FEvent
    {
        enum EType : int32
        {
            Unknown = 0,
            Connected = 1,
            Disconnected = 2,
            MouseMove = 3,
            MouseDown = 4,
            MouseUp = 5,
            MouseWheel = 6,
            KeyPress = 7,
            KeyDown = 8,
            KeyUp = 9,
            Resize = 10,
            TakeControl = 11,
            PasteClipboard = 12,
            InputText = 13,
        };

        EType Type = Unknown;

        int32 ClientId = -1;

        float MouseX = 0.0f;
        float MouseY = 0.0f;

        float WheelX = 0.0f;
        float WheelY = 0.0f;

        int32 MouseBtn = 0;

        int32 Key = 0;

        int32 ClientWidth = 1920;
        int32 ClientHeight = 1080;

        uint32 Ip;

        std::string ClipboardText;
        std::string InputtedText;
    };

    ImGuiWS();
    ~ImGuiWS();

    bool Init(int32 PortListen, const FString& PathOnDisk);
    bool Init(int32 PortListen, const FString& PathOnDisk, THandler&& ConnectHandler, THandler&& DisconnectHandler);
    void Tick();
    bool SetTexture(FTextureId TextureId, FTexture::Type TextureType, int32 Width, int32 Height, const uint8* Data);
    bool SetDrawData(const struct ImDrawData* DrawData);
    struct FDrawInfo
    {
        int32 MouseCursor = 0;
        int32 ControlId;
        uint32 ControlIp;
        float MousePosX;
        float MousePosY;
        float ViewportSizeX;
        float ViewportSizeY;
        uint8 bWantTextInput;
    };
    bool SetDrawInfo(FDrawInfo&& DrawInfo);
    void AddVar(const TPath& Path, TGetter&& Getter);
    void AddServerEvent(int32 ClientId, int32 EventId, TArray<uint8>&& Payload);

    int32 NumConnected() const;

    TQueue<FEvent>& TakeEvents();
private:
    struct FImpl;
    TUniquePtr<FImpl> Impl;
};
