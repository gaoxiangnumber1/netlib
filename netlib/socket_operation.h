#ifndef NETLIB_NETLIB_SOCKET_OPERATION_H_
#define NETLIB_NETLIB_SOCKET_OPERATION_H_

#include <arpa/inet.h> // htonl, htons, ntohl, ntohs.

namespace netlib
{

namespace socket_operation
{

// #include <arpa/inet.h>
// uint16_t htons(uint16_t hostshort)
// uint32_t htonl(uint32_t hostlong);
// uint16_t ntohs(uint16_t netshort);
// uint32_t ntohl(uint32_t netlong);

inline uint16_t HostToNetwork16(int host16)
{
	return htons(static_cast<uint16_t>(host16));
}
inline uint32_t HostToNetwork32(int host32)
{
	return htonl(static_cast<uint32_t>(host32));
}
inline uint16_t NetworkToHost16(int net16)
{
	return ntohs(static_cast<uint16_t>(net16));
}

// `struct sockaddr_in` is the IPv4 socket address structure.
// Return a nonnegative file descriptor for the accepted socket on success.
int Accept(int socket_fd, struct sockaddr_in *address);
// Assign the address of `address` to the socket `socket_fd`.
void BindOrDie(int socket_fd, const struct sockaddr_in &address);
// Close the `socket_fd` file descriptor.
void Close(int socket_fd);
// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
int CreateNonblockingOrDie();
// Mark the socket `socket_fd` as a passive socket(i.e., accept connections)
void ListenOrDie(int socket_fd);
// Convert the address to string representation: `IP:Port`.
void ToHostPort(char *buffer, size_t size, const struct sockaddr_in &address);
// Return the address to which the socket socket_fd is bound.
struct sockaddr_in GetLocalAddress(int socket_fd);
int GetSocketError(int socket_fd);
void ShutdownWrite(int socket_fd);

}

}

namespace nso = netlib::socket_operation;

#endif // NETLIB_NETLIB_SOCKET_OPERATION_H_
