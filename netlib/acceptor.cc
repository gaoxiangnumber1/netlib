#include <netlib/acceptor.h>

namespace nso = netlib::socket_operation;
/************************************************************************
Since the connections are too complex, work to know one line at a time.
*************************************************************************/

Acceptor::Acceptor(EventLoop *loop, const SocketAddress &listen_address):
	loop_(loop),
	// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
	accept_socket_(nso::CreateNonblockingOrDie()),
	accept_channel_(loop_, accept_socket_.socket_fd()),
	listening_(false)
{
	accept_socket_.SetReuseAddress(true); // Enable SO_REUSEADDR
	accept_socket_.BindAddress(listen_address); // Wrapper for ::bind().
	accept_channel_.set_read_callback(bind(&Acceptor::ReadCallback, this));
}

void ReadCallback()
{
	loop_->AssertInLoopThread();
	SocketAddress peer_address(0); // Construct an endpoint with given port number.
	int connection_fd = accept_socket_.Accept(&peer_address);
	if(connection_fd >= 0)
	{
		if(new_connection_callback_) // void(int, const InetAddress&)
		{
			new_connection_callback_(connection_fd, peer_address);
		}
		else
		{
			nso::Close(connection_fd);
		}
	}
}
