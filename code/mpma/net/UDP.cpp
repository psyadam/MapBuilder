//!\file UDP.h UDP packet flinging
//written by Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "UDP.h"

#ifdef MPMA_COMPILE_NET

#include "UPNP.h"
#include "../base/Memory.h"
#include "../base/DebugRouter.h"
#include "PlatformSockets.h"

using namespace NET_INTERNAL;

namespace NET
{
    UDPClient::UDPClient()
    {
        sock=-1;
    }

    //Creates a socket to be used to send upd packet.  Returns 0 on failure.
    UDPClient* UDPClient::Create(uint16 localPort, bool allowBroadcast, bool openNatAutomatically)
    {
        //set us up the socket
        UDPClient *udp=new2(UDPClient,UDPClient);
        udp->isInBlockingMode=true;
        udp->autoUPNP=false;
        udp->sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp->sock<0)
        {
            MPMA::ErrorReport()<<"Failed to create socket for udp.\n";
            FreeClient(udp);
            return 0;
        }

        int val=1;
        setsockopt(udp->sock, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val));

        if (allowBroadcast)
        {
            int broadcast=1;
            if (setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)))
            {
                MPMA::ErrorReport()<<"Failed to enabled udp broadcasts on the socket.";
                FreeClient(udp);
                return 0;
            }
        }

        if(openNatAutomatically && localPort!=0)
            UPNP::ClaimUDPPort(localPort);

        //bind it
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family=PF_INET;
        addr.sin_addr.s_addr=INADDR_ANY;
        addr.sin_port=htons(localPort);

        if (bind(udp->sock, (sockaddr*)&addr, sizeof(addr)))
        {
            MPMA::ErrorReport()<<"Failed to bind to port "<<localPort<<": "<<Internal_GetLastError()<<"\n";
            FreeClient(udp);
            return 0;
        }

        //store the port we bound to
        memset(&addr, 0, sizeof(addr));
        addr.sin_family=PF_INET;
        addr.sin_addr.s_addr=INADDR_ANY;
        socklen_t addrlen=sizeof(addr);
        getsockname(udp->sock, (sockaddr*)&addr, &addrlen);
        udp->localPortUsed=ntohs(addr.sin_port);

        if(openNatAutomatically && localPort==0)
            UPNP::ClaimUDPPort(udp->localPortUsed);

        udp->autoUPNP=openNatAutomatically;
        return udp;
    }

    //Frees the client.
    void UDPClient::FreeClient(UDPClient *client)
    {
        if (client)
        {
            if (client->autoUPNP)
                UPNP::ReleaseUDPPort(client->localPortUsed);

            Internal_SetSocketBlocking(client->sock, true);
            Internal_CloseSocket(client->sock);
            delete2(client);
        }
    }

    //Sends a packet of data.  Returns false if the send failed.
    bool UDPClient::Send(const Address &addr, const void *data, nuint dataLen)
    {
#ifdef _DEBUG
        if (dataLen>65507)
        {
            MPMA::ErrorReport()<<"Sending a packet of size "<<dataLen<<" which is greater than max udp packet size of "<<65507<<".  This will probably fail or be truncated.\n";
        }
#endif

        if (!isInBlockingMode)
        {
            isInBlockingMode=true;
            if (!Internal_SetSocketBlocking(sock, true))
            {
                MPMA::ErrorReport()<<"UDP send: Failed to set socket blocking mode.\n";
            }
        }

        sockaddr_in saddr;
        saddr.sin_family=AF_INET;
        saddr.sin_addr.s_addr=inet_addr(addr.GetIPString().c_str());
        saddr.sin_port=htons(addr.GetPort());

        int count=sendto(sock, (const char*)data, (int)dataLen, 0, (sockaddr*)&saddr, sizeof(saddr));
        if (count<=0)
        {
            MPMA::ErrorReport()<<"UDP send failed: "<<Internal_GetLastError()<<"\n";
            return false;
        }

        return true;
    }

    //Receives the next packet and appends it to data.  source (if not 0) will be filled with the address the packet came from.  Returns false if there was nothing received.
    bool UDPClient::Receive(std::vector<uint8> &data, Address *source)
    {
        if (isInBlockingMode)
        {
            isInBlockingMode=false;
            if (!Internal_SetSocketBlocking(sock, false))
            {
                MPMA::ErrorReport()<<"UDP receive: Failed to set socket nonblocking mode.\n";
            }
        }

        uint8 buf[64*1024]; //big enough to hold the max udp datagram size

        int count;
        if (!source)
            count=recv(sock, (char*)&buf[0], sizeof(buf), 0);
        else
        {
            sockaddr_in saddr;
            socklen_t saddrlen=sizeof(saddr);
            count=recvfrom(sock, (char*)&buf[0], sizeof(buf), 0, (sockaddr*)&saddr, &saddrlen);
            if (count>0)
            {
                source->SetAddress(inet_ntoa(saddr.sin_addr));
                source->SetPort(ntohs(saddr.sin_port));
            }
        }

        if (count<=0)
            return false;

        data.insert(data.end(), &buf[0], &buf[count]);
        return true;
    }
}

#endif //#ifdef MPMA_COMPILE_NET
