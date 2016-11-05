#ifndef NETLIB_NETLIB_ACCEPTOR_H_
#define NETLIB_NETLIB_ACCEPTOR_H_

#include <netlib/non_copyable.h>

#include <netlib/socket.h>
#include <netlib/channel.h>

namespace netlib
{

class EventLoop;
class SocketAddress;

class Acceptor: public NonCopyable
{
public:
	Acceptor(EventLoop *loop, const SocketAddress &listen_address);

	bool listening() const
	{
		return listening_;
	}

	void set_new_connection_callback(const NewConnectionCallback &callback)
	{
		new_connection_callback_ = callback;
	}

	void Listen();

private:
	void ReadCallback();

	EventLoop *loop_;
	Socket accept_socket_; // A listening socket, i.e., a server socket.
	Channel accept_channel_; // Monitors the IO events(readable) of accept_socket_.
	NewConnectionCallback new_connection_callback_;
	bool listening_;
};

}

#endif // NETLIB_NETLIB_ACCEPTOR_H_
