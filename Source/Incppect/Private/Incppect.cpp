#include "Incppect.h"

#include <sstream>

#include "LogIncppect.h"
#include "WebSocketServer.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP (TEXT("Incppect"), STATGROUP_Incppect, STATCAT_Advanced);

namespace
{
    inline int64 TimeStamp()
    {
        return FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64());
    }
}

struct FIncppect::FImpl
{
    using FIpAddress = uint8[4];

    struct FRequest {
        int64 LastUpdatedMs = -1;
        int64 LastRequestedMs = -1;
        int64 MinUpdateMs = 16;
        int64 LastRequestTimeoutMs = 3000;

        TIdxs Idxs;
        int32 GetterId = -1;

        TArray<uint8> PrevData;
    };

    struct FClientData
    {
        int64 ConnectedMs = -1;

        FIpAddress IpAddress;

        TArray<int32> LastRequests;
        TMap<int32, FRequest> Requests;

        TArray<uint8> PrevBuffer;

        struct FToServerEvent
        {
            int32 EventId;
            TArray<uint8> Payload;
        };
        TArray<FToServerEvent> ToServerEvents;
    };

    struct FPerSocketData
    {
        int32 ClientId = 0;
        Incppect::FWebSocket* Socket;
    };

    TUniquePtr<Incppect::FWebSocketServer> Server;

    void Init()
    {
        UE_LOG(LogIncppect, Log, TEXT("running instance. serving HTTPS from '%s'"), *Parameters.HttpRoot);

        using namespace Incppect;

        Server = MakeUnique<FWebSocketServer>();
        if (!Server)
        {
            UE_LOG(LogIncppect, Warning, TEXT("failed create websocket server"));
            return;
        }

        TArray<FWebSocketHttpMount> Mounts;
        {
            FWebSocketHttpMount& Mount = Mounts.AddDefaulted_GetRef();
            Mount.SetWebPath(Parameters.HttpRoot);
            Mount.SetPathOnDisk(Parameters.PathOnDisk);
            Mount.SetDefaultFile("index.html");
        }
        Server->EnableHTTPServer(Mounts);
        Server->SetFilterConnectionCallback(FWebSocketFilterConnectionCallback::CreateLambda([](FString OriginHeader, FString ClientIP)
        {
            return EWebsocketConnectionFilterResult::ConnectionAccepted;
        }));

        const bool bSucceed = Server->Init(Parameters.PortListen, FWebSocketClientConnectedCallBack::CreateLambda([this](FWebSocket* Socket)
        {
            static int32 UniqueId = 0;
            ++UniqueId;

            const int32 ClientId = UniqueId;

            auto& ClientData = ClientDataMap.Add(ClientId);
            ClientData.ConnectedMs = ::TimeStamp();
			int32 Port;
            const auto RemoteAddr = Socket->GetRawRemoteAddr(Port);
            ClientData.IpAddress[0] = RemoteAddr[0];
            ClientData.IpAddress[1] = RemoteAddr[1];
            ClientData.IpAddress[2] = RemoteAddr[2];
            ClientData.IpAddress[3] = RemoteAddr[3];

            SocketDataMap.Add(ClientId, { ClientId, Socket });

            UE_LOG(LogIncppect, Log, TEXT("client with id = %d connected"), ClientId);

            if (Handler)
            {
                Handler(ClientId, Connect, { ClientData.IpAddress, 4 } );
            }

            Socket->SetConnectedCallBack(FWebSocketInfoCallBack::CreateLambda([]
            {

            }));
            Socket->SetReceiveCallBack(FWebSocketPacketReceivedCallBack::CreateLambda([this, ClientId](void* RawData, int32 Size)
            {
                const uint8* Data = static_cast<const uint8*>(RawData);

                RxTotalBytes += Size;
                if (Size < sizeof(int32))
                {
                    return;
                }

                int32 Type = -1;
                FMemory::Memcpy(&Type, Data, sizeof(Type));

                bool DoUpdate = true;

                auto& ClientData = ClientDataMap[ClientId];

                switch (Type)
                {
                    case 1:
                        {
                            std::stringstream ss(static_cast<const char*>(RawData) + 4);
                            while (true)
                            {
                                FRequest Request;

                                std::string RawPath;
                                ss >> RawPath;
                                const FName Path{ UTF8_TO_TCHAR(RawPath.c_str()) };
                                if (ss.eof()) break;
                                int32 RequestId = 0;
                                ss >> RequestId;
                                int32 IdxsNum = 0;
                                ss >> IdxsNum;
                                static const FName MyIdPath{ TEXT("my_id[%d]") };
                                if (Path == MyIdPath)
                                {
                                    for (int32 I = 0; I < IdxsNum; ++I)
                                    {
                                        int32 Idx = 0;
                                        ss >> Idx;
                                        if (Idx == -1) Idx = ClientId;
                                        Request.Idxs.Add(Idx);
                                    }
                                }
                                else
                                {
                                    for (int32 I = 0; I < IdxsNum; ++I)
                                    {
                                        int32 Idx = 0;
                                        ss >> Idx;
                                        Request.Idxs.Add(Idx);
                                    }
                                }

                                if (const int32* GetterIdx = PathToGetter.Find(Path))
                                {
                                    UE_LOG(LogIncppect, Verbose, TEXT("requestId = %d, path = '%s', nidxs = %d"), RequestId, *Path.ToString(), IdxsNum);
                                    Request.GetterId = *GetterIdx;

                                    ClientData.Requests.Emplace(RequestId, Request);
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
                            const int32 NumRequests = (Size - sizeof(int32))/sizeof(int32);
                            if (NumRequests*sizeof(int32) + sizeof(int32) != Size)
                            {
                                UE_LOG(LogIncppect, Error, TEXT("error : invalid message data!"));
                                return;
                            }
                            UE_LOG(LogIncppect, Verbose, TEXT("received requests: %d"), NumRequests);
                            ClientData.LastRequests.Empty();
                            for (int32 i = 0; i < NumRequests; ++i)
                            {
                                int32 CurRequest = -1;
                                FMemory::Memcpy(&CurRequest, Data + 4*(i + 1), sizeof(CurRequest));
                                if (const auto Request = ClientData.Requests.Find(CurRequest))
                                {
                                    ClientData.LastRequests.Add(CurRequest);
                                    Request->LastRequestedMs = ::TimeStamp();
                                    Request->LastRequestTimeoutMs = Parameters.tLastRequestTimeout_ms;
                                }
                            }
                        }
                        break;
                    case 3:
                        {
                            for (const auto CurRequest : ClientData.LastRequests)
                            {
                                if (const auto Request = ClientData.Requests.Find(CurRequest))
                                {
                                    Request->LastRequestedMs = ::TimeStamp();
                                    Request->LastRequestTimeoutMs = Parameters.tLastRequestTimeout_ms;
                                }
                            }
                        }
                        break;
                    case 4:
                        {
                            DoUpdate = false;
                            if (Handler && Size > sizeof(int32))
                            {
                                Handler(ClientId, Custom, { Data + sizeof(int32), static_cast<int32>(Size - sizeof(int32)) } );
                            }
                        }
                        break;
                    default:
                        UE_LOG(LogIncppect, Warning, TEXT("unknown message type: %d"), Type);
                };

                if (DoUpdate)
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

                ClientDataMap.Remove(ClientId);
                SocketDataMap.Remove(ClientId);

                if (Handler)
                {
                    Handler(ClientId, Disconnect, {} );
                }
            }));
        }));
        if (bSucceed)
        {
            UE_LOG(LogIncppect, Log, TEXT("listening on port %d"), Parameters.PortListen);
            UE_LOG(LogIncppect, Log, TEXT("http://localhost:%d/"), Parameters.PortListen);
        }
        else
        {
            UE_LOG(LogIncppect, Warning, TEXT("Falied init websocket server"));
        }
    }

    void Update()
    {
        for (auto& [ClientId, ClientData] : ClientDataMap)
        {
            TArray<uint8> CurBuffer;
            auto& PrevBuffer = ClientData.PrevBuffer;

            {
                uint32 TypeAll = 0;
                CurBuffer.Append(reinterpret_cast<uint8*>(&TypeAll), sizeof(TypeAll));
            }

            static auto GetPaddingBytes = [](int32& DataSizeBytes)
            {
                constexpr int32 kPadding = 4;

                int32 PaddingBytes = 0;
                {
                    int32 r = DataSizeBytes%kPadding;
                    while (r > 0 && r < kPadding)
                    {
                        ++DataSizeBytes;
                        ++PaddingBytes;
                        ++r;
                    }
                }
                return PaddingBytes;
            };

            for (auto& [RequestId, Req] : ClientData.Requests)
            {
                DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Incppect_Getter"), STAT_Incppect_Getter, STATGROUP_Incppect);

                auto& Getter = Getters[Req.GetterId];
                const int64 CurMS = ::TimeStamp();
                if (((Req.LastRequestTimeoutMs < 0 && Req.LastRequestedMs > 0) || (CurMS - Req.LastRequestedMs < Req.LastRequestTimeoutMs)) &&
                    CurMS - Req.LastUpdatedMs > Req.MinUpdateMs)
                {
                    if (Req.LastRequestTimeoutMs < 0)
                    {
                        Req.LastRequestedMs = 0;
                    }

                    const auto GetterData{ Getter(Req.Idxs) };
                    TArrayView<uint8> CurData{ (uint8*)GetterData.data(), (int32)GetterData.size() };
                    Req.LastUpdatedMs = CurMS;

                    int32 DataSizeBytes = CurData.Num();
                    int32 PaddingBytes = GetPaddingBytes(DataSizeBytes);

                    int32 Type = 0; // full update
                    if (Req.PrevData.Num() == CurData.Num() + PaddingBytes && CurData.Num() > 256)
                    {
                        Type = 1; // run-length encoding of diff
                    }

                    CurBuffer.Append(reinterpret_cast<uint8*>(&Type), sizeof(Type));
                    CurBuffer.Append(reinterpret_cast<uint8*>(&RequestId), sizeof(RequestId));

                    if (Type == 0)
                    {
                        CurBuffer.Append(reinterpret_cast<uint8*>(&DataSizeBytes), sizeof(DataSizeBytes));
                        CurBuffer.Append(CurData);
                        {
                            for (int32 i = 0; i < PaddingBytes; ++i)
                            {
                                CurBuffer.Add(0);
                            }
                        }
                    }
                    else if (Type == 1)
                    {
                        uint32 a = 0;
                        uint32 b = 0;
                        uint32 c = 0;
                        uint32 n = 0;
                        TArray<uint8> DiffData;
                        for (int32 Idx = 0; Idx < CurData.Num(); Idx += 4)
                        {
                            FMemory::Memcpy(&a, Req.PrevData.GetData() + Idx, sizeof(a));
                            FMemory::Memcpy(&b, CurData.GetData() + Idx, sizeof(b));
                            a = a ^ b;
                            if (a == c)
                            {
                                ++n;
                            }
                            else
                            {
                                if (n > 0)
                                {
                                    DiffData.Append(reinterpret_cast<uint8*>(&n), sizeof(n));
                                    DiffData.Append(reinterpret_cast<uint8*>(&c), sizeof(c));
                                }
                                n = 1;
                                c = a;
                            }
                        }

                        if (CurData.Num() % 4 != 0)
                        {
                            a = 0;
                            b = 0;
                            const uint32 Idx = (CurData.Num()/4)*4;
                            const uint32 k = CurData.Num() - Idx;
                            FMemory::Memcpy(&a, Req.PrevData.GetData() + Idx, k);
                            FMemory::Memcpy(&b, CurData.GetData() + Idx, k);
                            a = a ^ b;
                            if (a == c)
                            {
                                ++n;
                            }
                            else
                            {
                                DiffData.Append(reinterpret_cast<uint8*>(&n), sizeof(n));
                                DiffData.Append(reinterpret_cast<uint8*>(&c), sizeof(c));
                                n = 1;
                                c = a;
                            }
                        }

                        DiffData.Append(reinterpret_cast<uint8*>(&n), sizeof(n));
                        DiffData.Append(reinterpret_cast<uint8*>(&c), sizeof(c));

                        DataSizeBytes = DiffData.Num();
                        CurBuffer.Append(reinterpret_cast<uint8*>(&DataSizeBytes), sizeof(DataSizeBytes));
                        CurBuffer.Append(DiffData);
                    }

                    Req.PrevData = CurData;
                }
            }

            if (ClientData.ToServerEvents.Num() > 0)
            {
                for (const auto& Event : ClientData.ToServerEvents)
                {
                    int32 Type = 2; // to server event
                    CurBuffer.Append(reinterpret_cast<const uint8*>(&Type), sizeof(Type));
                    CurBuffer.Append(reinterpret_cast<const uint8*>(&Event.EventId), sizeof(Event.EventId));
                    int32 DataSizeBytes = Event.Payload.Num();
                    const int32 PaddingBytes = GetPaddingBytes(DataSizeBytes);
                    CurBuffer.Append(reinterpret_cast<uint8*>(&DataSizeBytes), sizeof(DataSizeBytes));
                    CurBuffer.Append(Event.Payload);
                    {
                        for (int32 i = 0; i < PaddingBytes; ++i)
                        {
                            CurBuffer.Add(0);
                        }
                    }
                }
                ClientData.ToServerEvents.Empty();
            }

            if (CurBuffer.Num() > 4)
            {
                DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Incppect_Diff"), STAT_Incppect_Diff, STATGROUP_Incppect);

                if (CurBuffer.Num() == PrevBuffer.Num() && CurBuffer.Num() > 256)
                {
                    TArray<uint8> DiffBuffer;
                    uint32 a = 0;
                    uint32 b = 0;
                    uint32 c = 0;
                    uint32 n = 0;

                    uint32 TypeAll = 1;
                    DiffBuffer.Append(reinterpret_cast<uint8*>(&TypeAll), sizeof(TypeAll));

                    for (int32 Idx = 4; Idx < (int32) CurBuffer.Num(); Idx += 4)
                    {
                        FMemory::Memcpy(&a, PrevBuffer.GetData() + Idx, sizeof(a));
                        FMemory::Memcpy(&b, CurBuffer.GetData() + Idx, sizeof(b));
                        a = a ^ b;
                        if (a == c)
                        {
                            ++n;
                        }
                        else
                        {
                            if (n > 0)
                            {
                                DiffBuffer.Append(reinterpret_cast<uint8*>(&n), sizeof(n));
                                DiffBuffer.Append(reinterpret_cast<uint8*>(&c), sizeof(c));
                            }
                            n = 1;
                            c = a;
                        }
                    }

                    DiffBuffer.Append(reinterpret_cast<uint8*>(&n), sizeof(n));
                    DiffBuffer.Append(reinterpret_cast<uint8*>(&c), sizeof(c));

                    if (SocketDataMap[ClientId].Socket->Send(DiffBuffer.GetData(), DiffBuffer.Num(), false) == false)
                    {
                        UE_LOG(LogIncppect, Warning, TEXT("backpressure for client %d increased"), ClientId);
                    }
                }
                else
                {
                    if (SocketDataMap[ClientId].Socket->Send(CurBuffer.GetData(), CurBuffer.Num(), false) == false)
                    {
                        UE_LOG(LogIncppect, Warning, TEXT("backpressure for client %d increased"), ClientId);
                    }
                }

                TxTotalBytes += CurBuffer.Num();

                PrevBuffer = MoveTemp(CurBuffer);
            }
        }
    }

    FParameters Parameters;

    double TxTotalBytes = 0;
    double RxTotalBytes = 0;

    TMap<TPath, int32> PathToGetter;
    TArray<TGetter> Getters;

    TMap<int32, FPerSocketData> SocketDataMap;
    TMap<int32, FClientData> ClientDataMap;

    THandler Handler = nullptr;
};

FIncppect::FIncppect()
{

}

FIncppect::~FIncppect()
{

}

void FIncppect::Init(const FParameters& Parameters)
{
    Impl = MakeUnique<FImpl>();
    Var(TEXT("incppect.nclients"), [this](const TIdxs& ) { return view(Impl->SocketDataMap.Num()); });
    Var(TEXT("incppect.tx_total"), [this](const TIdxs& ) { return view(Impl->TxTotalBytes); });
    Var(TEXT("incppect.rx_total"), [this](const TIdxs& ) { return view(Impl->RxTotalBytes); });
    Var(TEXT("incppect.ip_address[%d]"), [this](const TIdxs& idxs)
    {
        const auto& ClientData = Impl->ClientDataMap[idxs[0]];
        return view(ClientData.IpAddress);
    });

    Impl->Parameters = Parameters;
    Impl->Init();
}

void FIncppect::Tick()
{
    Impl->Server->Tick();
}

void FIncppect::Stop()
{
    Impl.Reset();
}

int32 FIncppect::NumConnected() const
{
    return Impl->SocketDataMap.Num();
}

void FIncppect::Var(const TPath& Path, TGetter&& Getter)
{
    Impl->PathToGetter.Add(Path, Impl->Getters.Num());
    Impl->Getters.Emplace(Getter);
}

void FIncppect::ServerEvent(int32 ClientId, int32 EventId, TArray<uint8>&& Payload)
{
    if (const auto ClientData = Impl->ClientDataMap.Find(ClientId))
    {
        ClientData->ToServerEvents.Add({ EventId, MoveTemp(Payload) });
    }
}

void FIncppect::SetHandler(THandler && handler)
{
    Impl->Handler = MoveTemp(handler);
}
