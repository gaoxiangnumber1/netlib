#ifndef __GNU_SOURCE_
#define __GNU_SOURCE_
#endif // __GNU_SOURCE_
#include <netlib/socket_operation.h>

#include <sys/socket.h>

namespace
{

using SA = struct sockaddr;

const SA *CastToConstsockaddr(const struct sockaddr_in *address)
{
	return static_cast<const SA*>(address);
}

SA *CastToNonConstsockaddr(struct sockaddr_in *address)
{
	return static_cast<SA*>(address);
}

} // Unnamed namespace.

namespace nso = netlib::socket_operation;

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
	if(socket_fd < 0)
	{
		LOG_FATAL("nso::CreateNonblockingOrDie");
	}
	return socket_fd;
}

void nso::BindOrDie(int socket_fd, const struct sockaddr_in &address)
{
	// int bind(int sockfd, const struct sockaddr *address, socklen_t address_length);
	// When a socket is created with socket(2), it has no address assigned to it.
	// bind() assigns the address specified by address to the socket referred to by
	// the file descriptor sockfd. address_length specifies the size, in bytes, of the
	// address structure pointed to by address.
	int ret = ::bind(socket_fd, CastToConstsockaddr(&address), sizeof address);
	// On success, 0 is returned. On error, -1 is returned, and errno is set.
	if(ret < 0)
	{
		LOG_FATAL("sockets::bindOrDie");
	}
}

int Accept(int socket_fd, struct sockaddr_in *address)
{
	socklen_t address_length = sizeof *address;
	// int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
	// accept() extracts the first connection request on the queue of pending connections
	// for the listening socket, sockfd, creates a new connected socket, and returns a new
	// file descriptor referring to that socket. sockfd is unaffected by this call.
	// sockfd is a socket that has been created with socket(2), bound to a local address
	// with bind(2), and is listening for connections after a listen(2).
	// addr is a pointer to a sockaddr structure. This structure is filled in with the address
	// of the peer socket. When addr is NULL, nothing is filled in and
	// addrlen should be NULL.
	// addrlen is a value-result argument: the caller must initialize it to contain the size
	// (in bytes) of the structure pointed to by addr;
	// on return it will contain the size of the peer address.
	// On success, return a non-negative integer that is a descriptor for the accepted socket.
	// On error, -1 is returned, and errno is set.
	int connection_fd = ::accept4(sockfd, CastToNonConstsockaddr(address),
	                              &address_length, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(connection_fd < 0)
	{
		int saved_errno = errno;
		LOG_SYSERR << "Socket::accept";
		switch (saved_errno)
		{
		case EAGAIN:
		case ECONNABORTED:
		case EINTR:
		case EPROTO: // TODO
		case EPERM:
		case EMFILE: // per-process limit of open file descriptor TODO
			// Expected errors: ignore this error.
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
			// unexpected errors
			LOG_FATAL("unexpected error of ::accept");
			break;
		default:
			LOG_FATAL("unknown error of ::accept");
			break;
		}
	}
	return connection_fd;
}
