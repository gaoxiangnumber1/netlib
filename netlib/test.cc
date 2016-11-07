#include <stdio.h>

#include <netlib/tcp_connection.h>
#include <netlib/time_stamp.h>
#include <netlib/socket_address.h>
#include <netlib/event_loop.h>
#include <netlib/tcp_server.h>

using netlib::TcpConnectionPtr;
using netlib::Buffer;
using netlib::TimeStamp;
using netlib::SocketAddress;
using netlib::EventLoop;
using netlib::TcpServer;

void onConnection(const TcpConnectionPtr& conn)
{
	if (conn->Connected())
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
               Buffer* buf,
               TimeStamp receiveTime)
{
	printf("onMessage(): received %d bytes from connection [%s] at %s\n",
	       buf->ReadableByte(),
	       conn->name().c_str(),
	       receiveTime.ToFormattedString().c_str());
	printf("onMessage(): [%s]\n", buf->RetrieveAsString().c_str());
}

int main()
{
	printf("main(): pid = %d\n", getpid());

	SocketAddress listenAddr(7188);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.set_connection_callback(onConnection);
	server.set_message_callback(onMessage);
	server.Start();

	loop.Loop();
}
