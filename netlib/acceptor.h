#ifndef NETLIB_NETLIB_ACCEPTOR_H_
#define NETLIB_NETLIB_ACCEPTOR_H_

#include <functional> // function<>

#include <netlib/channel.h>
#include <netlib/non_copyable.h>
#include <netlib/socket.h>

namespace netlib
{

class EventLoop;
class SocketAddress;

// Acceptor is used to accept(2) new TCP connections and it notifies its caller by
// new_connection_callback_. This is an internal class, used by TcpServer, and
// its lifetime is controlled by TcpServer class.
class Acceptor: public NonCopyable
{
public:
	// NewConnectionCallback is used only in Acceptor class,
	// so we don't put it in callback.h.
	using NewConnectionCallback = std::function<void(int, const SocketAddress&)>;

	Acceptor(EventLoop *owner_loop,
	         const SocketAddress &listen_address,
	         bool is_reuse_port);
	~Acceptor();

	bool listening() const
	{
		return listening_;
	}
	// Called in TcpServer Ctor: `acceptor_->set_new_connection_callback(
	//					    bind(&TcpServer::NewConnectionCallback, this, _1, _2));`
	void set_new_connection_callback(const NewConnectionCallback &callback)
	{
		new_connection_callback_ = callback;
	}
	// Let accept_channel_ dispatch IO readable events and let accept_socket_ to listen().
	void Listen();

private:
	// Call accept(2) to accept new connections and call user's callback.
	void HandleRead();

	EventLoop *owner_loop_; // owner loop.
	Socket accept_socket_; // A listening socket, i.e., a server socket.
	Channel accept_channel_;
	NewConnectionCallback new_connection_callback_;
	bool listening_;
	int idle_fd_; // TODO: Read muduo-7.7.1
};

}

#endif // NETLIB_NETLIB_ACCEPTOR_H_
