#ifndef NETLIB_NETLIB_SOCKET_H_
#define NETLIB_NETLIB_SOCKET_H_

#include <netlib/non_copyable.h>

// struct tcp_info; TODO: add when need it.

namespace netlib
{

class SocketAddress;

// A RAII handle of socket file descriptor. It closes the socket_fd when destructs.
// It's thread safe, all operations are delegated to OS.
class Socket: public NonCopyable
{
public:
	explicit Socket(int fd): socket_fd_(fd) {}
	// TODO: Socket(Socket&&) // move constructor in C++11
	~Socket();

	int socket_fd() const
	{
		return socket_fd_;
	}

	// Assign local_address object's address_ to this Socket's object's socket_fd_.
	// Abort if address already in use.
	void Bind(const SocketAddress &local_address);
	// Mark socket_fd_ as a passive socket(i.e., accept connections).
	// Abort if address already in use.
	void Listen();
	// Return a non-negative integer that is a descriptor for the accepted socket on success,
	// which has been set to non-blocking and close-on-exec. peer_address is assigned.
	// On error, -1 is returned, and peer_address is untouched.
	int Accept(SocketAddress &peer_address);

	// SHUT_WR: further transmissions will be disallowed.
	void ShutdownOnWrite();

	// Enable/Disable SO_REUSEADDR.
	void SetReuseAddress(bool on);
	// Enable/Disable SO_REUSEPORT
	void SetReusePort(bool on);
	// Enable/disable SO_KEEPALIVE
	void SetTcpKeepAlive(bool on);
	// Enable/Disable TCP_NODELAY (disable/enable Nagle's algorithm).
	void SetTcpNoDelay(bool on);

private:
	const int socket_fd_;
};

}

#endif // NETLIB_NETLIB_SOCKET_H_
