//provides helpers for platform-specific socket operations.
//this is meant for internal framework use only.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../../Config.h"

#ifdef MPMA_COMPILE_NET

#include "../PlatformSockets.h"
#include "../../base/Vary.h"
#include "../../base/DebugRouter.h"

#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <vector>
#include <memory.h>

namespace NET_INTERNAL
{
    //sets a socket to be blocking or nonblocking
    bool Internal_SetSocketBlocking(nsint sock, bool block)
    {
        int flags=fcntl(sock, F_GETFL, 0);
        if (block) flags&=~O_NONBLOCK;
        else flags|=O_NONBLOCK;

        if (fcntl(sock, F_SETFL, flags)<0)
            return false;
        return true;
    }

    //clears any previous error codes set
    void Internal_ClearLastError()
    {
        errno=0;
    }

    //gets a string that represents a socket error for the most recent operation
    std::string Internal_GetLastError()
    {
        MPMA::Vary reason;
        if (errno==0) reason="No error";
        else if (errno==EADDRINUSE) reason="Address already in use";
        else if (errno==EADDRNOTAVAIL) reason="Address not available";
        else if (errno==EACCES) reason="Access denied";
        else if (errno==ETIMEDOUT) reason="Connection timed out";
        else if (errno==ECONNREFUSED) reason="Connection refused";
        else if (errno==ENETUNREACH) reason="Destination unreachable";
        else if (errno==EAFNOSUPPORT) reason="Not supported";
        else if (errno==EINVAL) reason="Invalid argument";
        else
        {
            reason="Unknown(";
            reason+=errno;
            reason+=")";
        }
        return reason;
    }

    //closes a socket
    void Internal_CloseSocket(nsint sock)
    {
        shutdown(sock, 2); //graceful disconnect
        close(sock); //invalidate descriptor
    }

    //returns whether the error from the most recent socket operations indicates a loss of connection, or whether the socket was nicely closed
    bool Internal_IsLastSocketErrorADisconnect()
    {
        if (errno==ECONNRESET || errno==ENOTCONN || errno==ETIMEDOUT || errno==EBADF || errno==ENOTSOCK)
            return true;
        else
            return false;
    }

    //used by Internal_GetSubnet
    struct IFace
    {
        uint32 addr;
        uint32 mask;
        uint16 flags;
    };

    //gets our subnet mask and subnet address
    bool Internal_GetSubnet(uint32 &mask, uint32 &net, uint32 &localIp)
    {
        //get interface list (eth0-eth9)
        int s=socket(AF_INET, SOCK_DGRAM, 0);
        if (s<0)
        {
            MPMA::ErrorReport()<<"Internal_GetSubnet: socket creation failed.\n";
            return false;
        }

        std::vector<IFace> ifaces;
        for (int i=0; i<10; ++i)
        {
            IFace iface;
            MPMA::VaryString ifaceName("eth");
            ifaceName+=i;
            ifreq f;
            memset(&f, 0, sizeof(f));
            //f.ifru_addr.sa_family=AF_INET;
            strcpy(f.ifr_name, ifaceName.c_str());

            if (!ioctl(s, SIOCGIFADDR, &f))
            {
                iface.addr=((sockaddr_in*)(&f.ifr_addr))->sin_addr.s_addr;

                ioctl(s, SIOCGIFNETMASK, &f);
                iface.mask=((sockaddr_in*)(&f.ifr_netmask))->sin_addr.s_addr;

                ioctl(s, SIOCGIFFLAGS, &f);
                iface.flags=f.ifr_flags;

                ifaces.push_back(iface);
            }
        }

        //search list for one that look right
        bool found=false;
        for (std::vector<IFace>::iterator i=ifaces.begin(); i!=ifaces.end(); ++i)
        {
            if ((*i).flags&IFF_UP) //must be up
            {
                if (!((*i).flags&IFF_LOOPBACK || (*i).flags&IFF_POINTOPOINT)) //must not be these types
                {
                    //address must not be 0.0.0.0 or 255.255.255.255
                    if (!((*i).addr==0 || (*i).addr==0xffffffff))
                    {
                        found=true;
                        mask=(*i).mask;
                        localIp=(*i).addr;
                        net=localIp&mask;
                    }
                }
            }
        }

        if (!found)
        {
            MPMA::ErrorReport()<<"Internal_GetSubnet: Not appropriate interfaces found.\n";
        }

        shutdown(s, SHUT_RDWR);
        return found;
    }
}

#endif //#ifdef MPMA_COMPILE_NET
