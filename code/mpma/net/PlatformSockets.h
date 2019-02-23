//includes whatever files are needed to access the platform's socket implementation and provides helpers for platform-specific socket operations.
//this is meant for internal framework use only.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_NET

#include <string>
#include "../base/Types.h"

// -- do platform includes and typedefs

#if defined(_WIN32) || defined(_WIN64) // msvc

#include <Winsock2.h>
#undef SetPort //wow

typedef int socklen_t;

#elif defined(linux)  // g++ linux

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>

#else // ?

    #error Unknown platform in PlatformSockets.h

#endif

// -- functions we have to abstract

namespace NET_INTERNAL
{
    //sets a socket to be blocking or nonblocking
    bool Internal_SetSocketBlocking(nsint sock, bool block);

    //clears any previous error codes set
    void Internal_ClearLastError();

    //gets a string that represents a socket error for the most recent operation
    std::string Internal_GetLastError();

    //closes a socket
    void Internal_CloseSocket(nsint sock);

    //returns whether the error from the most recent socket operations indicates a loss of connection, or whether the socket was nicely closed
    bool Internal_IsLastSocketErrorADisconnect();

    //gets our subnet mask and subnet address
    bool Internal_GetSubnet(uint32 &mask, uint32 &net, uint32 &localIp);
}

#endif //#ifdef MPMA_COMPILE_NET
