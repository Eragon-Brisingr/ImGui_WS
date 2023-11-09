/*! \file imgui-ws.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <functional>
#include <vector>
#include <queue>
#include <string>
#include <memory>

class ImGuiWS {
    public:
        using TextureId = uint32_t;

        using THandler = std::function<void()>;
        using TPath = std::string;
        using TIdxs = std::vector<int>;
        using TGetter = std::function<std::string_view(const TIdxs & idxs)>;

        struct Texture {
            enum class Type : int32_t {
                Alpha8 = 0,
                Gray8  = 1,
                RGB24  = 2,
                RGBA32 = 3,
            };

            int revision = 0;
            std::vector<char> data;
        };

        struct Event {
            enum Type : int32 {
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

            Type type = Unknown;

            int32_t clientId = -1;

            float mouse_x = 0.0f;
            float mouse_y = 0.0f;

            float wheel_x = 0.0f;
            float wheel_y = 0.0f;

            int32_t mouse_but = 0;

            int32_t key = 0;

            int32_t client_width = 1920;
            int32_t client_height = 1080;

            uint32_t ip;
            
            std::string clipboard_text;
            std::string input_text;
        };

        ImGuiWS();
        ~ImGuiWS();

        bool init(int32_t port, std::string pathHttp, std::vector<std::string> resources, const std::function<void()>& preMainLoop);
        bool init(int32_t port, std::string pathHttp, std::vector<std::string> resources, THandler && connect_handler, THandler && disconnect_handler, const std::function<void()>& preMainLoop);
        bool setTexture(TextureId textureId, Texture::Type textureType, int32_t width, int32_t height, const char * data);
        bool setDrawData(const struct ImDrawData* drawData);
        struct DrawInfo
        {
            int32_t mouseCursor = 0;
            std::string clipboardText;
            int32_t controlId;
            uint32_t controlIp;
            float mousePosX;
            float mousePosY;
            float viewportSizeX;
            float viewportSizeY;
            uint8 bWantTextInput;
        };
        bool setDrawInfo(DrawInfo&& drawInfo);
        bool addVar(const TPath & path, TGetter && getter);


        int32_t nConnected() const;

        std::deque<Event> takeEvents();

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
};
