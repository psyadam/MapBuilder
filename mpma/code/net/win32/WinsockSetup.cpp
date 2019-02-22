//Setup and Shutdown winsock
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../PlatformSockets.h"

#ifdef MPMA_COMPILE_NET

#include "../../Setup.h"
#include "../../base/DebugRouter.h"

namespace
{
    void DoWinsockInitialize()
    {
        WSADATA wsaData;
        WORD wVersionRequested=MAKEWORD(2,2);
        int err=WSAStartup(wVersionRequested, &wsaData);

        if (err!=0)
        {
            MPMA::ErrorReport()<<"Could not load winsock2.\n";
            return;
        }
        
        if (LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2)
        {
            MPMA::ErrorReport()<<"Could not load the correct version of winsock2.\n";
            WSACleanup();
        }

    }
    void DoWinsockShutdown()
    {
        WSACleanup();
    }
    
    class AutoInitWinsock
    {
    public:
        //hookup init callbacks
        AutoInitWinsock()
        {
            MPMA::Internal_AddInitCallback(DoWinsockInitialize,1000);
            MPMA::Internal_AddShutdownCallback(DoWinsockShutdown,1000);
        }
    } autoInitWinsock;
}

bool mpmaForceReferenceToWinsockSetupWin32CPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_NET
