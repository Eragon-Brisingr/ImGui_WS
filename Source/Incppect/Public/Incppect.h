/*! \file Incppect.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

class INCPPECT_API FIncppect
{
public:
    enum EventType
    {
        Connect,
        Disconnect,
        Custom,
    };

    using TUrl = FName;
    using TPath = FName;
    using TIdxs = TArray<int32>;
    using TGetter = TFunction<std::string_view(const TIdxs& /*idxs*/)>;
    using THandler = TFunction<void(int32 /*ClientId*/, EventType /*EventType*/, TArrayView<const uint8>)>;

    // service parameters
    struct FParameters
    {
        uint32 PortListen = 3000;
        int64 tLastRequestTimeout_ms = 3000;
        uint32 tIdleTimeout_s = 120;

        FString HttpRoot = ".";
        FString PathOnDisk;
    };

    FIncppect();
    ~FIncppect();

    // run the incppect service main loop in the current thread
    // blocking call
    void Init(const FParameters& Parameters);

    void Tick();

    // terminate the server instance
    void Stop();

    // number of connected clients
    int32 NumConnected() const;

    // define variable/memory to inspect
    //
    // examples:
    //
    //   Var("path0", [](auto ) { ... });
    //   Var("path1[%d]", [](auto idxs) { ... idxs[0] ... });
    //   Var("path2[%d].foo[%d]", [](auto idxs) { ... idxs[0], idxs[1] ... });
    //
    void Var(const TPath& Path, TGetter&& Getter);
    // direct send event to server
    void ServerEvent(int32 ClientId, int32 EventId, TArray<uint8>&& Payload);

    // handle input from the clients
    void SetHandler(THandler && handler);

    // shorthand for string_view from var
    template <typename T>
        static std::string_view view(T& v) {
            if constexpr (std::is_same_v<T, std::string>) {
                return std::string_view { v.data(), v.size() };
            }
            return std::string_view { (char *)(&v), sizeof(v) };
        }

    template <typename T>
        static std::string_view view(T && v) {
            static T t;
            t = std::move(v);
            return std::string_view { (char *)(&t), sizeof(t) };
        }
private:
    struct FImpl;
    TUniquePtr<FImpl> Impl;
};