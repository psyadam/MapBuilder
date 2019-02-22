//tests that the geo library functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <iostream>
#include "net/Common.h"
#include "net/UDP.h"
#include "net/TCP.h"
#include "net/UPNP.h"
#include "base/Timer.h"

#ifdef DECLARE_TESTS_CODE
class NetDNS: public UnitTest
{
public:
    bool Run()
    {
        std::string ipstr="";

        //dns to ip lookup
        std::cout<<"Doing dns lookup on www.neoclaw.net..."<<std::endl;
        {
            NET::Address addr("www.neoclaw.net:80");
            if (addr.GetIP()==0)
            {
                std::cout<<"Call failed."<<std::endl;
                return false;
            }
            ipstr=addr.GetIPString();
            std::cout<<addr.GetNameString()<<" resolves to "<<ipstr<<std::endl;
        }

        //ip to dns reverse lookup
        std::cout<<"Doing dns reverse lookup..."<<std::endl;
        {
            NET::Address addr(ipstr);
            std::string namestr=addr.GetNameString();
            std::cout<<ipstr<<" has name "<<namestr<<std::endl;
            if (namestr==ipstr)
            {
                std::cout<<"Name lookup failed, since name matches IP."<<std::endl;
                return false;
            }
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(NetDNS);

#ifdef DECLARE_TESTS_CODE
class NetUDPSendRecv: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;

        //quick check on a random port
        NET::UDPClient *udpRandom=NET::UDPClient::Create();
        if (udpRandom==0)
        {
            std::cout<<"UDPClient on a random port failed to set up."<<std::endl;
            good=false;
        }

        //real test
        NET::UDPClient *udp=NET::UDPClient::Create(13245);
        if (udp==0)
        {
            std::cout<<"UDPClient on a fixed port 12345 failed to set up."<<std::endl;
            good=false;
        }
        
        if (good)
        {
            if (!udp->GetLocalPort())
            {
                std::cout<<"Sender says local port is 0... that's not valid."<<std::endl;
                good=false;
            }
        }

        if (good)
        {
            //do a receive first - it should immediately return
            MPMA::Timer timer;
            std::vector<uint8> data;
            if (udp->Receive(data))
            {
                good=false;
                std::cout<<"Did not expect Receive to return true yet."<<std::endl;
            }

            if (!data.empty())
            {
                good=false;
                std::cout<<"Did not expect any data to be received."<<std::endl;
            }

            if ((float)timer.Step()>0.05f)
            {
                good=false;
                std::cout<<"Receive took tot long to return."<<std::endl;
            }

            //do send
            NET::Address addr("127.0.0.1:13245");
            if (!udp->Send(addr, "hello", 6))
            {
                good=false;
                std::cout<<"Send failed."<<std::endl;
            }

            MPMA::Sleep(20);

            //do receive and verify
            data.clear();
            if (!udp->Receive(data))
            {
                good=false;
                std::cout<<"No data received."<<std::endl;
            }
            else
            {
                if (data.size()!=6 || memcmp(&data[0], "hello", 6)!=0)
                {
                    std::cout<<"Received data does not match what was sent."<<std::endl;
                }
            }
        }

        NET::UDPClient::FreeClient(udp);
        NET::UDPClient::FreeClient(udpRandom);
        return good;
    }
};
#endif
DECLARE_TEST_CASE(NetUDPSendRecv);

#ifdef DECLARE_TESTS_CODE
class NetUDPBroadcast: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;

        //verify that our broadcast interface stuff works right
        std::string directBC=NET::GetDirectBroadcastIP();
        std::string netBC=NET::GetNetworkBroadcastIP();

        std::cout<<"Direct Broadcast IP: "<<directBC<<std::endl;
        std::cout<<"Network Broadcast IP: "<<netBC<<std::endl;

        if (directBC==netBC)
        {
            std::cout<<"Direct and Network broadcast IP are the same... this means there was a problem getting the net broadcast ip."<<std::endl;
            return false;
        }

        //do an actual broadcast test
        NET::UDPClient *send=NET::UDPClient::Create(0, true);
        if (send==0)
        {
            std::cout<<"Failed to create broadcasting udp sender."<<std::endl;
            good=false;
        }
        else
        {
            char packet='x';
            if (!send->Send(NET::Address(NET::GetNetworkBroadcastIP(), 12345), &packet, 1))
            {
                std::cout<<"Failed to broadcast udp packet."<<std::endl;
                good=false;
            }
        }

        NET::UDPClient::FreeClient(send);
        return good;
    }
};
#endif
DECLARE_TEST_CASE(NetUDPBroadcast);

/*#ifdef DECLARE_TESTS_CODE
class NetTCPConnectFailure: public UnitTest
{
public:
    bool Run()
    {
        NET::Address addr("1.2.3.4:31098");
        NET::TCPClient *client=NET::TCPClient::Connect(addr);
        if (client!=0)
        {
            NET::TCPClient::FreeClient(client);
            std::cout<<"Did not expect connection attempt to succeed."<<std::endl;
            return false;
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(NetTCPConnectFailure);*/

#ifdef DECLARE_TESTS_CODE
class NetTCPConnectSendRecv: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;

        NET::TCPServer *server=NET::TCPServer::Listen(13254);
        if (!server)
        {
            good=false;
            std::cout<<"Server failed to start."<<std::endl;
        }
        else
            std::cout<<"Local server started..."<<std::endl;

        NET::TCPClient *client0=0, *client1=0;
        if (good)
        {
            client0=server->GetNextConnection();
            if (client0)
            {
                good=false;
                std::cout<<"Did not expect any connection yet."<<std::endl;
            }
        }

        if (good)
        {
            std::cout<<"Connecting..."<<std::endl;
            client0=NET::TCPClient::Connect(NET::Address("127.0.0.1",13254));
            if (!client0)
            {
                good=false;
                std::cout<<"Failed to connect to local server."<<std::endl;
            }

            MPMA::Sleep(50);
        }

        if (good)
        {
            std::cout<<"Checking server..."<<std::endl;
            client1=server->GetNextConnection();
            if (!client1)
            {
                good=false;
                std::cout<<"Sever did not get connection from client."<<std::endl;
            }
            else
            {
                if (!client0->IsConnected())
                {
                    good=false;
                    std::cout<<"Client 0 says it is not connected."<<std::endl;
                }
                if (!client1->IsConnected())
                {
                    good=false;
                    std::cout<<"Client 1 says it is not connected."<<std::endl;
                }
            }
        }

        std::vector<uint8> data0;
        std::vector<uint8> data1;
        if (good)
        {
            std::cout<<"Client 0 (local port "<<client0->GetLocalPort()<<") is connected to "<<client0->GetRemoteAddress().GetIPPortString()<<"\n";
            std::cout<<"Client 1 (local port "<<client1->GetLocalPort()<<") is connected to "<<client1->GetRemoteAddress().GetIPPortString()<<std::endl;

            client1->EnableSendCoalescing(true); //just because.

            if (client0->Receive(data0) || !data0.empty())
            {
                good=false;
                std::cout<<"Did not expect client to receive any data yet."<<std::endl;
            }

            client0->Send("Hello", 5);
            client1->Send("Hi", 2);

            MPMA::Sleep(50);

            if (!client0->Receive(data0) || data0.empty())
            {
                good=false;
                std::cout<<"Client 0 did not get the message."<<std::endl;
            }
            if (!client1->Receive(data1) || data1.empty())
            {
                good=false;
                std::cout<<"Client 1 did not get the message."<<std::endl;
            }
        }

        if (good)
        {
            if (data0.size()!=2 && memcmp(&data0[0], "Hi", 2)!=0)
            {
                good=false;
                std::cout<<"Client 0 got wrong data."<<std::endl;
            }
            if (data1.size()!=5 && memcmp(&data1[0], "Hello", 5)!=0)
            {
                good=false;
                std::cout<<"Client 1 got wrong data."<<std::endl;
            }
        }

        if (good)
        {
            std::cout<<"Data exchange worked.  Disconnecting..."<<std::endl;

            NET::TCPClient::FreeClient(client0);
            client0=0;

            data1.clear();
            MPMA::Timer timer;
            double elapsed=0;
            while (client1->IsConnected())
            {
                if (client1->Receive(data1) || !data1.empty())
                {
                    good=false;
                    std::cout<<"Client 1 impossibly got data (that wasn't even sent) after disconnect."<<std::endl;
                    break;
                }
                MPMA::Sleep(1);
                elapsed+=timer.Step();

                if (elapsed>10)
                {
                    good=false;
                    std::cout<<"Client 1 did not notice disconnect within 10s."<<std::endl;
                    break;
                }
            }
            if (good)
                std::cout<<"Client 1 got disconnect after "<<(float)elapsed<<"s."<<std::endl;
        }

        NET::TCPClient::FreeClient(client0);
        NET::TCPClient::FreeClient(client1);
        NET::TCPServer::FreeServer(server);
        server=0;

        return good;
    }
};
#endif
DECLARE_TEST_CASE(NetTCPConnectSendRecv);

#ifdef DECLARE_TESTS_CODE
class NetUPNP: public UnitTest
{
public:
    bool Run()
    {
        //if not available.. not much we can do
        if (!NET::UPNP::IsAvailable())
        {
            std::cout<<"UPNP is not available."<<std::endl;
            return true;
        }

        //spit out some info
        std::cout<<"Our local IP is: "<<NET::GetLocalIP()<<std::endl;
        std::cout<<"Our internet-facing IP is: "<<NET::UPNP::GetInternetFacingIP()<<std::endl;
        std::cout<<"Connection device: "<<NET::UPNP::GetDeviceName()<<std::endl;

        //open a couple ports
        NET::UPNP::ClaimUDPPort(23245);
        NET::UPNP::ClaimTCPPort(23241);

        //close them back up
        NET::UPNP::ReleaseUDPPort(23245);
        NET::UPNP::ReleaseTCPPort(23241);

        //validate ext ip
        if (NET::GetLocalIP()==NET::UPNP::GetInternetFacingIP())
        {
            std::cout<<"Internal and external IPs are the same.. that's probably not right."<<std::endl;
            return false;
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(NetUPNP);
