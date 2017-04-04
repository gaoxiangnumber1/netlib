#ifndef NETLIB_NETLIB_SOCKET_H_
#define NETLIB_NETLIB_SOCKET_H_

#include <netlib/non_copyable.h>

namespace netlib
{

class SocketAddress;

// Interface:
// Ctor
// Dtor
// socket_fd
// Bind
// Listen
// Accept
// ShutdownOnWrite
// SetReuseAddress
// SetReusePort
// SetTcpKeepAlive
// SetTcpNoDelay

class Socket: public NonCopyable
{
public:
	explicit Socket(const int fd): socket_(fd) {}
	~Socket();

	int socket() const
	{
		return socket_;
	}

	void Bind(const SocketAddress &local_address);
	void Listen();
	int Accept(SocketAddress &peer_address);

	void ShutdownOnWrite();
	void SetReuseAddress(bool on);
	void SetReusePort(bool on);
	void SetTcpKeepAlive(bool on);
	void SetTcpNoDelay(bool on);

private:
	const int socket_;
};

}

#endif // NETLIB_NETLIB_SOCKET_H_
