//!\file TCP.h TCP/IP client and server(listener)
//written by Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_NET

#include "Common.h"
#include <vector>

class MPMAMemoryManager; //FD

namespace NET
{
    //!Represents a stream-based reliable connection to another PC.
    class TCPClient
    {
    public:
        //!Initiates a new connection to a remote host.  If localPort is 0, the system will select one automatically.  Returns 0 on failure.
        static TCPClient* Connect(const Address &addr, uint16 localPort=0);

        //!Disconnects and frees a client.
        static void FreeClient(TCPClient *client);

        //!Returns whether the client is still connected to the remote.
        bool IsConnected() const;

        //!Sends data to the the remote.  Returns false if the send failed.
        bool Send(const void *data, nuint dataLen);

        //!Receives bytes from the remote and appends it to data.  Returns the number of bytes retrieved.  If exactBytesToRetrieve is specified, then either exactly that many bytes will be returned from the stream, or none will be.
        nuint Receive(std::vector<uint8> &data, nuint exactBytesToRetrieve=0);

        //!Returns the local port that the client is using.
        inline uint16 GetLocalPort() const { return port; }

        //!Returns the address of the remote.
        inline const Address& GetRemoteAddress() const { return remoteAddr; }

        //!Improves network bandwidth at the expense of latency (on by default).
        void EnableSendCoalescing(bool allow);

        //TODO: Some sort of callback mechanism for notifications.

    private:
        //we are the only allocator
        TCPClient();
        inline ~TCPClient() {}
        inline TCPClient(const TCPClient&) {}
        inline void operator=(const TCPClient&) {}
        friend class ::MPMAMemoryManager;
        friend class TCPServer;

        //
        Address remoteAddr;
        uint16 port;
        nsint sock;
        mutable bool connected;
        std::vector<uint8> bufferedBytes;
    };

    //!Listens for new connections and spawns a TCPClient to recieve them.
    class TCPServer
    {
    public:
        //!Creates a new server to listen for TCP connections on a single port.
        static TCPServer* Listen(uint16 port, bool openNatAutomatically=true);

        //!Stops listening and frees a server.
        static void FreeServer(TCPServer *server);

        //!Returns the next client that is connecting, or 0 if none.
        TCPClient* GetNextConnection();

        //!Returns the local port that the server is listening on.
        inline uint16 GetListenPort() const { return port; }

        //TODO: Some sort of callback mechanism for notifications.

    private:
        //we are the only allocator
        TCPServer();
        inline ~TCPServer() {}
        inline TCPServer(const TCPServer&) {}
        inline void operator=(const TCPServer&) {}
        friend class ::MPMAMemoryManager;

        //
        nsint sock;
        uint16 port;
        bool autoUPNP;
    };
}

#endif //#ifdef MPMA_COMPILE_NET
