#include <netlib/socket.h>

#include <netinet/tcp.h> // TCP_NODELAY
#include <strings.h> // bzero()
#include <unistd.h> // close()
#include <sys/socket.h> // setsockopt(), accept4(), listen(), bind(), shutdown()

#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/socket_operation.h>

using netlib::Socket;

Socket::~Socket()
{
	::close(socket_);
}

void Socket::Bind(const SocketAddress &local_address)
{
	if(::bind(socket_, local_address.socket_address(), sizeof(struct sockaddr)) == -1)
	{
		LOG_FATAL("bind(): FATAL");
	}
}
void Socket::Listen()
{
	if(::listen(socket_, SOMAXCONN) == -1)
	{
		LOG_FATAL("listen(): FATAL");
	}
}
int Socket::Accept(SocketAddress &peer_address)
{
	struct sockaddr_in address;
	bzero(&address, sizeof address);
	socklen_t address_length = sizeof address;
	int connected_fd = ::accept4(socket_,
	                             nso::CastToNonConstsockaddr(&address),
	                             &address_length,
	                             SOCK_NONBLOCK | SOCK_CLOEXEC);
	if(connected_fd >= 0)
	{
		peer_address.set_socket_address(address);
	}
	else
	{
		int saved_errno = errno;
		switch(saved_errno)
		{
		case EAGAIN:
		case ENETDOWN:
		case EPROTO:
		case ENOPROTOOPT:
		case EHOSTDOWN:
		case ENONET:
		case EHOSTUNREACH:
		case EOPNOTSUPP:
		case ENETUNREACH:
		case ECONNABORTED:
		case EINTR:
		case EPERM:
		case EMFILE:
			errno = saved_errno;
			LOG_ERROR("accept(): ERROR");
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
			LOG_FATAL("accept(): FATAL");
		default:
			LOG_FATAL("accept(): Unknown");
		}
	}
	return connected_fd;
}

void Socket::ShutdownOnWrite()
{
	if(::shutdown(socket_, SHUT_WR) == -1)
	{
		LOG_ERROR("shutdown(): ERROR");
	}
}
void Socket::SetReuseAddress(bool on)
{
	int option_value = on ? 1 : 0;
	int ret = ::setsockopt(socket_,
	                       SOL_SOCKET,
	                       SO_REUSEADDR,
	                       &option_value,
	                       sizeof option_value);
	if(ret == -1 && on)
	{
		LOG_ERROR("setsockopt(SO_REUSEADDR): ERROR");
	}
}
void Socket::SetReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int option_value = on ? 1 : 0;
	int ret = ::setsockopt(socket_,
	                       SOL_SOCKET,
	                       SO_REUSEPORT,
	                       &option_value,
	                       sizeof option_value);
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
	int ret = ::setsockopt(socket_,
	                       SOL_SOCKET,
	                       SO_KEEPALIVE,
	                       &option_value,
	                       sizeof option_value);
	if(ret == -1 && on)
	{
		LOG_ERROR("setsockopt(SO_KEEPALIVE): ERROR");
	}
}
void Socket::SetTcpNoDelay(bool on)
{
	int option_value = on ? 1 : 0;
	int ret = ::setsockopt(socket_,
	                       IPPROTO_TCP,
	                       TCP_NODELAY,
	                       &option_value,
	                       sizeof option_value);
	if(ret == -1 && on)
	{
		LOG_ERROR("SetTcpNoDelay error");
	}
}
