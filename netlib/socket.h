#ifndef NETLIB_NETLIB_SOCKET_H_
#define NETLIB_NETLIB_SOCKET_H_

#include <netlib/non_copyable.h>

namespace netlib
{

class SocketAddress;

// A RAII handle of socket file descriptor. It closes the socket_fd when destructs.
// It's thread safe, all operations are delegated to OS.
class Socket: public NonCopyable
{
public:
	explicit Socket(int fd): socket_fd_(fd) {}
	~Socket();

	int socket_fd() const
	{
		return socket_fd_;
	}

	// Enable/Disable SO_REUSEADDR.
	void SetReuseAddress(bool on);
	// Assign local_address object's address_ to this Socket's object's socket_fd_.
	void BindAddress(const SocketAddress &local_address);
	// Mark socket_fd_ as a passive socket(i.e., accept connections).
	void Listen();
	// Return a non-negative integer that is a descriptor for the accepted socket on success,
	// which has been set to non-blocking and close-on-exec. *peer_address is assigned.
	// On error, -1 is returned, and *peer_address is untouched.
	int Accept(SocketAddress *peer_address);
	void ShutdownWrite();

private:
	const int socket_fd_;
};

}

#endif // NETLIB_NETLIB_SOCKET_H_
