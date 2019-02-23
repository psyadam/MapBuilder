//!\file UPNP.cpp Utilities to help poke holes in evil NATs.
//written by Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

//This implementation is based off spec information from http://zbowling.com/projects/upnp/

#include "UPNP.h"

#ifdef MPMA_COMPILE_NET

#include "UDP.h"
#include "TCP.h"
#include "../Setup.h"
#include "../base/Memory.h"
#include "../base/Locks.h"
#include "../base/DebugRouter.h"
#include "../base/Timer.h"
#include "../base/MiscStuff.h"
#include "PlatformSockets.h"

#include <algorithm>
#include <cctype>

//#define VERBOSE_UPNP

//local data
namespace
{
    //device info
    bool didDeviceSearch=false;
    bool isAvailable=false;
    std::string controlUrl;
    std::string serviceUrn;
    NET::Address controlAddress;

    //cached data
    std::string routerName;
    std::string internetIP;

    //
    MPMA::MutexLock *upnpLock;

    //auto init code
    void DoUpnpInitialize()
    {
        upnpLock=new3(MPMA::MutexLock);
    }

    void DoUpnpShutdown()
    {
        delete3(upnpLock);
        upnpLock=0;
    }

    class AutoInitUpnp
    {
    public:
        //hookup init callbacks
        AutoInitUpnp()
        {
            MPMA::Internal_AddInitCallback(DoUpnpInitialize, 1010);
            MPMA::Internal_AddShutdownCallback(DoUpnpShutdown, 1010);
        }
    } autoInitUpnp;

    //information on a device that responded to our initial broadcast
    struct BroadcastResponse
    {
        NET::Address address;
        std::string url;

        inline bool operator==(const BroadcastResponse &o) const
        { return url==o.url; }
    };

    //parses an xml blob and returns a list of everything inside tags with a specific name
    void ParseXmlForTag(const std::string &inXmlBlob, const std::string &inTagName, std::list<std::string> &outList)
    {
        std::string lowerXmlBlob=MISC::MakeLower(inXmlBlob);
        std::string lowerTag=MISC::MakeLower(inTagName);
        std::string lowerTagOpenString="<"+lowerTag+">";
        std::string lowerTagCloseString="</"+lowerTag+">";

        int pos=0;
        while (pos<(int)inXmlBlob.size())
        {
            //find the next pair
            int tagStart=MISC::IndexOf(lowerXmlBlob, lowerTagOpenString, pos);
            int tagEnd=MISC::IndexOf(lowerXmlBlob, lowerTagCloseString, pos);
            if (tagStart==-1 || tagEnd==-1 || tagEnd<tagStart)
                break;

            //we want everything between that pair
            int startPos=tagStart+(int)lowerTagOpenString.size();
            std::string data=inXmlBlob.substr(startPos, tagEnd-startPos);
            outList.emplace_back(std::move(data));

            pos=tagEnd+(int)lowerTagCloseString.size();
        }
    }

    //returns the first result from ParseXmlForTag, or an empty string if none
    std::string ParseXmlForFirstTag(const std::string &inXmlBlob, const std::string &inTagName)
    {
        std::list<std::string> list;
        ParseXmlForTag(inXmlBlob, inTagName, list);
        if (list.empty())
            return "";

        return list.front();
    }

    //uses upnp to open a port
    bool ClaimPortWithUPNP(bool isTcp, uint16 port);

    //uses upnp to release a port
    void ReleasePortWithUPNP(bool isTcp, uint16 port);

    //uses ninja techniques to open a udp port
    void ClaimUdpPortWithNinjas(uint16 port);

    //uses ninja techniques to open a tcp port
    void ClaimTcpPortWithNinjas(uint16 port);
}

//
namespace NET
{
    namespace UPNP
    {
        //Whether the framework should attempt to try other nat-opening techniques if UPNP is not available when either of the Claim*Port functions are called.
        bool AllowNinjaNatTechniques=true;

        //Returns whether UPNP service is available on the network.  The initial call will block for a short time while waiting for information from the network.  If this returns false, all other functions here will likely fail.
        bool IsAvailable()
        {
            //if we already searched, return the cached result
            if (didDeviceSearch)
                return isAvailable;

            //set up a udp client
            MPMA::TakeMutexLock autoLock(*upnpLock);

            isAvailable=false;
            UDPClient *udp=UDPClient::Create(0, true, false);
            if (!udp)
                MPMA::ErrorReport()<<"Failed to create UDPClient for UPNP use.\n";

            for (;udp;) //scope to break to cleanup
            {
                //now broadcast the message and read responses... after 250ms we'll send again and wait another 250ms.  during this time we'll parse responses and save the ones we care about.
                const char outSearchMessage[]=
                    "M-SEARCH * HTTP/1.1\r\n"
                    "Host:239.255.255.250:1900\r\n"
                    "ST:upnp:rootdevice\r\n"
                    "Man:\"ssdp:discover\"\r\n"
                    "MX:3\r\n"
                    "\r\n"
                    "\r\n";

                std::list<BroadcastResponse> devices;
                for (int i=0; i<2; ++i)
                {
                    //broadcast search
                    if (!udp->Send(Address("239.255.255.250", 1900), outSearchMessage, strlen(outSearchMessage)))
                    {
                        MPMA::ErrorReport()<<"Failed to broadcast UPNP search packet.\n";
                        i=2;
                        break;
                    }
                    else
                    {
                        //collect responses
                        MPMA::Timer timer;
                        while (timer.Step(false)<0.25)
                        {
                            std::vector<uint8> response;
                            Address addr;
                            while (udp->Receive(response, &addr))
                            {
                                response.push_back(0);
                                std::string resStr=(char*)&response[0];
#ifdef VERBOSE_UPNP
                                MPMA::ErrorReport()<<"Response to UPNP search broadcast: \n"<<resStr;
#endif
                                //chop it up by lines and store any devices we care about
                                bool validDevice=false;
                                BroadcastResponse br;
                                br.address=addr;

                                std::vector<std::string> lines;
                                MISC::ExplodeString(resStr, lines, "\n\r");
                                for (std::vector<std::string>::iterator rawLine=lines.begin(); rawLine!=lines.end(); ++rawLine)
                                {
                                    std::string line=MISC::StripPadding(*rawLine);
                                    std::string lowerLine=line;
                                    std::transform(line.begin(), line.end(), lowerLine.begin(), tolower);

                                    if (MISC::StartsWith(lowerLine, "location:"))
                                    {
                                        line.push_back(0);
                                        br.url=MISC::StripPadding(&line[strlen("location:")]);

                                        validDevice=true; //only location is enough for now... don't require magic inside the usn header (my router doesn't have anything about wanipconnection in it)
                                    }
                                }

                                //if it's the kind of device we want and has info we need, store it
                                if (validDevice && !br.url.empty())
                                {
                                    if (std::find(devices.begin(), devices.end(), br)==devices.end())
                                        devices.emplace_back(std::move(br));
                                }
                            }

                            MPMA::Sleep(1);
                        }
                    }
                }

                //now go through any devices we found and poke them for more info until we find one that we want
#ifdef VERBOSE_UPNP
                MPMA::ErrorReport()<<"Found "<<devices.size()<<" UPNP devices.\n";
#endif
                if (devices.size()==0)
                    break;

                for (std::list<BroadcastResponse>::iterator br=devices.begin(); br!=devices.end(); ++br)
                {
                    //breakup the url into it's parts
                    std::string host=br->url;
                    std::transform(host.begin(), host.end(), host.begin(), tolower);
                    std::string uri;

                    if (MISC::StartsWith(host, "http://")) //strip http:// prefix off
                        host=&host[strlen("http://")];

                    std::string::iterator uriStart=std::find(host.begin(), host.end(), '/');
                    if (uriStart==host.end())
                        uri="/";
                    else
                    {
                        //divide the host from the uri
                        nuint hostLen=uriStart-host.begin();
                        host.push_back(0);
                        uri=&host[hostLen];
                        host.resize(hostLen);
                    }

                    //now talk to them with a normal http request to get their xml info blob
#ifdef VERBOSE_UPNP
                    MPMA::ErrorReport()<<"Querying host="<<host<<" and uri="<<uri<<" for info.\n";
#endif
                    Address deviceAddr(host);
                    TCPClient *tcp=TCPClient::Connect(deviceAddr);
                    if (tcp)
                    {
                        //send the request
                        std::string request="GET ";
                        request+=uri;
                        request+=" HTTP/1.1\r\n";
                        request+="Host: ";
                        request+=host;
                        request+="\r\n";
                        request+="Accept-Language: en-us,en\r\n";
                        request+="User-Agent: MPMA/1.0\r\n";
                        request+="\r\n\r\n";

                        tcp->Send(&request[0], request.size());

                        //read response the full response, but don't waste too much time waiting on them in case they decide to be evil
                        MPMA::Timer timer;
                        std::vector<uint8> responseBytes;
                        while (timer.Step(false)<5)
                        {
                            if (tcp->Receive(responseBytes))
                            {
                                //check for the closing root node to see that they're done (some routers don't send content-length header)
                                responseBytes.push_back(0);
                                std::string cur=MISC::MakeLower(MISC::StripPadding((char*)&responseBytes[0]));
                                responseBytes.pop_back();

                                if (MISC::EndsWith(cur, "</root>"))
                                    break;
                            }
                            else if (!tcp->IsConnected())
                                break;

                            MPMA::Sleep(1);
                        }

                        TCPClient::FreeClient(tcp);
                        tcp=0;

                        responseBytes.push_back(0);
                        std::string response=(char*)&responseBytes[0];
#ifdef VERBOSE_UPNP
                        MPMA::ErrorReport()<<"Response to query: \n"<<response<<"\n";
#endif

                        //get all sub-devices xml blobs for this device
                        std::list<std::string> subDeviceBlobs;
                        ParseXmlForTag(response, "device", subDeviceBlobs);

                        //now look at the services that each one offers
                        for (std::list<std::string>::iterator subDevBlob=subDeviceBlobs.begin(); subDevBlob!=subDeviceBlobs.end(); ++subDevBlob)
                        {
                            std::list<std::string> serviceBlobs;
                            ParseXmlForTag(*subDevBlob, "service", serviceBlobs);

                            for (std::list<std::string>::iterator serviceBlob=serviceBlobs.begin(); serviceBlob!=serviceBlobs.end(); ++serviceBlob)
                            {
                                //we want one with a WANIPConnection service type
                                std::list<std::string> serviceType;
                                ParseXmlForTag(*serviceBlob, "serviceType", serviceType);

                                for (std::list<std::string>::iterator service=serviceType.begin(); service!=serviceType.end(); ++service)
                                {
                                    std::string lowerService=MISC::MakeLower(*service);
                                    if (MISC::Contains(lowerService, "service:wanipconnection"))
                                    {
                                        //we found the one we want, so store some info about it
                                        routerName=ParseXmlForFirstTag(response, "manufacturer");
                                        if (!routerName.empty()) routerName+=" ";
                                        routerName+=ParseXmlForFirstTag(response, "modelName");

                                        serviceUrn=MISC::StripPadding(*service);
                                        controlUrl=MISC::StripPadding(ParseXmlForFirstTag(*serviceBlob, "controlURL"));
                                        controlAddress=deviceAddr;

                                        if (!controlUrl.empty())
                                        {
                                            isAvailable=true;
#ifdef VERBOSE_UPNP
                                            MPMA::ErrorReport()<<"Using UPNP device: "<<routerName<<"  control="<<controlUrl<<"  address="<<controlAddress.GetIPPortString()<<"\n";
#endif
                                        }
                                    }
                                }

                                if (isAvailable)
                                    break;
                            }

                            if (isAvailable)
                                break;
                        }
                    }

                    if (isAvailable)
                        break;
                }

                break;
            }

            //cleanup
            if (udp)
                UDPClient::FreeClient(udp);

            didDeviceSearch=true;
            return isAvailable;
        }

        //Retrieves the public internet-facing IP of the local network.  If that information is not available, the IP of the local machine is returned instead.
        std::string GetInternetFacingIP()
        {
            if (!IsAvailable())
                return GetLocalIP();

            if (!internetIP.empty())
                return internetIP;

            //open a connection to the device and ask it for our internet-facing IP
            TCPClient *tcp=TCPClient::Connect(controlAddress);
            if (!tcp)
            {
                MPMA::ErrorReport()<<"Failed to open connection to UPNP device to ask for our ip."<<"\n";
                internetIP=GetLocalIP();
                return internetIP;
            }

            std::string body="<s:Envelope\r\n"
                             "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
                             "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
                             "<s:Body>\r\n"
                             "<u:GetExternalIPAddress\r\n"
                             "xmlns:u=\""+serviceUrn+"\">\r\n"
                             "</u:GetExternalIPAddress>\r\n"
                             "</s:Body>\r\n"
                             "</s:Envelope>\r\n\r\n";
            std::string header="POST "+controlUrl+" HTTP/1.1\r\n"
                               "Host: "+controlAddress.GetIPPortString()+"\r\n"
                               "User-Agent: MPMA/1.0\r\n"
                               "Content-Length: "+MPMA::VaryString(body.size())+"\r\n"
                               "SOAPACTION: \""+serviceUrn+"#GetExternalIPAddress\"\r\n\r\n";
            std::string request=header+body;

            std::vector<uint8> responseBytes;
            for (int i=0; i<2; ++i) //try 2x, the second time twiddling the post method name
            {
                if (i==1)
                    header="M-"+header;

                tcp->Send(&request[0], request.size());

                //lets see what it said
                MPMA::Timer timer;
                while (timer.Step(false)<3)
                {
                    if (tcp->Receive(responseBytes))
                    {
                        //check for what we want to see if they're done (some routers don't send content-length header)
                        responseBytes.push_back(0);
                        std::string cur=MISC::MakeLower(MISC::StripPadding((char*)&responseBytes[0]));
                        responseBytes.pop_back();

                        std::string ip=ParseXmlForFirstTag(cur, "newexternalipaddress");
                        if (!ip.empty())
                        {
                            internetIP=Address(ip).GetIPString(); //reconvert to ensure it's in the format we expect
                            break;
                        }
                    }
                    else if (!tcp->IsConnected())
                    {
                        MPMA::ErrorReport()<<"Lost connection while trying to receive upnp ext ip response.\n";
                        break;
                    }

                    MPMA::Sleep(1);
                }

                if (!internetIP.empty())
                {
                    break;
                }
            }

            TCPClient::FreeClient(tcp);
            tcp=0;

            //verify that what we got is sane.. if not revert to using local IP
            if (internetIP=="0.0.0.0" || internetIP=="255.255.255.255")
                internetIP.clear();

            if (internetIP.empty())
            {
                MPMA::ErrorReport()<<"Got "<<(responseBytes.empty()?"no":"unexpected")<<" response from UPNP device in request for external IP.";
                MPMA::ErrorReport()<<"We sent:\n"<<request<<"\n";
                if (!responseBytes.empty())
                {
                    responseBytes.push_back(0);
                    std::string response=(char*)&responseBytes[0];
                    MPMA::ErrorReport()<<"We got back:\n"<<response<<"\n";
                }

                internetIP=GetLocalIP();
            }

            return internetIP;
        }

        //Returns the name of the router device that we are talking to with UPNP.
        std::string GetDeviceName()
        {
            if (!IsAvailable())
                return "";

            return routerName;
        }

        //Instructs the router to redirect all UDP packets sent to a specific port to us.
        void ClaimUDPPort(uint16 port)
        {
            if (!IsAvailable())
            {
                if (AllowNinjaNatTechniques)
                    ClaimUdpPortWithNinjas(port);
                return;
            }

            if (!ClaimPortWithUPNP(false, port) && AllowNinjaNatTechniques)
                ClaimUdpPortWithNinjas(port);
        }

        //Instructs the router to redirect all TCP connection attempts to a specific port to us.
        void ClaimTCPPort(uint16 port)
        {
            if (!IsAvailable())
            {
                if (AllowNinjaNatTechniques)
                    ClaimTcpPortWithNinjas(port);
                return;
            }

            if (!ClaimPortWithUPNP(true, port) && AllowNinjaNatTechniques)
                ClaimTcpPortWithNinjas(port);
        }

        //Instructs the router that we are no longer interested in a specific udp port.
        void ReleaseUDPPort(uint16 port)
        {
            if (IsAvailable())
                ReleasePortWithUPNP(false, port);
        }

        //Instructs the router that we are no longer interested in a specific tcp port.
        void ReleaseTCPPort(uint16 port)
        {
            if (IsAvailable())
                ReleasePortWithUPNP(true, port);
        }
    }
}

namespace
{
    //uses upnp to open a port
    bool ClaimPortWithUPNP(bool isTcp, uint16 port)
    {
        bool success=false;

        //open a connection to the device and ask it forward the port to us
        NET::TCPClient *tcp=NET::TCPClient::Connect(controlAddress);
        if (!tcp)
        {
            MPMA::ErrorReport()<<"Failed to open connection to UPNP device to add port mapping."<<"\n";
            return false;
        }

        std::string body="<s:Envelope\r\n"
                         "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
                         "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
                         "<s:Body>\r\n"
                         "<u:AddPortMapping\r\n"
                         "xmlns:u=\""+serviceUrn+"\">\r\n"
                         "<NewRemoteHost></NewRemoteHost>\r\n"
                         "<NewExternalPort>"+MPMA::VaryString(port)+"</NewExternalPort>\r\n"
                         "<NewProtocol>"+(isTcp?"TCP":"UDP")+"</NewProtocol>\r\n"
                         "<NewInternalPort>"+MPMA::VaryString(port)+"</NewInternalPort>\r\n"
                         "<NewInternalClient>"+NET::GetLocalIP()+"</NewInternalClient>\r\n"
                         "<NewEnabled>1</NewEnabled>\r\n"
                         "<NewPortMappingDescription>MPMA "+NET::GetLocalIP()+":"+MPMA::VaryString(port)+" "+(isTcp?"TCP":"UDP")+"</NewPortMappingDescription>\r\n"
                         "<NewLeaseDuration>0</NewLeaseDuration>\r\n"
                         "</u:AddPortMapping>\r\n"
                         "</s:Body>\r\n"
                         "</s:Envelope>\r\n\r\n";
        std::string header="POST "+controlUrl+" HTTP/1.1\r\n"
                           "Host: "+controlAddress.GetIPPortString()+"\r\n"
                           "User-Agent: MPMA/1.0\r\n"
                           "Content-Length: "+MPMA::VaryString(body.size())+"\r\n"
                           "SOAPACTION: \""+serviceUrn+"#AddPortMapping\"\r\n\r\n";
        std::string request=header+body;

        std::string response;
        std::vector<uint8> responseBytes;
        for (int i=0; i<2; ++i) //try 2x, the second time twiddling the post method name
        {
            if (i==1)
                header="M-"+header;

            tcp->Send(&request[0], request.size());

            //lets see what it said
            MPMA::Timer timer;
            while (timer.Step(false)<3)
            {
                if (tcp->Receive(responseBytes))
                {
                    //check for what we want to see if they're done (some routers don't send content-length header)
                    responseBytes.push_back(0);
                    std::string cur=MISC::MakeLower(MISC::StripPadding((char*)&responseBytes[0]));
                    responseBytes.pop_back();

                    std::string bodypart=ParseXmlForFirstTag(cur, "s:Body");
                    if (!bodypart.empty())
                    {
                        break;
                    }
                }
                else if (!tcp->IsConnected())
                {
                    MPMA::ErrorReport()<<"Lost connection while trying to receive port map response.\n";
                    break;
                }

                MPMA::Sleep(1);
            }

            //parse the first line for the http response
            responseBytes.push_back(0);
            response=(char*)&responseBytes[0];
            std::vector<std::string> responseLines;
            MISC::ExplodeString(response, responseLines, "\r\n");
            if (!responseLines.empty())
            {
                //a response will look something like "HTTP/1.1 200 OK"... we want that number.
                std::string httpResponse=responseLines.front();
                std::vector<std::string> responseParts;
                MISC::ExplodeString(httpResponse, responseParts, " ");
                if (responseParts.size()>=2)
                {
                    if (responseParts[1]=="200") //goodness
                    {
                        success=true;
                    }
                }
            }

            if (success)
                break;
        }

        NET::TCPClient::FreeClient(tcp);
        tcp=0;

        if (!success)
            MPMA::ErrorReport()<<"Failed to map port with UPNP device.  "<<(response.empty()?"No response":"Response:\n"+response)<<"\n";

        return success;
    }

    //uses upnp to release a port
    void ReleasePortWithUPNP(bool isTcp, uint16 port)
    {
        //open a connection to the device and tell it we don't care about a port anymore.
        NET::TCPClient *tcp=NET::TCPClient::Connect(controlAddress);
        if (!tcp)
        {
            MPMA::ErrorReport()<<"Failed to open connection to UPNP device to delete port mapping."<<"\n";
            return;
        }

        std::string body="<s:Envelope\r\n"
                         "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n"
                         "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
                         "<s:Body>\r\n"
                         "<u:DeletePortMapping\r\n"
                         "xmlns:u=\""+serviceUrn+"\">\r\n"
                         "<NewRemoteHost></NewRemoteHost>\r\n"
                         "<NewExternalPort>"+MPMA::VaryString(port)+"</NewExternalPort>\r\n"
                         "<NewProtocol>"+(isTcp?"TCP":"UDP")+"</NewProtocol>\r\n"
                         "</u:DeletePortMapping>\r\n"
                         "</s:Body>\r\n"
                         "</s:Envelope>\r\n\r\n";
        std::string header="POST "+controlUrl+" HTTP/1.1\r\n"
                           "Host: "+controlAddress.GetIPPortString()+"\r\n"
                           "User-Agent: MPMA/1.0\r\n"
                           "Content-Length: "+MPMA::VaryString(body.size())+"\r\n"
                           "SOAPACTION: \""+serviceUrn+"#DeletePortMapping\"\r\n\r\n";
        std::string request=header+body;

        //send it 2x, using both headers.  we don't care about the response since there's nothing we can really do about it.
        for (int i=0; i<2; ++i) //try 2x, the second time twiddling the post method name
        {
            if (i==1)
                header="M-"+header;

            tcp->Send(&request[0], request.size());
        }

        NET::TCPClient::FreeClient(tcp);
    }

    //uses ninja techniques to open a udp port
    void ClaimUdpPortWithNinjas(uint16 port)
    {
        //we need a destination to send something to.. if they didn't provide one, use an arbitrarily picked internet ip as a target
        std::string destIP="2.3.4.5";

        //first try to bind to the port... and send a packet from there
        NET::UDPClient *udp=NET::UDPClient::Create(port, false, false);
        if (udp)
        {
            char o=0;
            udp->Send(NET::Address(destIP), &o, 1);
            NET::UDPClient::FreeClient(udp);
            return;
        }

        //drat.. it was already used.. so try to at least send "to" that port and hope for the best
        udp=NET::UDPClient::Create(0, false, false);
        if (udp)
        {
            char o=0;
            udp->Send(NET::Address(destIP, port), &o, 1);
            NET::UDPClient::FreeClient(udp);
            return;
        }

        MPMA::ErrorReport()<<"The UDP Ninjas have failed for port "<<port<<"\n";
    }

    //uses ninja techniques to open a tcp port
    void ClaimTcpPortWithNinjas(uint16 port)
    {
        //we need a destination to send something to.. if they didn't provide one, use an arbitrarily picked internet ip as a target
        std::string destIP=NET::Address("google.com").GetIPString(); //use a real IP for this, since tcp has connection timeout stuff

        //try to bind to the port... and open a connection.  if that doesn't work we're pretty much out of luck.
        NET::TCPClient *tcp=NET::TCPClient::Connect(NET::Address(destIP, 80), port);
        if (tcp)
        {
            char o=0;
            tcp->Send(&o, 1);
            NET::TCPClient::FreeClient(tcp);
            return;
        }

        MPMA::ErrorReport()<<"The TCP Ninjas have failed for port "<<port<<"\n";
    }
}

bool mpmaForceReferenceToUPNPCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_NET
