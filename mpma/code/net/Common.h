//!\file Common.h Common structs and functions used by the different protocols.
//written by Luke Lenhart, 2007-2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_NET

#include "../base/Types.h"

#include <string>

//!Networking
namespace NET
{
    //!Represents an IP address and port
    class Address
    {
    public:
        //!ctor
        inline Address()
        {
            ZeroOut();
        }

        //!ctor - can take an address in either "1.2.3.4:56", "1.2.3.4", "name:port", or "name" format
        inline Address(const std::string &addressColonPort)
        {
            ZeroOut();
            SetAddress(addressColonPort);
        }

        //!ctor - takes an address in the form "1.2.3.4" or "name" and a numeric port value
        inline Address(const std::string &addressString, uint16 portNumber)
        {
            ZeroOut();
            SetAddress(addressString);
            port=portNumber;
        }

        //!Retrieves a string that represents the IP address
        std::string GetIPString() const;

        //!Retrieves the packed IP
        inline nuint GetIP() const
        {
            return ip;
        }

        //!Retrieves a string that represents the IP address and port in "1.2.3.4:56" format
        std::string GetIPPortString() const;

        //!Retrieves the string that represents the name of the address.  A DNS lookup may be performed.
        std::string GetNameString() const;

        //!Retrieves a string that represents the name and port in "name.net:56" format.  A DNS lookup may be performed.
        std::string GetNamePortString() const;

        //!Retrieves the port for this address
        inline uint16 GetPort() const
        {
            return port;
        }

        //!Sets the address - can take an address in either "1.2.3.4:56", "1.2.3.4", "name:port", or "name" format.  A DNS lookup may be performed.
        void SetAddress(const std::string &addressString);

        //!Sets the packed IP
        inline void SetIP(uint32 ipValue)
        {
            ip=ipValue;
            name.clear();
        }

        //!Sets the port for this address
        inline void SetPort(uint16 portNumber)
        {
            port=portNumber;
        }

        //!comparison
        inline bool operator==(const Address &o) const
        {
            return port==o.port && ip==o.ip;
        }

    private:
        uint32 ip;
        uint16 port;
        mutable std::string name; //cached dns name

        //resets all members to 0
        inline void ZeroOut()
        {
            ip=0;
            name="0.0.0.0";
            port=0;
        }

        //performs dns lookups
        static uint32 DnsNameToIP(const std::string &sourceName);
        static std::string DnsIpToName(uint32 sourceIP);

        //ip string to uint32
        static uint32 ParseIPString(const std::string &ipstr);
    };

    // -- common utility information

    //TODO: handle the case of multiple network adapters

    //!Retrieves the IP address of the current machine.  This will likely be the IP on the local network, and not the internet-facing IP.
    std::string GetLocalIP();

    //!Retrieves the IP address for broadcasting to the current subnet.
    std::string GetNetworkBroadcastIP();

    //!Retrieves the IP address for broadcasting to directly physically connected devices.
    std::string GetDirectBroadcastIP();
}

#endif //#ifdef MPMA_COMPILE_NET
