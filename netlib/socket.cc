#include <netlib/socket.h>

#include <netinet/tcp.h> // TCP_NODELAY
#include <strings.h> // bzero()
#include <sys/socket.h> // setsockopt(), accept4(), listen(), bind(), shutdown()

#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/socket_operation.h>

using netlib::Socket;

Socket::~Socket()
{
	nso::Close(socket_fd_);
}

// int bind(int sockfd, const struct sockaddr *address, socklen_t address_length);
// When a socket is created with socket(2), it has no address assigned to it.
// bind() assigns the address specified by address to the socket referred to by
// the file descriptor sockfd. address_length specifies the size, in bytes, of the
// address structure pointed to by address.
// On success, 0 is returned. On error, -1 is returned, and errno is set.
// Assign local_address object's address_ to this Socket's object's socket_fd_.
void Socket::Bind(const SocketAddress &local_address)
{
	const struct sockaddr *address = local_address.socket_address();
	int ret = ::bind(socket_fd_, address, static_cast<socklen_t>(sizeof address));
	if(ret == -1)
	{
		LOG_FATAL("Socket::BindAddress");
	}
}

// int listen(int sockfd, int backlog);
// listen() marks the socket `sockfd` as a passive socket, that is, as a socket that will
// be used to accept incoming connection requests using accept(2).
// backlog defines the maximum length to which the queue of pending connections
// for sockfd may grow. If a connection request arrives when the queue is full,
// the client may receive an error with an indication of ECONNREFUSED or, if
// the underlying protocol supports retransmission, the request may be ignored so
// that a later reattempt at connection succeeds.
// On success, 0 is returned. On error, -1 is returned and errno is set.
// Mark socket_fd_ as a passive socket(i.e., accept connections).
void Socket::Listen()
{
	// SOMAXCONN: Maximum queue length specifiable by listen.
	if(::listen(socket_fd_, SOMAXCONN) == -1)
	{
		LOG_FATAL("nso::listen()");
	}
}

// int accept4(int sockfd, struct sockaddr *address, socklen_t *addrlen, int flags);
// accept() extracts the first connection request on the queue of pending connections
// for the listening socket, sockfd, creates a new connected socket, and returns a new
// file descriptor referring to that socket. sockfd is unaffected by this call.
// sockfd is a socket that has been created with socket(2), bound to a local address
// with bind(2), and is listening for connections after a listen(2).
// address is a pointer to a sockaddr structure. This structure is filled in with the
// address of the peer socket. When address is NULL, nothing is filled in and
// addrlen should be NULL.
// addrlen is a value-result argument: the caller must initialize it to contain the size
// (in bytes) of the structure pointed to by address; on return it will contain the size
// of the peer address.
// Return a nonnegative integer that is a descriptor for the accepted socket on success.
// On error, -1 is returned, and errno is set.
// Return the file descriptor for the new accepted connection.
// Store the peer's address to `*peer_address` object.
int Socket::Accept(SocketAddress &peer_address)
{
	struct sockaddr_in address;
	bzero(&address, sizeof address);
	socklen_t address_length = static_cast<socklen_t>(sizeof address);
	int connected_fd = ::accept4(socket_fd_,
	                             nso::CastToNonConstsockaddr(&address),
	                             &address_length,
	                             SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(connected_fd == -1)
	{
		int saved_errno = errno;
		LOG_ERROR("Socket::Accept error");
		switch(saved_errno)
		{
		// EAGAIN or EWOULDBLOCK
		// The socket is marked nonblocking and no connections are present to be accepted.
		// A portable application should check for both possibilities.
		case EAGAIN:
		// case EWOULDBLOCK: // socket.cc:89:3: error: duplicate case value
		// accept()/accept4() passes pending network errors on the new socket as an error
		// code from accept(). The application should detect the network errors defined
		// for the protocol after accept() and treat them like EAGAIN by retrying. For
		// TCP/IP: ENETDOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN,
		// ENONET, EHOSTUNREACH, EOPNOTSUPP, and ENETUNREACH.
		case ENETDOWN:
		case EPROTO: // Protocol error.
		case ENOPROTOOPT:
		case EHOSTDOWN:
		case ENONET:
		case EHOSTUNREACH:
		case EOPNOTSUPP: // The socket is not of type SOCK_STREAM.
		case ENETUNREACH:
		// A connection has been aborted.
		case ECONNABORTED:
		// accept() was interrupted by a signal that was caught before a connection arrived.
		case EINTR:
		// Firewall rules forbid connection.
		case EPERM:
		// The per-process limit of open file descriptors has been reached.
		case EMFILE:
			// Temporary errors: ignore error.
			errno = saved_errno;
			break;
		// The descriptor is invalid.
		case EBADF:
		// The addr argument is not in a writable part of the user address space.
		case EFAULT:
		// `socket` is not listening, `addrlen` is invalid, or `flags` is invalid.
		case EINVAL:
		// The system limit on the total number of open files has been reached.
		case ENFILE:
		// ENOBUFS, ENOMEM
		// Not enough free memory. This often means that the memory allocation is
		// limited by the socket buffer limits, not by the system memory.
		case ENOBUFS:
		case ENOMEM:
		// The descriptor references a file, not a socket.
		case ENOTSOCK:
			LOG_FATAL("unexpected error of ::accept");
			break;
		default:
			LOG_FATAL("unknown error of ::accept");
			break;
		}
	}
	else if(connected_fd >= 0)
	{
		peer_address.set_socket_address(address);
	}
	return connected_fd;
}

// int shutdown(int sockfd, int how);
// On success, 0 is returned. On error, -1 is returned and errno is set.
void Socket::ShutdownOnWrite()
{
	if(::shutdown(socket_fd_, SHUT_WR) == -1)
	{
		LOG_ERROR("nso::ShutdownWrite error");
	}
}

// int setsockopt(int sockfd, int level, int optname,
//                const void *option_value, socklen_t optlen);
// sockfd must refer to an open socket descriptor.
// level specifies the code that interprets the option: the general socket code
// or some protocol-specific code(e.g., IPv4, IPv6, TCP).
// option_value is a pointer to a variable from which the new value of the option is fetched.
// optlen is the size of this variable.
// On success, 0 is returned. On error, -1 is returned, and errno is set.
void Socket::SetReuseAddress(bool on)
{
	int option_value = on ? 1 : 0;
	// SOL_SOCKET: manipulate options at the sockets API level.
	int ret = ::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR,
	                       &option_value, static_cast<socklen_t>(sizeof option_value));
	if(ret == -1 && on)
	{
		LOG_ERROR("SetReuseAddress() error.");
	}
}
void Socket::SetReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int option_value = on ? 1 : 0;
	int ret = ::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEPORT,
	                       &option_value, static_cast<socklen_t>(sizeof option_value));
	if(ret == -1 && on)
	{
		LOG_ERROR("SO_REUSEPORT error.");
	}
#else
	if(on)
	{
		LOG_INFO("SO_REUSEPORT is not supported.");
	}
#endif
}
void Socket::SetTcpKeepAlive(bool on)
{
	int option_value = on ? 1 : 0;
	int ret = ::setsockopt(socket_fd_, SOL_SOCKET, SO_KEEPALIVE,
	                       &option_value, static_cast<socklen_t>(sizeof option_value));
	if(ret == -1 && on)
	{
		LOG_ERROR("SetTcpNoDelay error");
	}
}
void Socket::SetTcpNoDelay(bool on)
{
	int option_value = on ? 1 : 0;
	// IPPROTO_TCP is used for TCP protocol.
	int ret = ::setsockopt(socket_fd_, IPPROTO_TCP, TCP_NODELAY,
	                       &option_value, static_cast<socklen_t>(sizeof option_value));
	if(ret == -1 && on)
	{
		LOG_ERROR("SetTcpNoDelay error");
	}
}
