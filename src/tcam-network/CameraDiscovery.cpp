/*
 * Copyright 2013 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream> // cerr

#include "CameraDiscovery.h"
#include "utils.h"
#include "gigevision.h"

#include <linux/if.h>
#include <netdb.h>
#include <cstring>

#include <thread>
#include <algorithm>
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
        // ignore VPN and inactive
        // VPN do not exhibit valid addresses and can cause
        // crashes, therfore jump over them
        if ((addrs->ifa_addr == NULL) || ! (addrs->ifa_flags & IFF_RUNNING))
        {
            addrs = addrs->ifa_next;
            continue;
        }

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


void discoverCameras (std::vector<std::string> selectected_interfaces,
                      std::function<void (std::shared_ptr<Camera>)> const & discover_call)
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
        if (std::find(selectected_interfaces.begin(), selectected_interfaces.end(), inf->getInterfaceName()) != selectected_interfaces.end())
        {
            std::shared_ptr<NetworkInterface> ni = inf;
            thread_list.push_back(std::thread( [inf, discover_call] () { sendDiscovery(inf, discover_call); } ));
        }
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
    try
    {
        auto s = interface->createSocket();
        s->sendAndReceive("255.255.255.255", &discovery_packet, sizeof(discovery_packet), callback, true);
    }
    catch (SocketSendToException& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void sendIpRecovery (const std::string mac, const ip4_address_t ip, const ip4_address_t netmask, const ip4_address_t gateway)
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

    packet.StaticIP = ip;
    packet.StaticSubnetMask = netmask;
    packet.StaticGateway = gateway;

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
    std::vector<std::thread> t;

    // Send through each interface
    for (auto& i :interfaces)
    {
        t.push_back(std::thread( [&] () { bf(i); } ));
    }

    for (auto& th : t)
    {
        th.join();
    }
}

} /* namespace tis */
