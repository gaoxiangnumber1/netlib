#ifndef NETLIB_NETLIB_SOCKET_OPERATION_H_
#define NETLIB_NETLIB_SOCKET_OPERATION_H_

#include <netinet/in.h>

namespace netlib
{

namespace socket_operation
{

// Interface:
// CastToConstsockaddr
// CastToNonConstsockaddr
// CreateNonblockingTcpSocket
// GetLocalAddress
// GetPeerAddress
// GetSocketError

const struct sockaddr *CastToConstsockaddr(const struct sockaddr_in*);
struct sockaddr *CastToNonConstsockaddr(struct sockaddr_in*);
int CreateNonblockingTcpSocket(sa_family_t family);
struct sockaddr_in GetLocalAddress(int socket);
struct sockaddr_in GetPeerAddress(int socket);
int GetSocketError(int socket);

}

}

namespace nso = netlib::socket_operation;

#endif // NETLIB_NETLIB_SOCKET_OPERATION_H_
