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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>

#include "gigevision.h"
#include "utils.h"


namespace tis
{

/// @struct SocketCreationException
struct SocketCreationException : public std::exception
{
    const char* what () const throw ()
    {
        return "Socket creation returned -1.";
    }
};


/// @struct SocketBindingException
struct SocketBindingException : public std::exception
{
    const char* what () const throw ()
    {
        return "Unable to bind Socket to Interface.";
    }
};


/// @struct SocketSendToException
struct SocketSendToException : public std::exception
{
    const char* what () const throw ()
    {
        return "Error while trying to send package";
    }
};


class Socket
{
private:

    // actual file descriptor
    int fd;

    // timeout in milliseconds
    int timeout_ms;

public :

    // send and receive signals
    enum SendAndReceiveSignals
    {
        CONTINUE, // continue waiting for incoming packets
        END       // end waiting now
    };

    /// Constructor
    explicit Socket (const sockaddr_in& address, int timeout=1500);

    /// copy constructor
    Socket (const Socket& _socket) = delete;
    Socket& operator=(const Socket& ) = delete;

    ~Socket ();

    /// @name setBroadcast
    /// @param enable - bool of the wished state
    /// @return true on success
    bool setBroadcast (bool enable);

    /// @name sendAndReceive
    /// @param destination_address - address that shall receive data
    /// @param data - information that shall be sent
    /// @param size - size of data
    /// @param callback - callback function that should be called on response; can return -1 to end waiting for additional response packages
    /// @param broadcast - wether this shall be broadcasted or not
    void sendAndReceive (const std::string& destination_address, void* data, size_t size, std::function<int(void*)> callback, const bool broadcast = false);

private:
    /// @name createSocket
    /// @return new socket fd
    int createSocket ();

    /// @name bindTo
    /// @param address - sockaddr_in containing the interface desciption this socket shall be bind to
    void bindTo (const sockaddr_in& address);

}; /* class Socket */

} /* namespace tis */

#endif /* _SOCKET_H_ */
