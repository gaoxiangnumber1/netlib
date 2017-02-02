#ifndef NETLIB_NETLIB_SOCKET_ADDRESS_H_
#define NETLIB_NETLIB_SOCKET_ADDRESS_H_

#include <netinet/in.h> // struct sockaddr_in

#include <string>

#include <netlib/copyable.h>

namespace netlib
{

// Wrapper of `struct sockaddr_in` that can convert byte order automatically.
// SocketAddress is value semantics, it can be copied.

// Interface:
// Ctor(int), Ctor(string, int), Ctor(const struct sockaddr_in&)
// socket_family
// socket_address
// set_socket_address
// ToIpPortString

class SocketAddress: public Copyable
{
public:
	// Construct an endpoint with given port number. Mostly used in server listening.
	explicit SocketAddress(int port = 0);
	// Construct an endpoint with given ip and port. ip should be "1.2.3.4".
	SocketAddress(std::string ip, int port);
	// Construct an endpoint with given `struct sockaddr_in`.
	// Mostly used when accepting new connections.
	explicit SocketAddress(const struct sockaddr_in &address): address_(address) {}
	// For future support for IPv6
	sa_family_t socket_family() const
	{
		return address_.sin_family;
	}
	const struct sockaddr *socket_address() const;
	void set_socket_address(const struct sockaddr_in &address)
	{
		address_ = address;
	}

	// Convert the address to string representation: `IP:Port`.
	std::string ToIpPortString() const;

private:
	struct sockaddr_in address_;
};

}

#endif // NETLIB_NETLIB_SOCKET_ADDRESS_H_
