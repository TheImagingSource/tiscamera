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

#include "Socket.h"
#include <unistd.h>

#include <exception>

namespace tis
{

Socket::Socket (const sockaddr_in& address, int timeout)
    : fd(-1), timeout_ms(timeout)
{
    try
    {
        fd = createSocket();
    }
    catch (SocketCreationException& e)
    {
        throw; //rethrow
    }

    try
    {
        this->bindTo(address);
    }
    catch (SocketBindingException& e)
    {
        close(fd);
        throw;
    }
}


Socket::~Socket ()
{
    close(fd);
}


int Socket::createSocket ()
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);

    if (s == -1)
    {
        throw SocketCreationException();
    }

    int val = 0;

    // assure that in case of error we still can use the port and no EADDRINUSE will be returned for new sockets
    setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof (val));

    return s;
}


void Socket::bindTo (const sockaddr_in& address)
{
    int bind_res = bind(fd, (const struct sockaddr*)&address, sizeof(address));

    if (bind_res < 0)
    {
        throw SocketBindingException();
    }
}


bool Socket::setBroadcast (bool enable)
{
    int result;
    int val = 0;

    if (enable)
    {
        val = 1;
    }
    result = setsockopt (this->fd, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof (val));

    return result == 0;
}


void Socket::sendAndReceive (const std::string& destination_address, void* data, size_t size, std::function<int(void*)> callback, const bool broadcast)
{
    sockaddr_in destAddr = fillAddr(destination_address, STANDARD_GVCP_PORT);
    setBroadcast(broadcast);

    ssize_t send = sendto(fd, (uint8_t*)data, size, 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (send <= 0)
    {
        throw SocketSendToException();
    }
    else
    {
        // we have nothing to wait for end just end here
        if (callback == NULL)
        {
            return;
        }

        timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        while (select(fd+1, &fds, NULL, NULL, &timeout) > 0)
        {
            char msg[1024];

            struct sockaddr_storage sender = sockaddr_storage();
            socklen_t sendsize = 0;

            if (recvfrom(fd, msg, sizeof(msg), 0, (sockaddr*)&sender, &sendsize) >= 0)
            {
                if (callback(msg) == SendAndReceiveSignals::END)
                {
                    return;
                }

                // not working due to gcc bug
                //auto cam = std::make_shared<Camera>(ack, interfaces.at(i).socket, interfaces.at(i).name);
            }
        }
    }
}

} /* namespace tis */
