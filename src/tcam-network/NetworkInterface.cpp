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

#include "NetworkInterface.h"
#include "Socket.h"
#include <netinet/in.h>
#include <cstring>
#include <linux/if.h>
#include <netdb.h>


namespace tis
{
    NetworkInterface::NetworkInterface (const struct ifaddrs* _addrs, int timeout)
        : name(), flags(_addrs->ifa_flags), addr(), netmask(0), timeout_ms(timeout)
    {
        if (_addrs->ifa_name != NULL)
        {
            name = std::string(_addrs->ifa_name);
        }

        addr = ((struct sockaddr_in*)_addrs->ifa_addr)->sin_addr.s_addr;

        netmask = ((struct sockaddr_in*)_addrs->ifa_netmask)->sin_addr.s_addr;

    }

    std::string NetworkInterface::getInterfaceName ()
    {
        return name;
    }


    uint32_t NetworkInterface::getInterfaceIP ()
    {
        return addr;
    }


    uint32_t NetworkInterface::getInterfaceNetmask ()
    {
        return netmask;
    }


    std::shared_ptr<Socket> NetworkInterface::createSocket ()
    {
        struct sockaddr_in socka = sockaddr_in();
        // Create address from which we want to send, and bind it
        socka.sin_family = AF_INET;

        struct in_addr address = in_addr();
        address.s_addr = (unsigned long) this->addr;
        socka.sin_addr = address;
        socka.sin_port = 0;

        auto s = std::shared_ptr<Socket>(new Socket(socka, timeout_ms));

        return s;
    }

} /* namespace tis */
