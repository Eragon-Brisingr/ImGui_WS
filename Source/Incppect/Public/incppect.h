/*! \file incppect.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

class INCPPECT_API FIncppect
{
    public:
        enum EventType {
            Connect,
            Disconnect,
            Custom,
        };

        using TUrl = FName;
        using TResourceContent = std::string;
        using TPath = FName;
        using TIdxs = TArray<int32>;
        using TGetter = TFunction<std::string_view(const TIdxs& idxs)>;
        using THandler = TFunction<void(int32 clientId, EventType etype, std::string_view)>;

        // service parameters
        struct Parameters {
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
        void Init(const Parameters& Parameters);

        void Tick();

        // terminate the server instance
        void Stop();

        // set a resource. useful for serving html/js files from within the application
        void setResource(const TUrl& url, const TResourceContent& content);

        // number of connected clients
        int32 nConnected() const;

        // define variable/memory to inspect
        //
        // examples:
        //
        //   var("path0", [](auto ) { ... });
        //   var("path1[%d]", [](auto idxs) { ... idxs[0] ... });
        //   var("path2[%d].foo[%d]", [](auto idxs) { ... idxs[0], idxs[1] ... });
        //
        bool var(const TPath& path, TGetter && getter);

        // handle input from the clients
        void handler(THandler && handler);

        // shorthand for string_view from var
        template <typename T>
            static std::string_view view(T& v) {
                if constexpr (std::is_same<T, std::string>::value) {
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

        // get global instance
        static FIncppect& getInstance() {
            static FIncppect instance;
            return instance;
        }

    private:
        struct Impl;
        TUniquePtr<Impl> m_impl;
};