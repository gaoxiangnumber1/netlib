#include <netlib/socket_address.h>

#include <strings.h> // bzero()
#include <endian.h> // htobe*()
#include <arpa/inet.h> // inet_ntop(), inet_pton()

#include <netlib/socket_operation.h>
#include <netlib/logging.h>

using std::string;
using netlib::SocketAddress;

SocketAddress::SocketAddress(int port)
{
	bzero(&address_, sizeof address_);
	address_.sin_family = AF_INET; // IPv4
	// Network byte order is big-endian.
	address_.sin_port = htobe16(static_cast<uint16_t>(port));
	address_.sin_addr.s_addr = htobe32(INADDR_ANY); // Kernel choose IPv4 address.
}
SocketAddress::SocketAddress(string ip, int port)
{
	bzero(&address_, sizeof address_);
	address_.sin_family = AF_INET;
	address_.sin_port = htobe16(static_cast<uint16_t>(port));
	if(::inet_pton(AF_INET, ip.c_str(), &address_.sin_addr) != 1)
	{
		LOG_ERROR("inet_pton(): ERROR");
	}
}

const struct sockaddr *SocketAddress::socket_address() const
{
	return nso::CastToConstsockaddr(&address_);
}

string SocketAddress::ToIpPortString() const
{
	char ip[16], ip_port[32]; // ip "255.255.255.255\0" is 16B; port is uint16_t.
	::inet_ntop(address_.sin_family, &address_.sin_addr, ip, sizeof ip);
	::snprintf(ip_port, sizeof ip_port, "%s:%u", ip, be16toh(address_.sin_port));
	return ip_port;
}
