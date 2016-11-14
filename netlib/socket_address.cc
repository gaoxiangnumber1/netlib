#include <netlib/socket_address.h>

#include <strings.h> // bzero()
#include <endian.h> // htobe*()
#include <arpa/inet.h> // inet_ntop(), inet_pton()

#include <netlib/socket_operation.h>
#include <netlib/logging.h>

using std::string;
using netlib::SocketAddress;

// 	struct in_addr
// 	{
// 		uint32_t s_addr; // IPv4 address in network byte ordered.
// 	};
// 	struct sockaddr_in // An IPv4 socket address structure.
// 	{
// 		uint8_t sin_len; // length of structure (16)
// 		sa_family_t sin_family; // Address family: AF_INET
// 		uint16_t sin_port; // Port number in network byte ordered
// 		struct in_addr sin_addr; // 32-bit IPv4 address, network byte ordered
// 		char sin_zero[8]; // unused
// 	};

SocketAddress::SocketAddress(int port)
{
	bzero(&address_, sizeof address_);
	address_.sin_family = AF_INET; // IPv4 Internet protocols
	// Network byte order is big-endian byte order. uint16_t htobe16(uint16_t);
	address_.sin_port = htobe16(static_cast<uint16_t>(port));
	// The constant INADDR_ANY tells the kernel to choose the IPv4 address.
	address_.sin_addr.s_addr = htobe32(INADDR_ANY);
}
SocketAddress::SocketAddress(string ip, int port)
{
	bzero(&address_, sizeof address_);
	address_.sin_family = AF_INET;
	address_.sin_port = htobe16(static_cast<uint16_t>(port));
	// #include <arpa/inet.h>
	// int inet_pton(int af, const char *src, void *dst);
	// Return 1 on success.
	if(::inet_pton(AF_INET, ip.c_str(), &address_.sin_addr) != 1)
	{
		LOG_ERROR("sockets::fromIpPort");
	}
}

const struct sockaddr *SocketAddress::socket_address() const
{
	return nso::CastToConstsockaddr(&address_);
}

string SocketAddress::ToIpPortString() const
{
	char host[INET_ADDRSTRLEN] = ""; // Defined <netinet/in.h>, 16
	// #include <arpa/inet.h>
	// const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
	::inet_ntop(AF_INET, &address_.sin_addr, host, sizeof host);
	// uint16_t be16toh(uint16_t)
	uint16_t port = be16toh(address_.sin_port);

	char buffer[32] = "";
	::snprintf(buffer, sizeof buffer, "%s:%u", host, port);
	return buffer;
}
