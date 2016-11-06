#ifndef __GNU_SOURCE_ // for accept4()
#define __GNU_SOURCE_
#endif // __GNU_SOURCE_

#include <netlib/socket_operation.h>
#include <netlib/logging.h>

#include <stdio.h> // snprintf()
// struct sockaddr, accept4(), bind(), socket(), listen(), getscokname()
#include <sys/socket.h>
#include <errno.h> // errno
#include <unistd.h> // close()

namespace
{

using SA = struct sockaddr; // Generic socket address structure

const SA *CastToConstsockaddr(const struct sockaddr_in *address)
{
	return static_cast<const SA*>(static_cast<const void*>(address));
}

SA *CastToNonConstsockaddr(struct sockaddr_in *address)
{
	return static_cast<SA*>(static_cast<void*>(address));
}

} // Unnamed namespace.

namespace nso = netlib::socket_operation;

// Return a nonnegative file descriptor for the accepted socket on success.
int nso::Accept(int socket_fd, struct sockaddr_in *address)
{
	// int accept4(int sockfd, struct sockaddr *address, socklen_t *addrlen, int flags);
	// accept() extracts the first connection request on the queue of pending connections
	// for the listening socket, sockfd, creates a new connected socket, and returns a new
	// file descriptor referring to that socket. sockfd is unaffected by this call.
	// sockfd is a socket that has been created with socket(2), bound to a local address
	// with bind(2), and is listening for connections after a listen(2).
	// address is a pointer to a sockaddr structure. This structure is filled in with the address
	// of the peer socket. When address is NULL, nothing is filled in and
	// addrlen should be NULL.
	// addrlen is a value-result argument: the caller must initialize it to contain the size
	// (in bytes) of the structure pointed to by address;
	// on return it will contain the size of the peer address.
	// Return a nonnegative integer that is a descriptor for the accepted socket on success.
	// On error, -1 is returned, and errno is set.
	socklen_t address_length = sizeof *address;
	int connection_fd = ::accept4(socket_fd, CastToNonConstsockaddr(address),
	                              &address_length, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(connection_fd == -1)
	{
		int saved_errno = errno;
		LOG_INFO("Socket::accept error");
		switch(saved_errno)
		{
		case EAGAIN:
		case ECONNABORTED:
		case EINTR:
		case EPROTO: // TODO
		case EPERM:
		case EMFILE: // per-process limit of open file descriptor TODO
			// Temporary errors: ignore error.
			errno = saved_errno;
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
		case EOPNOTSUPP:
			// Unexpected errors
			LOG_FATAL("unexpected error of ::accept");
			break;
		default:
			// Unknown errors.
			LOG_FATAL("unknown error of ::accept");
			break;
		}
	}
	return connection_fd;
}

// Assign the address of `address` to the socket `socket_fd`.
void nso::BindOrDie(int socket_fd, const struct sockaddr_in &address)
{
	// int bind(int sockfd, const struct sockaddr *address, socklen_t address_length);
	// When a socket is created with socket(2), it has no address assigned to it.
	// bind() assigns the address specified by address to the socket referred to by
	// the file descriptor sockfd. address_length specifies the size, in bytes, of the
	// address structure pointed to by address.
	int ret = ::bind(socket_fd, CastToConstsockaddr(&address), sizeof address);
	// On success, 0 is returned. On error, -1 is returned, and errno is set.
	if(ret == -1)
	{
		LOG_FATAL("sockets::bindOrDie");
	}
}

// Close the `socket_fd` file descriptor.
void nso::Close(int socket_fd)
{
	// int close(int fd);
	// 0 on success. -1 on error and errno is set.
	if(::close(socket_fd) == -1)
	{
		LOG_INFO("sockets::close error");
	}
}

// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
int nso::CreateNonblockingOrDie()
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
	int socket_fd = ::socket(AF_INET,
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

// Mark the socket `socket_fd` as a passive socket(i.e., accept connections)
void nso::ListenOrDie(int socket_fd)
{
	// int listen(int sockfd, int backlog);
	// listen() marks the socket `sockfd` as a passive socket, that is, as a socket that will
	// be used to accept incoming connection requests using accept(2).
	// backlog defines the maximum length to which the queue of pending connections
	// for sockfd may grow. If a connection request arrives when the queue is full,
	// the client may receive an error with an indication of ECONNREFUSED or, if
	// the underlying protocol supports retransmission, the request may be ignored so
	// that a later reattempt at connection succeeds.
	// On success, 0 is returned. On error, -1 is returned and errno is set.
	// SOMAXCONN: Maximum queue length specifiable by listen.
	int ret = ::listen(socket_fd, SOMAXCONN);
	if(ret < 0)
	{
		LOG_FATAL("nso::listen()");
	}
}

// Convert the address to string representation: `IP:Port`.
void nso::ToHostPort(char *buffer, size_t size, const struct sockaddr_in &address)
{
	char host[INET_ADDRSTRLEN] = "INVALID"; // Defined <netinet/in.h>, 16
	::inet_ntop(AF_INET, &address.sin_addr, host, sizeof host);
	uint16_t port = nso::NetworkToHost16(static_cast<int>(address.sin_port));
	::snprintf(buffer, size, "%s:%u", host, port);
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
		LOG_INFO("nso::GetLocalAddress error");
	}
	return local_address;
}
