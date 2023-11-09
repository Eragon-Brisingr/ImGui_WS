#include "incppect.h"

#include <algorithm>
#include <chrono>

#include "INetworkingWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "IWebSocketServer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIncppect, Log, All);
DEFINE_LOG_CATEGORY(LogIncppect);
DECLARE_STATS_GROUP(TEXT("Incppect"), STATGROUP_Incppect, STATCAT_Advanced);

namespace {
    inline int64 timestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}

struct FIncppect::Impl {
    using FIpAddress = uint8[4];

    struct Request {
        int64 tLastUpdated_ms = -1;
        int64 tLastRequested_ms = -1;
        int64 tMinUpdate_ms = 16;
        int64 tLastRequestTimeout_ms = 3000;

        TIdxs idxs;
        int32 getterId = -1;

        std::vector<char> prevData;
        std::vector<char> diffData;
        std::string_view curData;
    };

    struct FClientData {
        int64 tConnected_ms = -1;

        FIpAddress IpAddress;

        TArray<int32> lastRequests;
        TMap<int32, Request> requests;

        std::vector<uint8> curBuffer;
        std::vector<uint8> prevBuffer;
        std::vector<uint8> diffBuffer;
    };

    struct FPerSocketData {
        int32 clientId = 0;

        INetworkingWebSocket* Socket;
    };

    TUniquePtr<IWebSocketServer> Server;

    inline bool hasExt(std::string_view file, std::string_view ext)
    {
        if (ext.size() > file.size())
        {
            return false;
        }
        return std::equal(ext.rbegin(), ext.rend(), file.rbegin());
    }

    void Init()
    {
        {
            UE_LOG(LogIncppect, Log, TEXT("running instance. serving HTTPS from '%s'"), *Parameters.HttpRoot);
        }

        Server = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

        TArray<FWebSocketHttpMount> Mounts;
        {
            FWebSocketHttpMount& Mount = Mounts.AddDefaulted_GetRef();
            Mount.SetWebPath(Parameters.HttpRoot);
            Mount.SetPathOnDisk(Parameters.PathOnDisk);
            // Mount.SetDefaultFile("index.html");
        }
        Server->EnableHTTPServer(Mounts);
        Server->SetFilterConnectionCallback(FWebSocketFilterConnectionCallback::CreateLambda([](FString OriginHeader, FString ClientIP)
        {
            return EWebsocketConnectionFilterResult::ConnectionAccepted;
        }));

        if (!Server)
        {
            return;
        }
        const bool bSucceed = Server->Init(Parameters.PortListen, FWebSocketClientConnectedCallBack::CreateLambda([this](INetworkingWebSocket* Socket)
        {
            static int32 UniqueId = 0;
            ++UniqueId;

            const int32 ClientId = UniqueId;

            auto& cd = ClientData.Add(ClientId);
            cd.tConnected_ms = ::timestamp();
			int32 Port;
            const auto RemoteAddr = Socket->GetRawRemoteAddr(Port);
            cd.IpAddress[0] = RemoteAddr[0];
            cd.IpAddress[1] = RemoteAddr[1];
            cd.IpAddress[2] = RemoteAddr[2];
            cd.IpAddress[3] = RemoteAddr[3];

            SocketData.Add(ClientId, { ClientId, Socket });

            UE_LOG(LogIncppect, Log, TEXT("client with id = %d connected"), ClientId);

            if (handler)
            {
                handler(ClientId, Connect, { (const char*)Socket->GetRemoteAddr(), 4 } );
            }

            Socket->SetConnectedCallBack(FWebSocketInfoCallBack::CreateLambda([]
            {

            }));
            Socket->SetReceiveCallBack(FWebSocketPacketReceivedCallBack::CreateLambda([this, ClientId](void* RawData, int32 Size)
            {
                const char* Data = (const char*)RawData;

                rxTotal_bytes += Size;
                if (Size < sizeof(int32))
                {
                    return;
                }

                int32 type = -1;
                std::memcpy((char *)(&type), Data, sizeof(type));

                bool doUpdate = true;

                auto& cd = ClientData[ClientId];

                switch (type)
                {
                    case 1:
                        {
                            std::stringstream ss(Data + 4);
                            while (true)
                            {
                                Request request;

                                std::string RawPath;
                                ss >> RawPath;
                                const FName Path{ UTF8_TO_TCHAR(RawPath.c_str()) };
                                if (ss.eof()) break;
                                int32 requestId = 0;
                                ss >> requestId;
                                int32 nidxs = 0;
                                ss >> nidxs;
                                static const FName MyIdPath{ TEXT("my_id[%d]") };
                                if (Path == MyIdPath)
                                {
                                    for (int32 i = 0; i < nidxs; ++i)
                                    {
                                        int32 idx = 0;
                                        ss >> idx;
                                        if (idx == -1) idx = ClientId;
                                        request.idxs.Add(idx);
                                    }
                                }
                                else
                                {
                                    for (int32 i = 0; i < nidxs; ++i)
                                    {
                                        int32 idx = 0;
                                        ss >> idx;
                                        request.idxs.Add(idx);
                                    }
                                }

                                if (const int32* GetterIdx = GathToGetter.Find(Path))
                                {
                                    UE_LOG(LogIncppect, Verbose, TEXT("requestId = %d, path = '%s', nidxs = %d"), requestId, *Path.ToString(), nidxs);
                                    request.getterId = *GetterIdx;

                                    cd.requests.Emplace(requestId, request);
                                }
                                else
                                {
                                    UE_LOG(LogIncppect, Warning, TEXT("missing path '%s'"), *Path.ToString());
                                }
                            }
                        }
                        break;
                    case 2:
                        {
                            const int32 nRequests = (Size - sizeof(int32))/sizeof(int32);
                            if (nRequests*sizeof(int32) + sizeof(int32) != Size)
                            {
                                UE_LOG(LogIncppect, Error, TEXT("error : invalid message data!"));
                                return;
                            }
                            UE_LOG(LogIncppect, Verbose, TEXT("received requests: %d"), nRequests);

                            cd.lastRequests.Empty();
                            for (int32 i = 0; i < nRequests; ++i)
                            {
                                int32 curRequest = -1;
                                std::memcpy((char *)(&curRequest), Data + 4*(i + 1), sizeof(curRequest));
                                if (const auto Request = cd.requests.Find(curRequest))
                                {
                                    cd.lastRequests.Add(curRequest);
                                    Request->tLastRequested_ms = ::timestamp();
                                    Request->tLastRequestTimeout_ms = Parameters.tLastRequestTimeout_ms;
                                }
                            }
                        }
                        break;
                    case 3:
                        {
                            for (auto curRequest : cd.lastRequests)
                            {
                                if (const auto Request = cd.requests.Find(curRequest))
                                {
                                    Request->tLastRequested_ms = ::timestamp();
                                    Request->tLastRequestTimeout_ms = Parameters.tLastRequestTimeout_ms;
                                }
                            }
                        }
                        break;
                    case 4:
                        {
                            doUpdate = false;
                            if (handler && Size > sizeof(int32))
                            {
                                handler(ClientId, Custom, { Data + sizeof(int32), Size - sizeof(int32) } );
                            }
                        }
                        break;
                    default:
                        UE_LOG(LogIncppect, Warning, TEXT("unknown message type: %d"), type);
                };

                if (doUpdate)
                {
                    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Incppect_Update"), STAT_Incppect_Update, STATGROUP_Incppect);
                    Update();
                }
            }));
            Socket->SetErrorCallBack(FWebSocketInfoCallBack::CreateLambda([]
            {

            }));
            Socket->SetSocketClosedCallBack(FWebSocketInfoCallBack::CreateLambda([this, ClientId]
            {
                UE_LOG(LogIncppect, Log, TEXT("client with id = %d disconnected"), ClientId);

                ClientData.Remove(ClientId);
                SocketData.Remove(ClientId);

                if (handler)
                {
                    handler(ClientId, Disconnect, { nullptr, 0 } );
                }
            }));
        }));
        if (bSucceed)
        {
            UE_LOG(LogIncppect, Log, TEXT("listening on port %d"), Parameters.PortListen);
            UE_LOG(LogIncppect, Log, TEXT("https://localhost:%d/"), Parameters.PortListen);
        }
    }

    void Update()
    {
        for (auto& [clientId, cd] : ClientData)
        {
            /*if (socketData[clientId]->Socket->getBufferedAmount())
            {
                UE_LOG(LogIncppect, Warning, TEXT("warning: buffered amount = %d, not sending updates to client %d. waiting for buffer to drain"), socketData[clientId]->ws->getBufferedAmount(), clientId);
                continue;
            }*/

            auto& curBuffer = cd.curBuffer;
            auto& prevBuffer = cd.prevBuffer;
            auto& diffBuffer = cd.diffBuffer;

            curBuffer.clear();

            {
                uint32 typeAll = 0;
                std::copy((char *)(&typeAll), (char *)(&typeAll) + sizeof(typeAll), std::back_inserter(curBuffer));
            }

            for (auto& [requestId, req] : cd.requests)
            {
                DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Incppect_Getter"), STAT_Incppect_Getter, STATGROUP_Incppect);

                auto& getter = Getters[req.getterId];
                auto tCur = ::timestamp();
                if (((req.tLastRequestTimeout_ms < 0 && req.tLastRequested_ms > 0) || (tCur - req.tLastRequested_ms < req.tLastRequestTimeout_ms)) &&
                    tCur - req.tLastUpdated_ms > req.tMinUpdate_ms)
                {
                    if (req.tLastRequestTimeout_ms < 0)
                    {
                        req.tLastRequested_ms = 0;
                    }

                    req.curData = getter(req.idxs);
                    req.tLastUpdated_ms = tCur;

                    const int32 kPadding = 4;

                    int32 dataSize_bytes = req.curData.size();
                    int32 padding_bytes = 0;
                    {
                        int32 r = dataSize_bytes%kPadding;
                        while (r > 0 && r < kPadding)
                        {
                            ++dataSize_bytes;
                            ++padding_bytes;
                            ++r;
                        }
                    }

                    int32 type = 0; // full update
                    if (req.prevData.size() == req.curData.size() + padding_bytes && req.curData.size() > 256)
                    {
                        type = 1; // run-length encoding of diff
                    }

                    std::copy((char *)(&requestId), (char *)(&requestId) + sizeof(requestId), std::back_inserter(curBuffer));
                    std::copy((char *)(&type), (char *)(&type) + sizeof(type), std::back_inserter(curBuffer));

                    if (type == 0)
                    {
                        std::copy((char *)(&dataSize_bytes), (char *)(&dataSize_bytes) + sizeof(dataSize_bytes), std::back_inserter(curBuffer));
                        std::copy(req.curData.begin(), req.curData.end(), std::back_inserter(curBuffer));
                        {
                            char v = 0;
                            for (int32 i = 0; i < padding_bytes; ++i)
                            {
                                std::copy((char *)(&v), (char *)(&v) + sizeof(v), std::back_inserter(curBuffer));
                            }
                        }
                    }
                    else if (type == 1)
                    {
                        uint32 a = 0;
                        uint32 b = 0;
                        uint32 c = 0;
                        uint32 n = 0;
                        req.diffData.clear();

                        for (int32 i = 0; i < (int32) req.curData.size(); i += 4)
                        {
                            std::memcpy((char *)(&a), req.prevData.data() + i, sizeof(uint32));
                            std::memcpy((char *)(&b), req.curData.data() + i, sizeof(uint32));
                            a = a ^ b;
                            if (a == c)
                            {
                                ++n;
                            }
                            else
                            {
                                if (n > 0)
                                {
                                    std::copy((char *)(&n), (char *)(&n) + sizeof(uint32), std::back_inserter(req.diffData));
                                    std::copy((char *)(&c), (char *)(&c) + sizeof(uint32), std::back_inserter(req.diffData));
                                }
                                n = 1;
                                c = a;
                            }
                        }

                        if (req.curData.size() % 4 != 0)
                        {
                            a = 0;
                            b = 0;
                            uint32 i = (req.curData.size()/4)*4;
                            uint32 k = req.curData.size() - i;
                            std::memcpy((char *)(&a), req.prevData.data() + i, k);
                            std::memcpy((char *)(&b), req.curData.data() + i, k);
                            a = a ^ b;
                            if (a == c)
                            {
                                ++n;
                            }
                            else
                            {
                                std::copy((char *)(&n), (char *)(&n) + sizeof(uint32), std::back_inserter(req.diffData));
                                std::copy((char *)(&c), (char *)(&c) + sizeof(uint32), std::back_inserter(req.diffData));
                                n = 1;
                                c = a;
                            }
                        }

                        std::copy((char *)(&n), (char *)(&n) + sizeof(uint32), std::back_inserter(req.diffData));
                        std::copy((char *)(&c), (char *)(&c) + sizeof(uint32), std::back_inserter(req.diffData));

                        dataSize_bytes = req.diffData.size();
                        std::copy((char *)(&dataSize_bytes), (char *)(&dataSize_bytes) + sizeof(dataSize_bytes), std::back_inserter(curBuffer));
                        std::copy(req.diffData.begin(), req.diffData.end(), std::back_inserter(curBuffer));
                    }

                    req.prevData.resize(req.curData.size());
                    std::copy(req.curData.begin(), req.curData.end(), req.prevData.begin());
                }
            }

            if (curBuffer.size() > 4)
            {
                DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Incppect_Diff"), STAT_Incppect_Diff, STATGROUP_Incppect);

                if (curBuffer.size() == prevBuffer.size() && curBuffer.size() > 256)
                {
                    uint32 a = 0;
                    uint32 b = 0;
                    uint32 c = 0;
                    uint32 n = 0;
                    diffBuffer.clear();

                    uint32 typeAll = 1;
                    std::copy((char *)(&typeAll), (char *)(&typeAll) + sizeof(typeAll), std::back_inserter(diffBuffer));

                    for (int32 i = 4; i < (int32) curBuffer.size(); i += 4)
                    {
                        std::memcpy((char *)(&a), prevBuffer.data() + i, sizeof(uint32));
                        std::memcpy((char *)(&b), curBuffer.data() + i, sizeof(uint32));
                        a = a ^ b;
                        if (a == c)
                        {
                            ++n;
                        }
                        else
                        {
                            if (n > 0)
                            {
                                std::copy((char *)(&n), (char *)(&n) + sizeof(uint32), std::back_inserter(diffBuffer));
                                std::copy((char *)(&c), (char *)(&c) + sizeof(uint32), std::back_inserter(diffBuffer));
                            }
                            n = 1;
                            c = a;
                        }
                    }

                    std::copy((char *)(&n), (char *)(&n) + sizeof(uint32), std::back_inserter(diffBuffer));
                    std::copy((char *)(&c), (char *)(&c) + sizeof(uint32), std::back_inserter(diffBuffer));

                    if (SocketData[clientId].Socket->Send(diffBuffer.data(), diffBuffer.size(), false) == false)
                    {
                        UE_LOG(LogIncppect, Warning, TEXT("backpressure for client %d increased"), clientId);
                    }
                }
                else
                {
                    if (SocketData[clientId].Socket->Send(curBuffer.data(), curBuffer.size(), false) == false)
                    {
                        UE_LOG(LogIncppect, Warning, TEXT("backpressure for client %d increased"), clientId);
                    }
                }

                txTotal_bytes += curBuffer.size();

                prevBuffer.resize(curBuffer.size());
                std::copy(curBuffer.begin(), curBuffer.end(), prevBuffer.begin());
            }
        }
    }

    Parameters Parameters;

    double txTotal_bytes = 0;
    double rxTotal_bytes = 0;

    TMap<TPath, int32> GathToGetter;
    TArray<TGetter> Getters;

    TMap<int32, FPerSocketData> SocketData;
    TMap<int32, FClientData> ClientData;

    TMap<TUrl, TResourceContent> Resources;

    THandler handler = nullptr;
};

FIncppect::FIncppect()
{

}

FIncppect::~FIncppect()
{

}

void FIncppect::Init(const Parameters& Parameters)
{
    m_impl = MakeUnique<Impl>();
    var("incppect.nclients", [this](const TIdxs& ) { return view(m_impl->SocketData.Num()); });
    var("incppect.tx_total", [this](const TIdxs& ) { return view(m_impl->txTotal_bytes); });
    var("incppect.rx_total", [this](const TIdxs& ) { return view(m_impl->rxTotal_bytes); });
    var("incppect.ip_address[%d]", [this](const TIdxs& idxs) {
        const auto& ClientData = m_impl->ClientData[idxs[0]];
        return view(ClientData.IpAddress);
    });

    m_impl->Parameters = Parameters;
    m_impl->Init();
}

void FIncppect::Tick()
{
    m_impl->Server->Tick();
}

void FIncppect::Stop()
{
    m_impl.Reset();
}

int32 FIncppect::nConnected() const
{
    return m_impl->SocketData.Num();
}

void FIncppect::setResource(const TUrl& url, const TResourceContent& content)
{
    m_impl->Resources.Add(url, content);
}

bool FIncppect::var(const TPath& path, TGetter && getter)
{
    m_impl->GathToGetter.Add(path, m_impl->Getters.Num());
    m_impl->Getters.Emplace(getter);

    return true;
}

void FIncppect::handler(THandler && handler)
{
    m_impl->handler = std::move(handler);
}
