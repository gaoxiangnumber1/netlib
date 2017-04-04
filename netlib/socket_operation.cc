#include <netlib/socket_operation.h>

#include <sys/socket.h> // socket(), getsockname()
#include <strings.h> // bzero()

#include <netlib/logging.h>

const struct sockaddr *nso::CastToConstsockaddr(const struct sockaddr_in *address)
{
	return static_cast<const struct sockaddr*>(static_cast<const void*>(address));
}
struct sockaddr *nso::CastToNonConstsockaddr(struct sockaddr_in *address)
{
	return static_cast<struct sockaddr*>(static_cast<void*>(address));
}
int nso::CreateNonblockingTcpSocket(sa_family_t family)
{
	int socket_fd = ::socket(family,
	                         SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
	                         IPPROTO_TCP);
	if(socket_fd == -1)
	{
		LOG_FATAL("socket(): FATAL");
	}
	return socket_fd;
}
struct sockaddr_in nso::GetLocalAddress(int socket)
{
	struct sockaddr_in local_address;
	bzero(&local_address, sizeof local_address);
	socklen_t address_length = sizeof local_address;
	if (::getsockname(socket,
	                  CastToNonConstsockaddr(&local_address),
	                  &address_length) == -1)
	{
		LOG_ERROR("nso::GetLocalAddress error");
	}
	return local_address;
}
struct sockaddr_in nso::GetPeerAddress(int socket)
{
	struct sockaddr_in peer_address;
	bzero(&peer_address, sizeof peer_address);
	socklen_t address_length = sizeof peer_address;
	if(::getpeername(socket,
	                 CastToNonConstsockaddr(&peer_address),
	                 &address_length) == -1)
	{
		LOG_ERROR("nso::GetPeerAddress");
	}
	return peer_address;
}
int nso::GetSocketError(int socket)
{
	int option_value;
	socklen_t option_length = sizeof option_value;
	if(::getsockopt(socket, SOL_SOCKET, SO_ERROR,
	                &option_value, &option_length) == -1)
	{
		return errno;
	}
	return option_value;
}
