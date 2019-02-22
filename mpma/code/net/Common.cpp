//common structs used by different protocols
//written by Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "Common.h"

#ifdef MPMA_COMPILE_NET

#include "../base/Vary.h"
#include "../base/DebugRouter.h"
#include "PlatformSockets.h"

using namespace NET_INTERNAL;

//local data
namespace
{
    //cached information
    std::string cachedBroadcastIP;
    std::string cachedLocalIP;
}

//
namespace NET
{
    //Sets the address - can take an address in either "1.2.3.4:56", "1.2.3.4", "name:port", or "name" format.  A DNS lookup may be performed.
    void Address::SetAddress(const std::string &addressString)
    {
        name.clear();
        ip=0;

        //break it into address part and port part
        std::string addrPart, portPart;
        bool hitColon=false;
        for (std::string::const_iterator i=addressString.begin(); i!=addressString.end(); ++i)
        {
            if (*i==':')
                hitColon=true;
            else if (!hitColon)
                addrPart.push_back(*i);
            else
                portPart.push_back(*i);
        }

        //set port
        if (!portPart.empty())
            port=(uint16)(int)MPMA::Vary(portPart);

        //determine if the address is a name or ip
        if (!addrPart.empty())
        {
            bool isIP=true;
            int dotCount=0;
            for (std::string::const_iterator i=addrPart.begin(); i!=addrPart.end(); ++i)
            {
                if (*i=='.')
                    ++dotCount;
                else if (*i<'0' || *i>'9')
                {
                    isIP=false;
                    break;
                }
            }

            if (dotCount!=3)
                isIP=false;

            //if it's an IP then set it. else do dns lookup
            if (isIP)
                ip=ParseIPString(addrPart);
            else //dns name
                ip=DnsNameToIP(addrPart);
        }
    }

    //Retrieves a string that represents the IP address
    std::string Address::GetIPString() const
    {
        MPMA::Vary s;
        s+=(ip&0xff000000)>>24;
        s+=".";
        s+=(ip&0x0ff0000)>>16;
        s+=".";
        s+=(ip&0x0000ff00)>>8;
        s+=".";
        s+=(ip&0x000000ff);
        return (const std::string&)s;
    }

    //Retrieves a string that represents the IP address and port in "1.2.3.4:56" format
    std::string Address::GetIPPortString() const
    {
        MPMA::VaryString s=GetIPString();
        s+=":";
        s+=port;
        return (const std::string&)s;
    }

    //Retrieves the string that represents the name of the address
    std::string Address::GetNameString() const
    {
        if (name.empty()) //do DNS lookup
            name=DnsIpToName(ip);

        return name;
    }

    //Retrieves a string that represents the name and port in "name.net:56" format
    std::string Address::GetNamePortString() const
    {
        MPMA::VaryString s=GetNameString();
        s+=":";
        s+=port;
        return (const std::string&)s;
    }

    //convert a name to an IP
    uint32 Address::DnsNameToIP(const std::string &sourceName)
    {
        hostent *he=gethostbyname(sourceName.c_str());
        if (he==0)
        {
            MPMA::ErrorReport()<<"DnsNameToIP: gethostbyname failed for: "<<sourceName<<"\n";
            return 0;
        }

        return ParseIPString(inet_ntoa(*(in_addr*)(he->h_addr_list[0])));
    }

    //convert an IP to a name
    std::string Address::DnsIpToName(uint32 sourceIP)
    {
        Address convert;
        convert.SetIP(sourceIP);
        std::string ipstr=convert.GetIPString();

        in_addr addr;
        addr.s_addr=inet_addr(ipstr.c_str());
        hostent *he=gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
        if (he==0)
        {
            MPMA::ErrorReport()<<"DnsIpToName: gethostbyaddr failed for: "<<ipstr<<"\n";
            return ipstr;
        }

        return he->h_name;
    }

    //ip string to uint32
    uint32 Address::ParseIPString(const std::string &ipstr)
    {
        std::string ipChunks[4];
        int dotCount=0;
        for (std::string::const_iterator i=ipstr.begin(); i!=ipstr.end(); ++i)
        {
            if (*i=='.')
                ++dotCount;
            else if (dotCount<=3)
                ipChunks[dotCount].push_back(*i);
        }

        uint32 ip=0;
        ip|=(uint8)(int)MPMA::Vary(ipChunks[3]);
        ip|=((uint32)(uint8)(int)MPMA::Vary(ipChunks[2]))<<8;
        ip|=((uint32)(uint8)(int)MPMA::Vary(ipChunks[1]))<<16;
        ip|=((uint32)(uint8)(int)MPMA::Vary(ipChunks[0]))<<24;
        return ip;
    }

    //Retrieves the IP address of the current machine.  This will likely be the IP on the local network, and not the internet-facing IP.
    std::string GetLocalIP()
    {
        if (!cachedLocalIP.empty())
            return cachedLocalIP;

        //get info about the network adapter
        uint32 netmask, net, localIp;
        if (Internal_GetSubnet(netmask, net, localIp))
        {
            cachedLocalIP=inet_ntoa(*(in_addr*)&localIp);
            return cachedLocalIP;
        }

        //that failed... so fallback to asking for our hostname's ip
        char hostName[256];
        gethostname(hostName, 256);

        hostent *host;
        host=gethostbyname(hostName);

        cachedLocalIP=inet_ntoa(*(in_addr*)*host->h_addr_list);
        return cachedLocalIP;
    }

    //!Retrieves the IP address for network broadcasts for the current network.
    std::string GetNetworkBroadcastIP()
    {
        //use cache if possible
        if (!cachedBroadcastIP.empty())
            return cachedBroadcastIP;

        //get info about the subnet
        uint32 netmask, net, localIp;
        if (!Internal_GetSubnet(netmask, net, localIp))
        {
            MPMA::ErrorReport()<<"GetSubnet failed, so using direct broadcast IP instead.\n";
            cachedBroadcastIP=GetDirectBroadcastIP();
            return cachedBroadcastIP;
        }

        //set all 1's in the host bits of the net mask
        uint32 broadcast=net;
        for (int bit=0; bit<32; ++bit)
        {
            uint32 bitmask=1<<(32-bit-1);
            if (bitmask&netmask)
                break;

            broadcast|=bitmask;
        }

        //cache it
        cachedBroadcastIP=inet_ntoa(*(in_addr*)&broadcast);
        return cachedBroadcastIP;
    }

    //!Retrieves the global broadcast address.
    std::string GetDirectBroadcastIP()
    {
        return "255.255.255.255";
    }
}

#endif //#ifdef MPMA_COMPILE_NET
