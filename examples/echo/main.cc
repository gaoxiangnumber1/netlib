#include "echo.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>

using muduo::net::EventLoop;
using muduo::net::InetAddress;

int main()
{
	LOG_INFO << "gaoxiang";
	EventLoop loop;
	InetAddress listen_address(7188); // 'G' = 71, 'X' = 88
	EchoServer server(&loop, listen_address);
	server.Start();
	loop.loop();
}
