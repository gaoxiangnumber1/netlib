#include <stdio.h>

#include <algorithm>

#include <netlib/tcp_connection.h>
#include <netlib/time_stamp.h>
#include <netlib/socket_address.h>
#include <netlib/event_loop.h>
#include <netlib/thread.h>
#include <netlib/tcp_server.h>

using netlib::TcpConnectionPtr;
using netlib::Thread;
using netlib::Buffer;
using netlib::SocketAddress;
using netlib::EventLoop;
using netlib::TcpServer;
using netlib::TimeStamp;

void onConnection(const TcpConnectionPtr& conn)
{
	if(conn->Connected())
	{
		printf("onConnection(): tid=%d new connection [%s] from %s\n",
		       Thread::ThreadId(),
		       conn->name().c_str(),
		       conn->peer_address().ToHostPort().c_str());
	}
	else
	{
		printf("onConnection(): tid=%d connection [%s] is down\n",
		       Thread::ThreadId(),
		       conn->name().c_str());
	}
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               TimeStamp receiveTime)
{
	printf("onMessage(): tid=%d received %d bytes from connection [%s] at %s\n",
	       Thread::ThreadId(),
	       buf->ReadableByte(),
	       conn->name().c_str(),
	       receiveTime.ToFormattedString().c_str());

	printf("onMessage(): [%s]\n", buf->RetrieveAsString().c_str());
}

int main(int argc, char* argv[])
{
	printf("main(): pid = %d\n", getpid());

	SocketAddress listenAddr(7188);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.set_connection_callback(onConnection);
	server.set_message_callback(onMessage);
	if(argc > 1)
	{
		printf("cmd: %d\n", atoi(argv[1]));
		server.SetThreadNumber(atoi(argv[1]));
	}
	printf("Before server.Start()\n");
	server.Start();

	loop.Loop();
}
