///
/// @file Socket.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///
/// @brief Socket class
/// Interaction class that allows sending of packets

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <memory>
#include <sys/socket.h>
#include <unistd.h>

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

public :

    // send and receive signals
    enum SendAndReceiveSignals
    {
        CONTINUE, // continue waiting for incoming packets
        END       // end waiting now
    };

    /// Constructor
    explicit Socket (const sockaddr_in& address);

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
