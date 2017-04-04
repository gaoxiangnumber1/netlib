#ifndef NETLIB_NETLIB_ACCEPTOR_H_
#define NETLIB_NETLIB_ACCEPTOR_H_

#include <functional>

#include <netlib/channel.h>
#include <netlib/non_copyable.h>
#include <netlib/socket.h>

namespace netlib
{

class EventLoop;
class SocketAddress;

// Interface:
// Ctor -> -HandleRead
// Dtor
// listening
// set_new_connection_callback
// Listen

class Acceptor: public NonCopyable
{
public:
	using NewConnectionCallback = std::function<void(int, const SocketAddress&)>;

	Acceptor(EventLoop *owner_loop, const SocketAddress &server_address);
	~Acceptor();

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
	void HandleRead();

	EventLoop *owner_loop_;
	Socket server_socket_;
	Channel server_channel_;
	bool listening_;
	int idle_fd_;
	NewConnectionCallback new_connection_callback_;
};

}

#endif // NETLIB_NETLIB_ACCEPTOR_H_
