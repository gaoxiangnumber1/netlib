#include <netlib/socket.h>

#include <sys/socket.h> // setsockopt()

namespace nso = netlib::socket_operation;

using netlib::Socket;

Socket::~Socket()
{
	nso::close(socket_fd_);
}

int Accept(SocketAddress *peer_address)
{
	struct sockaddr_in address;
	bzero(&address, sizeof address);
	int connection_fd = nso::Accept(socket_fd_, &address);
	if(connection_fd >= 0)
	{
		peer_address->SetSocketAddress(address);
	}
	return connection_fd;
}

void Socket::BindAddress(const SocketAddress &local_address)
{
	nso::BindOrDie(socket_fd_, local_address.get_socket_address());
}

void Socket::SetReuseAddress(bool on)
{
	int option_value = on ? 1 : 0;
	// int setsockopt(int sockfd, int level, int optname,
	//                const void *optval, socklen_t optlen);
	// SOL_SOCKET: manipulate options at the sockets API level.
	// optval and optlen are used to access option values.
	// On success, 0 is returned. On error, -1 is returned, and errno is set.
	::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR,
	             &option_value, static_cast<socklen_t>(sizeof option_value));
	// FIXME: Add check.
}
