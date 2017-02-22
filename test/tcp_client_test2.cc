// TcpClient destructs when TcpConnection is connected but unique.

#include <unistd.h> // sleep

#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/tcp_client.h>
#include <netlib/thread.h>

using netlib::TcpClient;
using netlib::EventLoop;
using netlib::SocketAddress;
using netlib::Thread;
using std::bind;

void ThreadFunction(EventLoop *loop)
{
	SocketAddress server_address("127.0.0.1", 1000);
	TcpClient client(loop, server_address, "TcpClient");
	client.Connect();
	::sleep(10);
}

int main()
{
	SetLogLevel(DEBUG);
	EventLoop loop;
	loop.RunAfter(bind(&EventLoop::Quit, &loop), 3.0);
	Thread thread(bind(ThreadFunction, &loop));
	thread.Start();
	loop.Loop();
}
