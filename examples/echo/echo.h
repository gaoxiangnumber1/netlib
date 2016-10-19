#ifndef NETLIB_EXAMPLES_ECHO_ECHO_H_
#define NETLIB_EXAMPLES_ECHO_ECHO_H_

//A header that has a using directive or declaration at its top-level scope injects names
//into every file that includes the header. Headers should define only the names that are
//part of its interface, not names used in its own implementation. Header files should
//not contain using directives or using declarations except inside functions or
//namespaces(3.1).		CPP-Primer-18.2

#include <muduo/net/TcpServer.h>

// RFC 862
class EchoServer
{
public:
	EchoServer(muduo::net::EventLoop *loop,
	           const muduo::net::InetAddress &listen_address); // Constructor.

	void Start(); // call server_.start()

private:
	// Callback function when there is a new connection.
	void OnConnection(const muduo::net::TcpConnectionPtr &connection);

	void OnMessage(const muduo::net::TcpConnectionPtr &connection,
	               muduo::net::Buffer *buffer,
	               muduo::Timestamp time); // Callback function when there has new message.

	muduo::net::TcpServer server_;
};

#endif // NETLIB_EXAMPLES_ECHO_ECHO_H_
