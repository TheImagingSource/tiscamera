///
/// @file NetworkInterface.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "NetworkInterface.h"
#include "Socket.h"
#include <netinet/in.h>
#include <cstring>
#include <linux/if.h>
#include <netdb.h>


namespace tis
{
    NetworkInterface::NetworkInterface (const struct ifaddrs* _addrs)
    {
        if (_addrs->ifa_name != NULL)
        {
            name = std::string(_addrs->ifa_name);
        }

        flags = _addrs->ifa_flags;
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

        auto s = std::shared_ptr<Socket>(new Socket(socka));

        return s;
    }

} /* namespace tis */
