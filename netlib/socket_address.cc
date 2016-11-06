#include <netlib/socket_address.h>
#include <netlib/socket_operation.h>

#include <strings.h> // bzero()

namespace nso = netlib::socket_operation;
using netlib::SocketAddress;
using std::string;

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
	address_.sin_port = nso::HostToNetwork16(port);
	// The constant INADDR_ANY tells the kernel to choose the IPv4 address.
	address_.sin_addr.s_addr = nso::HostToNetwork32(INADDR_ANY);
}

string SocketAddress::ToHostPort() const
{
	char buffer[32];
	nso::ToHostPort(buffer, 32, address_);
	return buffer;
}
