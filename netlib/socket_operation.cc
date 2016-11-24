#include <netlib/socket_operation.h>

#include <sys/socket.h> // socket(), getsockname()
#include <strings.h> // bzero()

#include <netlib/logging.h>

// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
int nso::CreateNonblockingSocket(sa_family_t family)
{
	// int socket(int domain, int type, int protocol);
	// domain specifies a communication domain; this selects the protocol family which
	// will be used for communication.
	// AF_INET				IPv4 Internet protocols
	// AF_INET6				IPv6 Internet protocols
	// type specifies the communication semantics.
	// SOCK_STREAM
	//		Provide sequenced, reliable, two-way, connection-based byte streams.
	// SOCK_NONBLOCK
	//		Set the O_NONBLOCK file status flag on the new open file description.
	// SOCK_CLOEXEC
	//		Set the close-on-exec(FD_CLOEXEC) flag on the new file descriptor.
	// IPPROTO_TCP means using TCP.
	int socket_fd = ::socket(family,
	                         SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
	                         IPPROTO_TCP);
	// On success, a file descriptor for the new socket is returned.
	// On error, -1 is returned, and errno is set.
	if(socket_fd == -1)
	{
		LOG_FATAL("nso::CreateNonblockingOrDie");
	}
	return socket_fd;
}

// Return the address to which the socket socket_fd is bound.
struct sockaddr_in nso::GetLocalAddress(int socket_fd)
{
	struct sockaddr_in local_address;
	bzero(&local_address, sizeof local_address);
	socklen_t address_length = sizeof(local_address);
	// int getsockname(int slocal_addressockfd, struct sockaddr *addr, socklen_t *addrlen);
	// getsockname() return the address to which the socket sockfd is bound, in
	// the buffer pointed to by addr. addrlen should be initialized to indicate the
	// amount of space(in bytes) pointed to by addr. On return it contains the actual
	// size of the socket address.
	// 0 is returned on success. -1 is returned on error, and errno is set.
	if (::getsockname(socket_fd,
	                  CastToNonConstsockaddr(&local_address),
	                  &address_length) == -1)
	{
		LOG_ERROR("nso::GetLocalAddress error");
	}
	return local_address;
}

int nso::GetSocketError(int socket_fd)
{
	int option_value;
	socklen_t option_length = sizeof option_value;

	if(::getsockopt(socket_fd, SOL_SOCKET, SO_ERROR,
	                &option_value, &option_length) < 0)
	{
		return errno;
	}
	else
	{
		return option_value;
	}
}
