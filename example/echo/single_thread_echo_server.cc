#include "echo.h"

#include <unistd.h>

#include <netlib/logging.h>
#include <netlib/event_loop.h>
#include <netlib/socket_address.h>
#include <netlib/acceptor.h>

using namespace netlib;

int main()
{
	LOG_INFO("pid = %d", getpid());
	EventLoop loop;
	SocketAddress listen_address(7188);
	EchoServer server(&loop, listen_address);
	server.Start();
	loop.Loop();
}
