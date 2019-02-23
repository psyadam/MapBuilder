//TCP/IP client and server(listener)
//written by Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "TCP.h"

#ifdef MPMA_COMPILE_NET

#include "UPNP.h"
#include "../base/Memory.h"
#include "../base/DebugRouter.h"
#include "PlatformSockets.h"

using namespace NET_INTERNAL;

//#define VERBOSE_TCP

namespace NET
{
    // -- client

    TCPClient::TCPClient()
    {
        sock=-1;
        port=0;
        connected=true;
    }

    //Initiates a new connection to a remote host.  If localPort is 0, the system will select one automatically.  Returns 0 on failure.
    TCPClient* TCPClient::Connect(const Address &addr, uint16 localPort)
    {
#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: About to connect to "<<addr.GetIPPortString()<<" localport="<<localPort<<"\n";
#endif
        if (addr.GetPort()==0)
        {
            MPMA::ErrorReport()<<"Cannot connect to port 0.\n";
            return 0;
        }

        //set us up the socket
        TCPClient *client=new2(TCPClient,TCPClient);
        client->port=localPort;
        client->remoteAddr=addr;
        client->sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client->sock<0)
        {
            MPMA::ErrorReport()<<"Failed to create socket for tcp client.\n";
            FreeClient(client);
            return 0;
        }

        int val=1;
        setsockopt(client->sock, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val));

        //if they specified a local port, bind to that
        if (localPort!=0)
        {
            sockaddr_in baddr;
            memset(&baddr, 0, sizeof(baddr));
            baddr.sin_family=PF_INET;
            baddr.sin_addr.s_addr=INADDR_ANY;
            baddr.sin_port=htons(localPort);

            if (bind(client->sock, (sockaddr*)&baddr, sizeof(baddr)))
            {
                MPMA::ErrorReport()<<"Failed to bind to port "<<localPort<<": "<<Internal_GetLastError()<<"\n";
                FreeClient(client);
                return 0;
            }
        }

        //try to connect
        sockaddr_in raddr;
        memset(&raddr, 0, sizeof(raddr));
        raddr.sin_family=PF_INET;
        raddr.sin_addr.s_addr=inet_addr(addr.GetIPString().c_str());
        raddr.sin_port=htons(addr.GetPort());
        if (connect(client->sock, (sockaddr*)&raddr, sizeof(raddr)))
        {
            MPMA::ErrorReport()<<"Failed to connect to "<<addr.GetIPString()<<" port "<<addr.GetPort()<<": "<<Internal_GetLastError()<<"\n";
            FreeClient(client);
            return 0;
        }

        //set the port to what we are bound to
        sockaddr_in laddr;
        memset(&laddr, 0, sizeof(laddr));
        laddr.sin_family=PF_INET;
        laddr.sin_addr.s_addr=INADDR_ANY;
        socklen_t laddrlen=sizeof(laddr);
        getsockname(client->sock, (sockaddr*)&laddr, &laddrlen);
        client->port=ntohs(laddr.sin_port);

        //set initial socket options
        if (!Internal_SetSocketBlocking(client->sock, false))
        {
            MPMA::ErrorReport()<<"Failed to set socket nonblocking mode.\n";
            //ignore it... better than just failing.
        }
        client->EnableSendCoalescing(true);

        return client;
    }

    //Disconnects and frees a client.
    void TCPClient::FreeClient(TCPClient *client)
    {
        if (client!=0)
        {
            Internal_SetSocketBlocking(client->sock, true);
            Internal_CloseSocket(client->sock);
            delete2(client);
        }
    }

    //Sends data to the the remote.
    bool TCPClient::Send(const void *data, nuint dataLen)
    {
#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: About to send "<<dataLen<<" bytes.  connected="<<connected<<"\n";
#endif
        if (!connected) return false;

        Internal_SetSocketBlocking(sock, true); //we want to block if needed

        bool good=true;
        Internal_ClearLastError();
        int numSent=send(sock, (const char*)data, (int)dataLen, 0);
        if (Internal_IsLastSocketErrorADisconnect()) connected=false;
        if (numSent==-1 || numSent!=(int)dataLen)
        {
            MPMA::ErrorReport()<<"TCP send failed: "<<Internal_GetLastError()<<"\n";
            good=false;
        }

        Internal_SetSocketBlocking(sock, false); //go back to nonblocking

#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: Finished send, "<<numSent<<" returned from send.  success="<<good<<" connected="<<connected<<"\n";
#endif
        return good;
    }

    //Receives bytes from the remote and appends it to data.
    nuint TCPClient::Receive(std::vector<uint8> &data, nuint exactBytesToRetrieve)
    {
#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: About to receive.  connected="<<connected<<".  exactBytesToRetrieve="<<exactBytesToRetrieve<<"\n";
#endif
        if (!connected)
            return 0;

        //do the recv
        uint8 buff[32*1024];
#ifdef VERBOSE_TCP
        bool good=true;
#endif

        Internal_ClearLastError();
        int count=recv(sock, (char*)buff, sizeof(buff), 0);
        if (Internal_IsLastSocketErrorADisconnect())
            connected=false;
        if (count==0)
            connected=false;
#ifdef VERBOSE_TCP
        if (count<=0)
            good=false;
#endif

        //add them to the accumulated buffer, then pull out however many is needed from that
        if (count>0)
            bufferedBytes.insert(bufferedBytes.end(), &buff[0], &buff[count]);

        nsint countReturned=0;
        if (exactBytesToRetrieve==0 || exactBytesToRetrieve==bufferedBytes.size()) //all
        {
            data.insert(data.begin(), bufferedBytes.begin(), bufferedBytes.end());
            countReturned=(int)bufferedBytes.size();
            bufferedBytes.clear();
        }
        else //partial amount
        {
            if (bufferedBytes.size()>=exactBytesToRetrieve)
            {
                data.insert(data.end(), bufferedBytes.begin(), bufferedBytes.begin()+exactBytesToRetrieve);
                bufferedBytes.erase(bufferedBytes.begin(), bufferedBytes.begin()+exactBytesToRetrieve);
                countReturned=exactBytesToRetrieve;
            }
        }

#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: Finished receive, "<<count<<" returned from recv, returning "<<countReturned<<" bytes.  success="<<good<<" connected="<<connected<<"\n";
#endif
        return countReturned;
    }

    //Returns whether the client is still connected to the remote.
    bool TCPClient::IsConnected() const
    {
#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: About to check connection.  connected="<<connected<<"\n";
#endif
        if (!connected) return false;

        char tmp;
        Internal_ClearLastError();
        int count=recv(sock, &tmp, 1, MSG_PEEK);
        if (Internal_IsLastSocketErrorADisconnect()) connected=false;
        if (count==0) connected=false;

#ifdef VERBOSE_TCP
        MPMA::ErrorReport()<<"TCP: Finished connection check.  connected="<<connected<<"\n";
#endif
        return connected; 
    }

    //Improves network bandwidth at the expense of latency (off by default).
    void TCPClient::EnableSendCoalescing(bool allow)
    {
        uint32 val=(allow?1:0);
        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&val, sizeof(val)))
        {
            MPMA::ErrorReport()<<"EnableSendCoalescing: setsockopt for TCP_NODELAY failed.\n";
        }
    }

    // -- server

    TCPServer::TCPServer()
    {
        sock=-1;
        port=0;
    }

    //Creates a new server to listen for TCP connections on a single port.
    TCPServer* TCPServer::Listen(uint16 port, bool openNatAutomatically)
    {
        if (port==0)
        {
            MPMA::ErrorReport()<<"Cannot liston on port 0.\n";
            return 0;
        }

        //set us up the socket
        TCPServer *serv=new2(TCPServer,TCPServer);
        serv->port=port;
        serv->autoUPNP=false;
        serv->sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serv->sock<0)
        {
            MPMA::ErrorReport()<<"Failed to create socket for tcp client.\n";
            FreeServer(serv);
            return 0;
        }

        int val=1;
        setsockopt(serv->sock, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val));

        //bind to the port
        if (openNatAutomatically)
            UPNP::ClaimTCPPort(port);

        sockaddr_in baddr;
        memset(&baddr, 0, sizeof(baddr));
        baddr.sin_family=PF_INET;
        baddr.sin_addr.s_addr=INADDR_ANY;
        baddr.sin_port=htons(port);

        if (bind(serv->sock, (sockaddr*)&baddr, sizeof(baddr)))
        {
            MPMA::ErrorReport()<<"Failed to bind to port "<<port<<": "<<Internal_GetLastError()<<"\n";
            FreeServer(serv);
            return 0;
        }

        serv->autoUPNP=openNatAutomatically;

        //listen
        if (listen(serv->sock, 64))
        {
            MPMA::ErrorReport()<<"Failed to listen on the socket: "<<Internal_GetLastError()<<"\n";
            FreeServer(serv);
            return 0;
        }

        //set nonblocking
        if (!Internal_SetSocketBlocking(serv->sock, false))
        {
            MPMA::ErrorReport()<<"Failed to set socket nonblocking mode.\n";
            //ignore it... better than just failing.
        }

        return serv;
    }

    //Stops listening and frees a server.
    void TCPServer::FreeServer(TCPServer *server)
    {
        if (server!=0)
        {
            if (server->autoUPNP)
                UPNP::ReleaseTCPPort(server->port);

            Internal_SetSocketBlocking(server->sock, true);
            Internal_CloseSocket(server->sock);
            delete2(server);
        }
    }

    //Returns the next client that is connecting, or 0 if none.
    TCPClient* TCPServer::GetNextConnection()
    {
        //accept a request if there are any
        sockaddr_in saddr;
        socklen_t saddrlen=sizeof(saddr);
        nsint newsock=accept(sock, (sockaddr*)&saddr, &saddrlen);
        if (newsock<0)
            return 0;

        //create a client to represent it
        TCPClient *client=new2(TCPClient,TCPClient);
        client->port=port;
        client->remoteAddr.SetAddress(inet_ntoa(saddr.sin_addr));
        client->remoteAddr.SetPort(ntohs(saddr.sin_port));
        client->sock=newsock;
        client->connected=true;

        //set new client inital socket options
        if (!Internal_SetSocketBlocking(client->sock, false))
            MPMA::ErrorReport()<<"Failed to set new client socket to nonblocking mode.\n";
        client->EnableSendCoalescing(true);

        return client;
    }
}

#endif //#ifdef MPMA_COMPILE_NET
