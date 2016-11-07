#include <netlib/socket.h>
#include <netlib/socket_operation.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>

#include <sys/socket.h> // setsockopt()
#include <strings.h> // bzero()

namespace nso = netlib::socket_operation;

using netlib::Socket;

Socket::~Socket()
{
	nso::Close(socket_fd_);
}
// Enable/Disable SO_REUSEADDR
void Socket::SetReuseAddress(bool on)
{
	int option_value = on ? 1 : 0;
	// int setsockopt(int sockfd, int level, int optname,
	//                const void *optval, socklen_t optlen);
	// SOL_SOCKET: manipulate options at the sockets API level.
	// optval and optlen are used to access option values.
	// On success, 0 is returned. On error, -1 is returned, and errno is set.
	int ret = ::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR,
	                       &option_value, static_cast<socklen_t>(sizeof option_value));
	if(ret == -1)
	{
		LOG_INFO("SetReuseAddress() fails.");
	}
}
// Assign local_address object's address_ to this Socket's object's socket_fd_.
void Socket::BindAddress(const SocketAddress &local_address)
{
	nso::BindOrDie(socket_fd_, local_address.get_socket_address());
}
// Mark socket_fd_ as a passive socket(i.e., accept connections).
void Socket::Listen()
{
	nso::ListenOrDie(socket_fd_);
}
// Return the file descriptor for the new accepted connection.
// Store the peer's address to `*peer_address` object.
int Socket::Accept(SocketAddress *peer_address)
{
	struct sockaddr_in address;
	bzero(&address, sizeof address);
	int connection_fd = nso::Accept(socket_fd_, &address);
	if(connection_fd >= 0)
	{
		peer_address->set_socket_address(address);
	}
	return connection_fd;
}
