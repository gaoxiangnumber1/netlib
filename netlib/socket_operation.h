#ifndef NETLIB_NETLIB_SOCKET_OPERATION_H_
#define NETLIB_NETLIB_SOCKET_OPERATION_H_

#include <netinet/in.h> // struct sockaddr_in: sa_family_t

namespace netlib
{

namespace socket_operation
{

// Review:
// Function: CreateNonblockingSocket, Get*Address#a_l, GetSocketError#a_l

// Interface:
// CastToConstsockaddr
// CastToNonConstsockaddr
// CreateNonblockingSocket
// GetLocalAddress
// GetPeerAddress
// GetSocketError

const struct sockaddr *CastToConstsockaddr(const struct sockaddr_in*);
struct sockaddr *CastToNonConstsockaddr(struct sockaddr_in*);

// Create an IPv4/6, nonblocking, and TCP socket file descriptor, abort if any error.
// Called in Acceptor class and Connector class.
int CreateNonblockingSocket(sa_family_t family);
// Return the address to which the socket socket_fd is bound.
// Called in TcpServer class and TcpClient class.
struct sockaddr_in GetLocalAddress(int socket_fd);
struct sockaddr_in GetPeerAddress(int socket_fd);
// Used in `TcpConnection::HandleError()` and Connector class.
int GetSocketError(int socket_fd);

}

}

namespace nso = netlib::socket_operation;

#endif // NETLIB_NETLIB_SOCKET_OPERATION_H_
