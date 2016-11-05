#ifndef NETLIB_NETLIB_SOCKET_ADDRESS_H_
#define NETLIB_NETLIB_SOCKET_ADDRESS_H_

#include <copyable.h>

namespace netlib
{

// Wrapper of sockaddr_in. This is an POD interface class.
class SocketAddress: public Copyable
{
public:
	// Construct an endpoint with given port number. Mostly used in server listening.
	explicit SocketAddress(int port);
	const struct sockaddr_in &get_socket_address() const
	{
		return address_;
	}
	void SetSocketAddress(const struct sockaddr_in &address)
	{
		address_ = address;
	}

private:
	struct sockaddr_in address_;
};

}

#endif // NETLIB_NETLIB_SOCKET_ADDRESS_H_
