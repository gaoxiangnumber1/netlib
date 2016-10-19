#include "echo.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

using muduo::net::EventLoop;
using muduo::net::InetAddress;

int main()
{
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	InetAddress listen_address(7188); // 'G' = 71, 'X' = 88
	EchoServer server(&loop, listen_address);
	server.Start();
	loop.loop();
}
