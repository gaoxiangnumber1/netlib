#ifndef NETLIB_NETLIB_SOCKET_H_
#define NETLIB_NETLIB_SOCKET_H_

#include <netlib/non_copyable.h>

namespace netlib
{

class Socket: public NonCopyable
{
public:
	explicit Socket(int socket_fd): socket_fd_(socket_fd) {}
	~Socket();

	int socket_fd() const
	{
		return socket_fd_;
	}
	// Return a non-negative integer that is a descriptor for the accepted socket on success,
	// which has been set to non-blocking and close-on-exec. *peer_address is assigned.
	// On error, -1 is returned, and *peer_address is untouched.
	int Accept(SocketAddress *peer_address);
	// Abort if address in use.
	void BindAddress(const SocketAddress &local_address);
	// Enable/Disable SO_REUSEADDR
	void SetReuseAddress(bool on);

private:
	const int socket_fd_;
};

}

#endif // NETLIB_NETLIB_SOCKET_H_
