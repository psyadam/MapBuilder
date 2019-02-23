//provides helpers for platform-specific socket operations.
//this is meant for internal framework use only.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../PlatformSockets.h"

#ifdef MPMA_COMPILE_NET

#include "../../base/Vary.h"
#include "../../base/DebugRouter.h"

#include <ws2tcpip.h>

namespace NET_INTERNAL
{
    //sets a socket to be blocking or nonblocking
    bool Internal_SetSocketBlocking(nsint sock, bool block)
    {
        unsigned long val=(block?0:1);
        if (ioctlsocket(sock, FIONBIO, &val)==SOCKET_ERROR)
            return false;
        return true;
    }

    //clears any previous error codes set
    void Internal_ClearLastError()
    {
        WSASetLastError(0);
    }

    //gets a string that represents a socket error for the most recent operation
    std::string Internal_GetLastError()
    {
        int errnum=WSAGetLastError();
        MPMA::VaryString reason;
        if (errnum==0) reason="No errer";
        else if (errnum==WSAENETDOWN) reason="Network failure";
        else if (errnum==WSAEADDRINUSE) reason="Address already in use";
        else if (errnum==WSAEADDRNOTAVAIL) reason="Address not available";
        else if (errnum==WSAETIMEDOUT) reason="Connection timed out";
        else if (errnum==WSAECONNREFUSED) reason="Connection refused";
        else if (errnum==WSAENETUNREACH) reason="Destination unreachable";
        else if (errnum==WSAEINVAL) reason="Invalid argument";
        else
        {
            reason="Unknown(";
            reason+=errnum;
            reason+=")";
        }
        return (const std::string&)reason;
    }

    //closes a socket
    void Internal_CloseSocket(nsint sock)
    {
        closesocket(sock);
    }

    //returns whether the error from the most recent socket operations indicates a loss of connection, or whether the socket was nicely closed
    bool Internal_IsLastSocketErrorADisconnect()
    {
        int errnum=WSAGetLastError();
        if (errnum==WSAENETDOWN || errnum==WSAENOTCONN || errnum==WSAENETRESET || errnum==WSAESHUTDOWN || errnum==WSAECONNABORTED || errnum==WSAETIMEDOUT || errnum==WSAECONNRESET || errnum==WSAENOTSOCK || errnum==WSAEINVAL)
            return true;
        else
            return false;
    }

    //gets our subnet mask and subnet address
    bool Internal_GetSubnet(uint32 &mask, uint32 &net, uint32 &localIp)
    {
        //get interface list
        SOCKET s=WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
        if (s==INVALID_SOCKET)
        {
            MPMA::ErrorReport()<<"Internal_GetSubnet: WSASocket failed.\n";
            return false;
        }

        INTERFACE_INFO iface[20];
        unsigned long bytes=0;
        if (WSAIoctl(s, SIO_GET_INTERFACE_LIST, 0, 0, &iface, sizeof(iface), &bytes, 0, 0))
        {
            MPMA::ErrorReport()<<"Internal_GetSubnet: WSAIoctl failed.\n";
            shutdown(s, SD_BOTH);
            return false;
        }

        //search list for one that look right
        bool found=false;
        for (int i=0; i<20; ++i)
        {
            if (iface[i].iiFlags&IFF_UP) //must be up
            {
                if (!(iface[i].iiFlags&IFF_LOOPBACK || iface[i].iiFlags&IFF_POINTTOPOINT)) //must not be these types
                {
                    //address must not be 0.0.0.0 or 255.255.255.255
                    if (!(iface[i].iiAddress.AddressIn.sin_addr.s_addr==0 || iface[i].iiAddress.AddressIn.sin_addr.s_addr==0xffffffff))
                    {
                        found=true;
                        mask=iface[i].iiNetmask.AddressIn.sin_addr.s_addr;
                        localIp=iface[i].iiAddress.AddressIn.sin_addr.s_addr;
                        net=localIp&mask;
                    }
                }
            }
        }

        if (!found)
        {
            MPMA::ErrorReport()<<"Internal_GetSubnet: Not appropriate interfaces found.\n";
        }

        shutdown(s, SD_BOTH);
        return found;
    }
}

#endif //#ifdef MPMA_COMPILE_NET
