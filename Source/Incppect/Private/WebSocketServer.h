// Fill out your copyright notice in the Description page of Project Settings.

#pragma  once

// Copy from WebSocketNetworking plugin

#define USE_LIBWEBSOCKET 1

#if USE_LIBWEBSOCKET
#include "Runtime/Sockets/Private/BSDSockets/SocketSubsystemBSD.h" // IWYU pragma: keep
#else
#include <netinet/in.h>
#endif
#include <string>

typedef struct lws_context WebSocketInternalContext;
typedef struct lws WebSocketInternal;
typedef struct lws_protocols WebSocketInternalProtocol;
typedef struct lws_http_mount WebSocketInternalHttpMount;

namespace Incppect
{
class FWebSocketHttpMount
{
public:

	/**
	 * Sets the absolute path on disk to the directory we wish to serve.
	 * @param InPathOnDisk The absolute path to the directory.
	 */
	void SetPathOnDisk(const FString& InPathOnDisk) { PathOnDisk = std::string(TCHAR_TO_ANSI(*InPathOnDisk)); }

	/**
	 * Sets The web url path to use for this mount, e.g. /images .
	 * @param InWebPath The web url path we will map the directory to, e.g. /images.
	 */
	void SetWebPath(const FString& InWebPath) { WebPath = std::string(TCHAR_TO_ANSI(*InWebPath)); }

	/**
	 * Sets a file to serve when the root web path is requested.
	 * @param InDefaultFile The file to serve when the root web path is request, e.g. index.html.
	 */
	void SetDefaultFile(const FString& InDefaultFile) { DefaultFile = std::string(TCHAR_TO_ANSI(*InDefaultFile)); }

	const char* GetPathOnDisk() { return PathOnDisk.c_str(); }
	const char* GetWebPath() { return WebPath.c_str(); }
	const char* GetDefaultFile() { return DefaultFile.c_str(); }
	bool HasDefaultFile() { return DefaultFile.empty(); }

private:

	// Note: The below members are std::string purposefully as lws requires char* strings and FString may not be char*.

	/* The absolute path on disk to directory we wish to serve. */
	std::string PathOnDisk;

	/* The web url path to use for this mount, e.g. /images */
	std::string WebPath;

	/* When the root of the `WebPath` is requested, without a file, we can serve this file, e.g. index.html */
	std::string DefaultFile = "index.html";
};

enum class EWebsocketConnectionFilterResult : uint8
{
	ConnectionAccepted,
	ConnectionRefused
};

DECLARE_DELEGATE(FWebSocketInfoCallBack);
DECLARE_DELEGATE_TwoParams(FWebSocketPacketReceivedCallBack, void* /*Data*/, int32 /*Data Size*/);
DECLARE_DELEGATE_OneParam(FWebSocketClientConnectedCallBack, class FWebSocket* /*Socket*/);
DECLARE_DELEGATE_RetVal_TwoParams(EWebsocketConnectionFilterResult, FWebSocketFilterConnectionCallback, FString /*Origin*/, FString /*ClientIP*/);

class FWebSocket
{

public:

	// Initialize as client side socket.
	FWebSocket(const FInternetAddr& ServerAddress);

#if USE_LIBWEBSOCKET
	// Initialize as server side socket.
	FWebSocket(WebSocketInternalContext* InContext, WebSocketInternal* Wsi);
#endif

	//~ Begin INetworkingWebSocket interface
	~FWebSocket();
	void SetConnectedCallBack(FWebSocketInfoCallBack CallBack);
	void SetErrorCallBack(FWebSocketInfoCallBack CallBack);
	void SetReceiveCallBack(FWebSocketPacketReceivedCallBack CallBack);
	void SetSocketClosedCallBack(FWebSocketInfoCallBack CallBack);
	bool Send(const uint8* Data, uint32 Size, bool bPrependSize = true);
	void Tick();
	void Flush();
	TArray<uint8> GetRawRemoteAddr(int32& OutPort);
	FString RemoteEndPoint(bool bAppendPort);
	FString LocalEndPoint(bool bAppendPort);
	struct sockaddr_in* GetRemoteAddr() { return &RemoteAddr; }
	//~ End INetworkingWebSocket interface

// this was made public because of cross-platform build issues
public:
	void HandlePacket();
	void OnReceive(void* Data, uint32 Size);
	void OnRawRecieve(void* Data, uint32 Size);
	void OnRawWebSocketWritable(WebSocketInternal* wsi);
	void OnClose();

	/************************************************************************/
	/*	Various Socket callbacks											*/
	/************************************************************************/
	FWebSocketPacketReceivedCallBack  ReceivedCallback;
	FWebSocketInfoCallBack ConnectedCallBack;
	FWebSocketInfoCallBack ErrorCallBack;
	FWebSocketInfoCallBack SocketClosedCallback;

	/**  Recv and Send Buffers, serviced during the Tick */
	TArray<uint8> ReceiveBuffer;
	TArray<TArray<uint8>> OutgoingBuffer;

#if USE_LIBWEBSOCKET
	/** libwebsocket internal context*/
	WebSocketInternalContext* Context;

	/** libwebsocket web socket */
	WebSocketInternal* Wsi;

	/** libwebsocket Protocols that can be serviced by this implemenation*/
	WebSocketInternalProtocol* Protocols;
#else // ! USE_LIBWEBSOCKET -- HTML5 uses BSD network API
	int SockFd;
#endif
	struct sockaddr_in RemoteAddr;

	/** Server side socket or client side*/
	bool IsServerSide;

	friend class FWebSocketServer;
};

class FWebSocketServer
{
public:

	FWebSocketServer() = default;

	//~ Begin IWebSocketServer interface
	~FWebSocketServer();
	void EnableHTTPServer(TArray<FWebSocketHttpMount> DirectoriesToServe);
	bool Init(uint32 Port, FWebSocketClientConnectedCallBack, FString BindAddress = TEXT(""));
	void SetFilterConnectionCallback(FWebSocketFilterConnectionCallback InFilterConnectionCallback);
	void Tick();
	FString Info();
	//~ End IWebSocketServer interface

	bool IsHttpEnabled() const;

	// this was made public because of cross-platform build issues
	public:

	/** Callback for a new websocket connection to the server */
	FWebSocketClientConnectedCallBack  ConnectedCallBack;

	/** Handler called to filter incoming websocket connections. */
	FWebSocketFilterConnectionCallback FilterConnectionCallback;

	/** Internal libwebsocket context */
	WebSocketInternalContext* Context;

	/** Protocols serviced by this implementation */
	WebSocketInternalProtocol* Protocols;

	friend class FWebSocket;
	uint32 ServerPort;

private:
	bool bEnableHttp = false;

	TArray<FWebSocketHttpMount> DirectoriesToServe;
	WebSocketInternalHttpMount* LwsHttpMounts = NULL;
};

}
