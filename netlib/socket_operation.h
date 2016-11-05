#ifndef NETLIB_NETLIB_SOCKET_OPERATION_H_
#define NETLIB_NETLIB_SOCKET_OPERATION_H_

namespace netlib
{

namespace socket_operation
{

inline uint32_t HostToNetwork32(int host32)
{
	return htonl(static_cast<uint32_t>(host32));
}

inline uint16_t HostToNetwork16(int host16)
{
	return htons(static_cast<uint16_t>(host16));
}

// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
int CreateNonblockingOrDie();
void BindOrDie(int socket_fd, const struct sockaddr_in &address);
int Accept(int socket_fd, struct sockaddr_in *address);
}

}

#endif // NETLIB_NETLIB_SOCKET_OPERATION_H_
