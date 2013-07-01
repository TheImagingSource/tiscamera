///
/// @file NetworkInterface.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#ifndef _NETWORKINTERFACE_H_
#define _NETWORKINTERFACE_H_

#include "Socket.h"
#include <memory>
#include <string>
#include <ifaddrs.h>

namespace tis
{

class NetworkInterface
{
private:

    std::string name;
    uint32_t flags;
    uint32_t addr;
    uint32_t netmask;

public:

    ~NetworkInterface () = default;

    /// @name NetworkInterface
    /// @param _addrs - ifaddrs struct describing the interface that shall be represented
    /// @brief Constructor
    NetworkInterface (const struct ifaddrs* _addrs);

    /// @name getInterfaceName
    /// @return string containing the name
    std::string getInterfaceName ();

    /// @name getInterfaceIP
    /// @return uint32_t containing the interface ip
    uint32_t getInterfaceIP ();

    /// @name getInterfaceNetmask
    /// @return uint32_t containing the netmask
    uint32_t getInterfaceNetmask ();

    /// @name createSocket
    /// @return shared_ptr to newly created Socket object
    std::shared_ptr<Socket> createSocket ();

}; /* class NetworkInterface */

} /* namespace tis */

#endif /* _NETWORKINTERFACE_H_ */
