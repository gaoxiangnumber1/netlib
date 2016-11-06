#include <stdio.h>

#include <netlib/tcp_server.h>
#include <netlib/event_loop.h>
#include <netlib/socket_address.h>

using netlib::TcpConnectionPtr;
using netlib::EventLoop;
using netlib::SocketAddress;
using netlib::TcpServer;

void onConnection(const TcpConnectionPtr& conn)
{
	if(conn->connected())
	{
		printf("onConnection(): new connection [%s] from %s\n",
		       conn->name().c_str(),
		       conn->peer_address().ToHostPort().c_str());
	}
	else
	{
		printf("onConnection(): connection [%s] is down\n",
		       conn->name().c_str());
	}
}

void onMessage(const TcpConnectionPtr& conn,
               const char* data,
               ssize_t len)
{
	printf("onMessage(): received %zd bytes from connection [%s]\n",
	       len, conn->name().c_str());
}

int main()
{
	printf("main(): pid = %d\n", getpid());

	SocketAddress listenAddr(9981);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.set_connection_callback(onConnection);
	server.set_message_callback(onMessage);
	server.Start();

	loop.Loop();
}
