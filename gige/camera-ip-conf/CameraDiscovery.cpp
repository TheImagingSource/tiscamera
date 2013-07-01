///
/// @file CameraDiscovery.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include <iostream> // cerr

#include "CameraDiscovery.h"
#include "utils.h"
#include "gigevision.h"

#include <linux/if.h>
#include <netdb.h>
#include <cstring>

#include <thread>
#include <errno.h>

namespace tis
{

std::vector<std::shared_ptr<NetworkInterface>> detectNetworkInterfaces ()
{
    struct ifaddrs* addrs;
    struct ifaddrs* addrs_del;
    std::vector<std::shared_ptr<NetworkInterface>> interfaces;

    // detectInterfaces
    int ret = getifaddrs(&addrs);

    if (ret != 0)
    {
        return interfaces;
    }
    addrs_del = addrs;

    // to get wanted device we iterate over all devices
    // check the family and the device name
    while (addrs != NULL)
    {
        if  ( addrs->ifa_addr->sa_family == AF_INET
              && !(addrs->ifa_flags & IFF_LOOPBACK)
              && !(addrs->ifa_flags & IFF_POINTOPOINT)
              &&  (addrs->ifa_flags & IFF_BROADCAST) )
        {
            auto inf = std::shared_ptr<NetworkInterface>(new NetworkInterface(addrs));
            interfaces.push_back(inf);
        }
        addrs = addrs->ifa_next;
    }

    freeifaddrs(addrs_del);
    return interfaces;
}


void discoverCameras (std::function<void (std::shared_ptr<Camera>)> const & discover_call)
{
    std::vector<std::shared_ptr<Camera>> cameras;
    auto interfaces = detectNetworkInterfaces();

    if (interfaces.empty())
    {
        return;
    }

    std::vector<std::thread> thread_list;
    // allocate whole in one step
    thread_list.reserve(interfaces.size());

    for (auto& inf : interfaces)
    {
        std::shared_ptr<NetworkInterface> ni = inf;
        thread_list.push_back(std::thread( [inf, discover_call] () { sendDiscovery(inf, discover_call); } ));
    }

    for (auto& thr : thread_list)
    {
        thr.join();
    }
}


void sendDiscovery (std::shared_ptr<NetworkInterface> interface, std::function<void (std::shared_ptr<Camera>)> const & discover_call)
{
    Packet::CMD_DISCOVERY discovery_packet;
    discovery_packet.header.command = htons( Commands::DISCOVERY_CMD );
    discovery_packet.header.flag    = Flags::NEEDACK;
    discovery_packet.header.length  = htons( 0 );
    discovery_packet.header.magic   = 0x42;
    discovery_packet.header.req_id  = htons( 1 );

    auto callback = [&interface, &discover_call] (void* msg)
        {
            Packet::ACK_DISCOVERY* ack = (Packet::ACK_DISCOVERY*) msg;
            auto cam = std::shared_ptr<Camera>(new Camera(*ack, interface));
            discover_call(cam);
            return Socket::SendAndReceiveSignals::CONTINUE;
        };

    auto s = interface->createSocket();

    try
    {
        s->sendAndReceive("255.255.255.255", &discovery_packet, sizeof(discovery_packet), callback, true);
    }
    catch (SocketSendToException& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void sendIpRecovery (const std::string mac, const uint32_t ip, const uint32_t netmask, const uint32_t gateway)
{
    int id = 2;
    uint64_t macmac  = mac2int(mac);
    uint16_t machigh = macmac >> 32 & 0xFFFF;
    uint32_t maclow  = macmac & 0xFFFFFFFF;

    Packet::CMD_FORCEIP packet;

    packet.header.magic = 0x42;
    packet.header.flag = Flags::NEEDACK;
    packet.header.command = htons(Commands::FORCEIP_CMD);
    packet.header.length = htons(sizeof(Packet::CMD_FORCEIP)-sizeof(Packet::COMMAND_HEADER));
    packet.header.req_id = id;

    packet.DeviceMACHigh = htons(machigh) ;
    packet.DeviceMACLow = htonl(maclow) ;

    packet.StaticIP = htonl(ip);
    packet.StaticSubnetMask = htonl(netmask);
    packet.StaticGateway = htonl(gateway);

    auto bf = [&] (std::shared_ptr<NetworkInterface> interface)
        {
            auto s = interface->createSocket();
            try
            {
                s->sendAndReceive("255.255.255.255", &packet, sizeof(packet), NULL, true);
            }
            catch (SocketSendToException& e)
            {

            }
        };

    auto interfaces = detectNetworkInterfaces();
    std::thread t[interfaces.size()];

    // Send through each interface
    for (unsigned int i = 0; i < interfaces.size(); i++)
    {
        t[i] = std::thread( [&] () { bf(interfaces.at(i)); } );
    }

    for (unsigned int i = 0; i < interfaces.size(); i++)
    {
        t[i].join();
    }
}

} /* namespace tis */
