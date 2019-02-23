//!\file UPNP.h Utilities to help poke holes in evil NATs.
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
    //!Utilities to help bypass NATs.
    namespace UPNP
    {
        //!Returns whether UPNP service is available on the network.  If this returns false, most other functions here will likely fail.
        bool IsAvailable();

        //!Retrieves the public internet-facing IP of the local network.  If that information is not available, an the IP of the local machine is returned instead.
        std::string GetInternetFacingIP();

        //!Returns the name of the router device that we are talking to with UPNP.
        std::string GetDeviceName();

        //!Whether the framework should attempt to try other nat-opening techniques if UPNP is not available when either of the Claim*Port functions are called.  These techniques may include sending out packets and trying to open tcp connections.  Default is true.
        extern bool AllowNinjaNatTechniques;

        //!Instructs the router to redirect all UDP packets sent to a specific port to us.
        void ClaimUDPPort(uint16 port);

        //!Instructs the router to redirect all TCP connection attempts to a specific port to us.
        void ClaimTCPPort(uint16 port);

        //!Instructs the router that we are no longer interested in a specific udp port.
        void ReleaseUDPPort(uint16 port);

        //!Instructs the router that we are no longer interested in a specific tcp port.
        void ReleaseTCPPort(uint16 port);
    }
}

#endif //#ifdef MPMA_COMPILE_NET
