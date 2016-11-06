#ifndef NETLIB_NETLIB_TCP_CONNECTION_H_
#define NETLIB_NETLIB_TCP_CONNECTION_H_

#include <netlib/non_copyable.h>
#include <netlib/socket_address.h>
#include <netlib/callback.h>

#include <memory>
#include <string>

namespace netlib
{

class EventLoop;
class Socket;
class Channel;

// Because of the ambiguous lifetime of TcpConnection, we use shared_ptr<> to
// manage TcpConnection object by default.
// One TcpConnection object represents "one TCP connection". Once the TCP
// connection is closed, the object of this TCP connection has no use at all.
// This class can't connect positively, its constructor's arguments are the connected
// socket file descriptor. So its initial state is CONNECTING.
class TcpConnection: public NonCopyable,
	public std::enable_shared_from_this<TcpConnection>
{
public:
	// Construct a TcpConnection with a connected socket.
	// User should not create this object.
	TcpConnection(EventLoop *event_loop,
	              const std::string string_name,
	              int socket,
	              const SocketAddress &local,
	              const SocketAddress &peer);
	~TcpConnection();

	EventLoop *loop() const
	{
		return loop_;
	}
	const std::string &name() const
	{
		return name_;
	}
	const SocketAddress &local_address() const
	{
		return local_address_;
	}
	const SocketAddress &peer_address() const
	{
		return peer_address_;
	}
	bool Connected() const
	{
		return state_ == CONNECTED;
	}
	void set_connection_callback(ConnectionCallback callback)
	{
		connection_callback_ = callback;
	}
	void set_message_callback(MessageCallback callback)
	{
		message_callback_ = callback;
	}
	// Internal use only. Called when TcpServer accepts a new connection.
	// Should be called only once
	void ConnectEstablished();

private:
	enum State {CONNECTING, CONNECTED};
	void set_state(State state)
	{
		state_ = state;
	}
	void ReadCallback();

	EventLoop *loop_;
	std::string name_;
	State state_; // FIXME: Use atomic variable
	// We don't expose following classes to client.
	// TcpConnection owns TCP socket, its destructor will `::close(socket_)` in the
	// destructor of Socket object.
	std::unique_ptr<Socket> socket_;
	// TcpConnection gets the socket_'s IO events by channel_. It deals with the writable
	// events by itself, and pass readable events to user by MessageCallback.
	std::unique_ptr<Channel> channel_;
	SocketAddress local_address_;
	SocketAddress peer_address_;
	// std::function<void(const TcpConnection&)>;
	ConnectionCallback connection_callback_;
	// Pass readable events to user by MessageCallback.
	MessageCallback message_callback_;
};

}

#endif // NETLIB_NETLIB_TCP_CONNECTION_H_
