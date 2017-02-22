// TcpClient::Stop() is called in the same iteration of IO event.

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
	SocketAddress server_address("127.0.0.1", 2); // No such server.
	TcpClient client(&loop, server_address, "TcpClient");
	g_client = &client;
	loop.RunAfter(Timeout, 0.0);
	loop.RunAfter(bind(&EventLoop::Quit, &loop), 1.0);
	client.Connect();
	loop.Loop();
}

/* Sample output:
$ ./tcp_client_test1
21:21:24.297840 15247 INFO  ../netlib/tcp_client.cc:30 TcpClient::TcpClient[TcpClient] - connector 0x1ea3430
21:21:24.297938 15247 INFO  ../netlib/tcp_client.cc:118 TcpClient::connect[TcpClient] - connecting to 127.0.0.1:2
21:21:24.298027 15247 WARN  ../netlib/connector.cc:151 Connector::handleWrite - SO_ERROR = 111 Connection refused
21:21:24.298054 15247 INFO  ../netlib/connector.cc:199 Connector::retry - Retry connecting to 127.0.0.1:2 in 0.500000 seconds
21:21:24.298070 15247 ERROR ../netlib/connector.cc:222 Connector::handleError state = 0 Operation now in progress(errno=115)
21:21:24.298087 15247 INFO  tcp_client_test1.cc:15 Timeout
21:21:25.298046 15247 INFO  ../netlib/tcp_client.cc:85 TcpClient::~TcpClient[TcpClient] - connector 0x1ea3430
*/
