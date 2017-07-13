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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>

#include <ifaddrs.h>
#include <linux/if.h>
#include <netdb.h>

// ip address, network byte order
typedef uint32_t ip4_address_t;

namespace tis
{

bool isRPFilterActive ();

/// @name startsWith
/// @param searchThrough - string that shall be examined
/// @param searchFor - string that shall be searched
/// @return true if strings starts with search
bool startsWith (const std::string& searchThrough, const std::string& searchFor);

/// @name int2ip
/// @param addr - ip4_address_t containing the IP
/// @return string containing IP
std::string int2ip (const ip4_address_t addr);

/// @name int2mac
/// @param mac - uint64 containing the mac address to be converted
/// @return string containing the mac address
std::string int2mac (const uint64_t mac);

/// @name readHexByte
/// @param beg - char sequence that shall be converted
/// @param end - end of sequence that shall not be exceeded
/// @return int containing the value of first chars
/// @brief reads the first chars of sequence and returns their integer value
unsigned int readHexByte (const char** beg, const char* end);

/// @name mac2int
/// @param mac - string containing the MAC that shall be converted
/// @return uint64_t containing the converted mac
uint64_t mac2int (const std::string& mac);

/// @name ip2int
/// @param ip - string containing the ip to be converted
/// @return ip as int or -1 on failure
ip4_address_t ip2int (const std::string& ip);

/// @name fillAddr
/// @param address - ip address from recipient
/// @param port - port to use
/// @return sockaddr_in
sockaddr_in fillAddr (const std::string &address, const unsigned short port);

/// @name isValidIpAddress
/// @param ipAddress - string containing IP that shall be checked
/// @return true if valid IP
bool isValidIpAddress (const std::string& ipAddress);

/// @name isValidIpMAC
/// @param mac - string containing mac that shall be checked
/// @return true if valid mac
bool isValidMAC (const std::string& mac);

} /* namespace tis */

#endif /* _UTILS_H_ */
