//!\file UDP.h UDP packet flinging
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
    //!Used to send udp packets.
    class UDPClient
    {
    public:
        //!Creates a socket to be used to send/receivee upd packets.  If localPort is 0, the system picks a random port to use.  Returns 0 on failure.
        static UDPClient* Create(uint16 localPort=0, bool allowBroadcast=false, bool openNatAutomatically=true);

        //!Frees a UDPClient.
        static void FreeClient(UDPClient *client);

        //!Sends a packet of data to a specific address.  Returns false if the send failed.
        bool Send(const Address &addr, const void *data, nuint dataLen);

        //!Returns the local port.
        inline uint16 GetLocalPort() const
            { return localPortUsed; }

         //!Receives the next packet and appends it to data.  source (if not 0) will be filled with the address the packet came from.  Returns false if there was nothing to receive (does not block).
        bool Receive(std::vector<uint8> &data, Address *source=0);

        //TODO: callback system for data coming in

    private:
        //we are the only allocator
        UDPClient();
        inline ~UDPClient() {}
        inline UDPClient(const UDPClient&) {}
        inline void operator=(const UDPClient&) {}
        friend class ::MPMAMemoryManager;

        //
        nsint sock;
        uint16 localPortUsed;
        bool isInBlockingMode;
        bool autoUPNP;
    };
}

#endif //#ifdef MPMA_COMPILE_NET
