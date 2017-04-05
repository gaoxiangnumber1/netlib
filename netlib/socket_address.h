#ifndef NETLIB_NETLIB_SOCKET_ADDRESS_H_
#define NETLIB_NETLIB_SOCKET_ADDRESS_H_

#include <netinet/in.h> // struct sockaddr_in

#include <string>

#include <netlib/copyable.h>

namespace netlib
{

// Interface:
// Ctor(int), Ctor(string, int), Ctor(const struct sockaddr_in&)
// socket_family
// socket_address
// set_socket_address
// ToIpPortString

class SocketAddress: public Copyable
{
public:
	SocketAddress(int port = 0); // Usually used in server listening.
	SocketAddress(std::string ip, int port); // ip should be "1.2.3.4".
	SocketAddress(const struct sockaddr_in &address): // Usually used in server accepting.
		address_(address) {}

	sa_family_t socket_family() const
	{
		return address_.sin_family;
	}
	const struct sockaddr *socket_address() const;
	void set_socket_address(const struct sockaddr_in &address)
	{
		address_ = address;
	}

	std::string ToIpPortString() const;

private:
	struct sockaddr_in address_;
};

}

#endif // NETLIB_NETLIB_SOCKET_ADDRESS_H_
