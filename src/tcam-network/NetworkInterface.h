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
    int timeout_ms;

public:

    ~NetworkInterface () = default;

    /// @name NetworkInterface
    /// @param _addrs - ifaddrs struct describing the interface that shall be represented
    /// @brief Constructor
    NetworkInterface (const struct ifaddrs* _addrs, int timeout=1500);

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
