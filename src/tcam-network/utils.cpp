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

#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <glob.h>
#include <cstring>
#include <sstream>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <mutex>

namespace tis
{

bool isRPFilterActive ()
{
    bool ret = false;
    glob_t glob_result;
    glob("/proc/sys/net/ipv4/conf/**/rp_filter", GLOB_TILDE, NULL, &glob_result);
    for(unsigned int i=0; i < glob_result.gl_pathc; ++i)
    {
        std::string s;
        std::ifstream f;
        f.open(glob_result.gl_pathv[i]);

        getline(f, s);
        if (s.compare("1") == 0)
        {
            ret = true;
            f.close();
            break;
        }

        f.close();
    }
    globfree(&glob_result);

    return ret;
}


bool startsWith (const std::string& searchThrough, const std::string& searchFor)
{
    if (searchThrough.empty() || searchFor.empty())
    {
        return false;
    }

    size_t lensearchThrough = searchThrough.size();
    size_t lenstr = searchFor.size();
    if (lenstr > lensearchThrough)
    {
        return false;
    }
    else
    {
        if (searchThrough.compare(0, lenstr, searchFor) == 0)
        {
            return true;
        }
        return false;
    }
}


bool isValidIpAddress (const std::string& ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_aton(ipAddress.c_str(), &(sa.sin_addr));

    return result != 0;
}


std::string int2ip (const ip4_address_t addr)
{
    struct in_addr addrs = {addr};

    // inet_ntoa expects network byte order
    std::string a = inet_ntoa(addrs);
    return a;
}


std::string int2mac (const uint64_t mac)
{
    std::stringstream s;
    s << std::hex << mac;
    std::string m = s.str();

    // standardize mac to 12 digits
    while (m.size() != 12)
    {
        m.insert(0,"0");
    }

    // make output readable
    // adjust for previously inserted colons by incrementing place
    m.insert(2,":");
    m.insert(5,":");
    m.insert(8,":");
    m.insert(11,":");
    m.insert(14,":");

    return m;
}


unsigned int readHexByte (const char** beg, const char* end)
{
    if(end - *beg < 2)
    {
        throw std::invalid_argument("");
    }

    unsigned hi = (*beg)[0], lo = (*beg)[1];
    *beg += 2;
    hi -= hi >= '0' && hi <= '9' ? '0' :
        hi >= 'a' && hi <= 'f' ? 'a' - 10 :
        hi >= 'A' && hi <= 'F' ? 'A' - 10 :
        throw std::invalid_argument("");

    lo -= lo >= '0' && lo <= '9' ? '0' :
        lo >= 'a' && lo <= 'f' ? 'a' - 10 :
        lo >= 'A' && lo <= 'F' ? 'A' - 10 :
        throw std::invalid_argument("");

    return hi << 4 | lo;
}


uint64_t mac2int (const std::string& mac)
{
    char const *beg = mac.data();
    char const *end = beg + mac.size();
    uint64_t r = 0;
    try
    {
        r = readHexByte(&beg, end);
        beg += beg != end && ':' == *beg;

        r = r << 8 | readHexByte(&beg, end);
        beg += beg != end && ':' == *beg;

        r = r << 8 | readHexByte(&beg, end);
        beg += beg != end && ':' == *beg;

        r = r << 8 | readHexByte(&beg, end);
        beg += beg != end && ':' == *beg;

        r = r << 8 | readHexByte(&beg, end);
        beg += beg != end && ':' == *beg;

        r = r << 8 | readHexByte(&beg, end);
    }
    catch (std::invalid_argument&)
    {
        beg = end - 1;
    }
    if (beg != end)
    {
        throw std::runtime_error("invalid mac address format " + mac);
    }
    return r;
}


ip4_address_t ip2int (const std::string& ip)
{
    struct in_addr addr;

    // inet_aton always returns network byte order
    if (inet_aton(ip.c_str(), &addr) != 1)
    {
        return -1;
    }
    return addr.s_addr;
}


sockaddr_in fillAddr (const std::string &address, const unsigned short port)
{
    static std::mutex mutex;

    std::lock_guard<std::mutex>lck(mutex);

    sockaddr_in retv = sockaddr_in();
    retv.sin_family = AF_INET;

    hostent *host;  // Resolve name
    if ((host = gethostbyname(address.c_str())) == NULL)
    {
        // TODO throw exception
    }
    retv.sin_addr.s_addr = *((unsigned int*) host->h_addr_list[0]);

    retv.sin_port = htons(port);     // Assign port in network byte order

    return retv;
}


bool isValidMAC (const std::string& mac)
{
    const char* s = mac.c_str();
    for(int i = 0; i < 17; i++)
    {
        if(i % 3 != 2 && !isxdigit(s[i]))
        {
            return false;
        }
        if(i % 3 == 2 && s[i] != ':')
        {
            return false;
        }
    }
    if(s[17] != '\0')
    {
        return false;
    }
    return true;
}


} /* namespace tis */
