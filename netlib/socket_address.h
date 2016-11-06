#ifndef NETLIB_NETLIB_SOCKET_ADDRESS_H_
#define NETLIB_NETLIB_SOCKET_ADDRESS_H_

#include <netinet/in.h> // struct sockaddr_in

#include <string>

#include <netlib/copyable.h>

namespace netlib
{

// Wrapper of `struct sockaddr_in` that can convert byte order automatically.
// SocketClass is value semantics, it can be copied.
class SocketAddress: public Copyable
{
public:
	// Construct an endpoint with given port number. Mostly used in server listening.
	explicit SocketAddress(int port);
	// Construct an endpoint with given `struct sockaddr_in`.
	// Mostly used when accepting new connections.
	SocketAddress(const struct sockaddr_in &address): address_(address) {}
	const struct sockaddr_in &get_socket_address() const
	{
		return address_;
	}
	void set_socket_address(const struct sockaddr_in &address)
	{
		address_ = address;
	}

	std::string ToHostPort() const;

private:
	struct sockaddr_in address_;
};

}

#endif // NETLIB_NETLIB_SOCKET_ADDRESS_H_
