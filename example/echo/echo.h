#ifndef NETLIB_EXAMPLE_ECHO_ECHO_H_
#define NETLIB_EXAMPLE_ECHO_ECHO_H_

#include <netlib/tcp_server.h>

// Interface:
// Ctor -> -HandleConnection -> -HandleMessage
// Start

class EchoServer
{
public:
	EchoServer(netlib::EventLoop*, const netlib::SocketAddress&);
	void Start();

private:
	void HandleConnection(const netlib::TcpConnectionPtr&);
	void HandleMessage(const netlib::TcpConnectionPtr&, netlib::Buffer*, netlib::TimeStamp);

	netlib::TcpServer server_;
};

#endif // NETLIB_EXAMPLE_ECHO_ECHO_H_
