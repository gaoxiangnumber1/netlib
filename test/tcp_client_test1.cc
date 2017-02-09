#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/tcp_client.h>

using netlib::TcpClient;
using netlib::EventLoop;
using netlib::SocketAddress;
using std::bind;

TcpClient *g_client;

void Timeout()
{
	LOG_INFO("Timeout");
	g_client->Stop();
}

int main()
{
	EventLoop loop;
	SocketAddress server_address("127.0.0.1", 2);
	TcpClient client(&loop, server_address, "TcpClient");
	g_client = &client;
	loop.RunAfter(Timeout, 0.0);
	loop.RunAfter(bind(&EventLoop::Quit, &loop), 1.0);
	client.Connect();
	loop.Loop();
}
