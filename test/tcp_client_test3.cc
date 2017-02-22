#include <unistd.h> // sleep

#include <netlib/event_loop_thread.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/tcp_client.h>

using netlib::TcpClient;
using netlib::EventLoopThread;
using netlib::SocketAddress;
using std::bind;

int main()
{
	EventLoopThread loop_thread;
	{
		SocketAddress server_address("127.0.0.1", 1234);
		TcpClient client(loop_thread.StartLoop(), server_address, "TcpClient");
		client.Connect();
		sleep(1);
		client.Disconnect();
	}
	sleep(1);
}
