#include <netlib/socket_address.h>

namespace nso = netlib::socket_operation;

SocketAddress::SocketAddress(int port)
{
	bzero(&address_, sizeof address_);
	address_.sin_family = AF_INET; // IPv4 Internet protocols
	// The constant INADDR_ANY tells the kernel to choose the IPv4 address.
	address_.sin_addr.s_addr = nso::HostToNetwork32(INADDR_ANY);
	address_.sin_port = nso::HostToNetwork16(port);
}
