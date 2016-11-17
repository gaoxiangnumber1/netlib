#include <netlib/socket_address.h>

#include <assert.h>

#include <string>

#include <netlib/logging.h>

using std::string;
using netlib::SocketAddress;

int main()
{
	LOG_INFO("Begin test.");
	SocketAddress address1(7188);
	assert(address1.ToIpPortString() == string("0.0.0.0:7188"));

	SocketAddress address2("1.2.3.4", 7188);
	assert(address2.ToIpPortString() == string("1.2.3.4:7188"));

	SocketAddress address3("255.254.253.252", 65535);
	assert(address3.ToIpPortString() == string("255.254.253.252:65535"));
	LOG_INFO("All passed.")
}
