/*! \file imgui-ws.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "imgui-ws.h"
#include "imgui-draw-data-compressor.h"

// #include "common.h"

#include "Incppect/incppect.h"

#include <atomic>
#include <array>
#include <map>
#include <thread>
#include <sstream>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include "imgui.h"
#include "UnrealImGui_Log.h"

// not using ssl
using incppect = Incppect<false>;

struct ImGuiWS::Impl {
    struct Events {
        std::deque<Event> data;

        std::mutex mutex;
        std::condition_variable cv;

        void push(Event && event) {
            std::lock_guard<std::mutex> lock(mutex);
            data.push_back(std::move(event));
            cv.notify_one();
        }
    };

    struct Data {
        std::map<int, TextureId> textureIdMap;
        std::map<TextureId, Texture> textures;

        ImDrawDataCompressor::Interface::DrawLists drawLists;
    };

    Impl() : compressorDrawData(new ImDrawDataCompressor::XorRlePerDrawListWithVtxOffset()) {}

    std::atomic<int32_t> nConnected = 0;

    FThread worker;
    mutable std::shared_mutex mutex;

    Data dataWrite;
    Data dataRead;

    DrawInfo drawInfo;

    Events events;

    incppect incpp;

    THandler handlerConnect;
    THandler handlerDisconnect;

    std::unique_ptr<ImDrawDataCompressor::Interface> compressorDrawData;
};

ImGuiWS::ImGuiWS() : m_impl(new Impl()) {
}

ImGuiWS::~ImGuiWS() {
    m_impl->incpp.stop();
    if (m_impl->worker.IsJoinable()) {
        m_impl->worker.Join();
    }
}

bool ImGuiWS::addVar(const TPath & path, TGetter && getter) {
    return m_impl->incpp.var(path, std::move(getter));
}

bool ImGuiWS::init(int32_t port, std::string pathHttp, std::vector<std::string> resources, const std::function<void()>& preMainLoop) {
    m_impl->incpp.var("my_id[%d]", [](const auto & idxs) {
        static int32_t id;
        id = idxs[0];
        return incppect::view(id);
    });

    // number of textures available
    m_impl->incpp.var("imgui.n_textures", [this](const auto & ) {
        std::shared_lock lock(m_impl->mutex);

        return incppect::view(m_impl->dataRead.textures.size());
    });

    // sync mouse cursor
    m_impl->incpp.var("imgui.mouse_cursor", [this](const auto& )
    {
        return incppect::view(m_impl->drawInfo.mouseCursor);
    });

    // current control_id
    m_impl->incpp.var("control_id", [this](const auto& )
   {
       return incppect::view(m_impl->drawInfo.controlId);
   });

    // current control IP
    m_impl->incpp.var("control_ip", [this](const auto& )
   {
       std::shared_lock lock(m_impl->mutex);

       return incppect::view(m_impl->drawInfo.controlIp);
   });

    // sync clipboard
    m_impl->incpp.var("imgui.clipboard", [this](const auto& )
    {
        return incppect::view(m_impl->drawInfo.clipboardText);
    });

    m_impl->incpp.var("imgui.want_input_text", [this](const auto& )
    {
        return incppect::view(m_impl->drawInfo.bWantTextInput);
    });

    // sync to uncontrol mouse position
    m_impl->incpp.var("imgui.mouse_pos", [this](const auto& )
    {
        static std::array<float, 2> mousePos;
        mousePos = { m_impl->drawInfo.mousePosX, m_impl->drawInfo.mousePosY };
        return incppect::view(mousePos);
    });
    
    // sync to uncontrol viewport size
    m_impl->incpp.var("imgui.viewport_size", [this](const auto& )
    {
        static std::array<float, 2> viewportSize;
        viewportSize = { m_impl->drawInfo.viewportSizeX, m_impl->drawInfo.viewportSizeY };
        return incppect::view(viewportSize);
    });
    
    // texture ids
    m_impl->incpp.var("imgui.texture_id[%d]", [this](const auto & idxs) {
        std::shared_lock lock(m_impl->mutex);

        if (m_impl->dataRead.textureIdMap.find(idxs[0]) == m_impl->dataRead.textureIdMap.end()) {
            return std::string_view { };
        }

        return incppect::view(m_impl->dataRead.textureIdMap[idxs[0]]);
    });

    // texture revision
    m_impl->incpp.var("imgui.texture_revision[%d]", [this](const auto & idxs) {
        std::shared_lock lock(m_impl->mutex);

        if (m_impl->dataRead.textures.find(idxs[0]) == m_impl->dataRead.textures.end()) {
            return std::string_view { };
        }

        return incppect::view(m_impl->dataRead.textures[idxs[0]].revision);
    });

    // get texture by id
    m_impl->incpp.var("imgui.texture_data[%d]", [this](const auto & idxs) {
        static std::vector<char> data;
        {
            std::shared_lock lock(m_impl->mutex);
            const auto texture_id = idxs[0];
            if (m_impl->dataRead.textures.find(texture_id) == m_impl->dataRead.textures.end()) {
                return std::string_view { 0, 0 };
            }
            data.clear();
            std::copy(m_impl->dataRead.textures[texture_id].data.data(), m_impl->dataRead.textures[texture_id].data.data() + m_impl->dataRead.textures[texture_id].data.size(),
                      std::back_inserter(data));
        }

        return std::string_view { data.data(), data.size() };
    });

    // get imgui's draw data
    m_impl->incpp.var("imgui.n_draw_lists", [this](const auto & ) {
        // std::shared_lock lock(m_impl->mutex);

        return incppect::view(m_impl->dataRead.drawLists.size());
    });

    m_impl->incpp.var("imgui.draw_list[%d]", [this](const auto & idxs) {
        static std::vector<char> data;
        {
            // std::shared_lock lock(m_impl->mutex);

            if (idxs[0] >= (int) m_impl->dataRead.drawLists.size()) {
                return std::string_view { nullptr, 0 };
            }

            data.clear();
            std::copy(m_impl->dataRead.drawLists[idxs[0]].data(),
                      m_impl->dataRead.drawLists[idxs[0]].data() + m_impl->dataRead.drawLists[idxs[0]].size(),
                      std::back_inserter(data));
        }

        return std::string_view { data.data(), data.size() };
    });

    m_impl->incpp.handler([&](int clientId, incppect::EventType etype, std::string_view data) {
        Event event;

        event.clientId = clientId;

        switch (etype) {
            case incppect::Connect:
                {
                    ++m_impl->nConnected;
                    event.type = Event::Connected;
                    event.ip = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
                    if (m_impl->handlerConnect) {
                        m_impl->handlerConnect();
                    }
                }
                break;
            case incppect::Disconnect:
                {
                    --m_impl->nConnected;
                    event.type = Event::Disconnected;
                    if (m_impl->handlerDisconnect) {
                        m_impl->handlerDisconnect();
                    }
                }
                break;
            case incppect::Custom:
                {
                    std::stringstream ss;
                    ss << data;

                    int type = -1;
                    ss >> type;

                    event.type = Event::Type{ type };
                    switch (event.type) {
                        case Event::MouseMove:
                            {
                                ss >> event.mouse_x >> event.mouse_y;
                            }
                            break;
                        case Event::MouseDown:
                            {
                                ss >> event.mouse_but >> event.mouse_x >> event.mouse_y;
                            }
                            break;
                        case Event::MouseUp:
                            {
                                ss >> event.mouse_but >> event.mouse_x >> event.mouse_y;
                            }
                            break;
                        case Event::MouseWheel:
                            {
                                ss >> event.wheel_x >> event.wheel_y;
                            }
                            break;
                        case Event::KeyPress:
                            {
                                ss >> event.key;
                            }
                            break;
                        case Event::KeyDown:
                            {
                                ss >> event.key;
                            }
                            break;
                        case Event::KeyUp:
                            {
                                ss >> event.key;
                            }
                            break;
                        case Event::Resize:
                            {
                                ss >> event.client_width >> event.client_height;
                            }
                            break;
                        case Event::TakeControl:
                            {
                                // take control
                            }
                            break;
                        case Event::PasteClipboard:
                            {
                                ss >> event.clipboard_text;
                            }
                            break;
                        case Event::InputText:
                            {
                                ss >> event.input_text;
                            }
                            break;
                        default:
                            {
                                event.type = Event::Unknown;
                                ensure(false);
                                UE_LOG(LogImGui, Warning, TEXT("Unknown input received from client: id = %d, type = %d"), clientId, type);
                            }
                            break;
                    };
                }
                break;
        };

        m_impl->events.push(std::move(event));
    });

    // start the http/websocket server
    incppect::Parameters parameters;
    parameters.portListen = port;
    parameters.maxPayloadLength_bytes = 8*1024*1024;
    parameters.tLastRequestTimeout_ms = -1;
    parameters.httpRoot = std::move(pathHttp);
    parameters.resources = std::move(resources);
    parameters.sslKey = "key.pem";
    parameters.sslCert = "cert.pem";
    parameters.preMainLoop = preMainLoop;
    m_impl->worker = FThread{ TEXT("ImGui_WS"), [incpp = &m_impl->incpp, parameters]
    {
        incpp->run(parameters);
    }, 0, TPri_Lowest};
    return m_impl->worker.IsJoinable();
}

bool ImGuiWS::setTexture(TextureId textureId, Texture::Type textureType, int32_t width, int32_t height, const char * data) {
    int bpp = 1; // bytes per pixel
    switch (textureType) {
        case Texture::Type::Alpha8: bpp = 1; break;
        case Texture::Type::Gray8:  bpp = 1; break;
        case Texture::Type::RGB24:  bpp = 3; break;
        case Texture::Type::RGBA32: bpp = 4; break;
    };

    if (m_impl->dataWrite.textures.find(textureId) == m_impl->dataWrite.textures.end()) {
        m_impl->dataWrite.textures[textureId].revision = 0;
        m_impl->dataWrite.textures[textureId].data.clear();
        m_impl->dataWrite.textureIdMap.clear();

        int idx = 0;
        for (const auto & t : m_impl->dataWrite.textures) {
            m_impl->dataWrite.textureIdMap[idx++] = t.first;
        }
    }

    m_impl->dataWrite.textures[textureId].revision++;
    m_impl->dataWrite.textures[textureId].data.resize(sizeof(TextureId) + sizeof(Texture::Type) + 3*sizeof(int32_t) + bpp*width*height);

    int revision = m_impl->dataWrite.textures[textureId].revision;

    size_t offset = 0;
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, &textureId, sizeof(textureId)); offset += sizeof(textureId);
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, &textureType, sizeof(textureType)); offset += sizeof(textureType);
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, &width, sizeof(width)); offset += sizeof(width);
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, &height, sizeof(height)); offset += sizeof(height);
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, &revision, sizeof(revision)); offset += sizeof(revision);
    std::memcpy(m_impl->dataWrite.textures[textureId].data.data() + offset, data, bpp*width*height);

    {
        std::unique_lock lock(m_impl->mutex);
        m_impl->dataRead.textures[textureId] = m_impl->dataWrite.textures[textureId];
        m_impl->dataRead.textureIdMap = m_impl->dataWrite.textureIdMap;
    }

    return true;
}

bool ImGuiWS::init(int32_t port, std::string pathHttp, std::vector<std::string> resources, THandler && handlerConnect, THandler && handlerDisconnect, const std::function<void()>& preMainLoop) {
    m_impl->handlerConnect = std::move(handlerConnect);
    m_impl->handlerDisconnect = std::move(handlerDisconnect);

    return init(port, std::move(pathHttp), std::move(resources), preMainLoop);
}

bool ImGuiWS::setDrawData(const ImDrawData* drawData) {
    bool result = true;

    result &= m_impl->compressorDrawData->setDrawData(drawData);

    auto& drawLists = m_impl->compressorDrawData->getDrawLists();

    // make the draw lists available to incppect clients
    {
        // std::unique_lock lock(m_impl->mutex);

        m_impl->dataRead.drawLists = std::move(drawLists);
    }

    return result;
}

bool ImGuiWS::setDrawInfo(DrawInfo&& drawInfo) {
    m_impl->drawInfo = std::move(drawInfo);
    return true;
}

int32_t ImGuiWS::nConnected() const {
    return m_impl->nConnected;
}

std::deque<ImGuiWS::Event> ImGuiWS::takeEvents() {
    std::lock_guard<std::mutex> lock(m_impl->events.mutex);
    auto res = std::move(m_impl->events.data);
    return res;
}
