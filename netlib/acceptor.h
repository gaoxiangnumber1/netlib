#ifndef NETLIB_NETLIB_ACCEPTOR_H_
#define NETLIB_NETLIB_ACCEPTOR_H_

#include <netlib/non_copyable.h>
#include <netlib/socket.h>
#include <netlib/channel.h>

namespace netlib
{

class EventLoop;
class SocketAddress;

// Acceptor is used to accept(2) new TCP connections and it notify its caller by
// new_connection_callback_. This is an internal class, used by TcpServer, and
// its lifetime is controlled by TcpServer class.
class Acceptor: public NonCopyable
{
public:
	// NewConnectionCallback is only used in Acceptor class,
	// so we don't put it in callback.h.
	using NewConnectionCallback = std::function<void(int, const SocketAddress&)>;

	Acceptor(EventLoop *loop, const SocketAddress &listen_address);

	bool listening() const
	{
		return listening_;
	}
	// Called in TcpServer constructor: acceptor_->set_new_connection_callback(
	//					    bind(&TcpServer::NewConnectionCallback, this, _1, _2));
	// `void NewConnectionCallback(int socket_fd, const SocketAddress &peer_address);`
	void set_new_connection_callback(const NewConnectionCallback &callback)
	{
		new_connection_callback_ = callback;
	}

	void Listen();

private:
	// Call accept(2) to accept new connections and call user's callback.
	void ReadCallback();

	EventLoop *loop_;
	Socket accept_socket_; // A listening socket, i.e., a server socket.
	// Monitor the IO readable events of accept_socket_, and then call ReadCallback().
	Channel accept_channel_;
	NewConnectionCallback new_connection_callback_;
	bool listening_;
};

}

#endif // NETLIB_NETLIB_ACCEPTOR_H_
