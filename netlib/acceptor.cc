#include <unistd.h>
#include <netlib/acceptor.h>
#include <netlib/event_loop.h>
#include <netlib/socket_address.h>

namespace nso = netlib::socket_operation;
using std::bind;
using netlib::Acceptor;

Acceptor::Acceptor(EventLoop *loop, const SocketAddress &listen_address):
	loop_(loop),
	// Create an IPv4, nonblocking, and TCP socket file descriptor, abort if any error.
	accept_socket_(nso::CreateNonblockingOrDie()),
	accept_channel_(loop_, accept_socket_.socket_fd()),
	listening_(false)
{
	accept_socket_.SetReuseAddress(true); // Enable SO_REUSEADDR
	accept_socket_.BindAddress(listen_address); // Wrapper for ::bind().
	accept_channel_.set_read_callback(bind(&Acceptor::HandleRead, this));
}

// Let accept_channel_ monitor IO readable events and let accept_socket_ to listen().
// Called by `TcpServer::Start()`
void Acceptor::Listen()
{
	loop_->AssertInLoopThread();
	listening_ = true;
	// Monitor IO readable events. The trigger for Channel::HandleEvent() is
	// that the listening socket is readable, which indicates new connection arrives.
	accept_channel_.set_requested_event_read();
	accept_socket_.Listen();
}

// When the listening socket(i.e., the accept_socket_) is readable, Poller::Poll()
// will return and calls Channel::HandleEvent(), that in turn calls this function.
// Call accept_socket_.Accept() to accept(2) new connections and call new connection
// callback if user supply.
void Acceptor::HandleRead()
{
	loop_->AssertInLoopThread();

	SocketAddress peer_address(0); // Construct an endpoint with given port number.
	// FIXME: Here we accept(2) one socket each time, which is suitable for long
	// connection. There two strategies for short-connection:
	// 1.	accept(2) until no more new connections arrive.
	// 2.	accept(2) N connections at a time, N normally is 10.
	int connection_fd = accept_socket_.Accept(&peer_address);
	// TODO: See 7.7 "How to avoid the `haojin` of file descriptors?"
	if(connection_fd >= 0)
	{
		if(new_connection_callback_) // void(int, const InetAddress&)
		{
			// FIXME: Here pass the connection_fd by value, which may not free memory
			// properly. C++11: create Socket object -> use std::move() to move this object
			// to new_connection_callback. This Guarantee the safe memory release.
			new_connection_callback_(connection_fd, peer_address);
		}
		else
		{
			::close(connection_fd);
		}
	}
}
