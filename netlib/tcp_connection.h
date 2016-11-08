#ifndef NETLIB_NETLIB_TCP_CONNECTION_H_
#define NETLIB_NETLIB_TCP_CONNECTION_H_

#include <netlib/non_copyable.h>
#include <netlib/socket_address.h>
#include <netlib/callback.h>
#include <netlib/buffer.h>

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
	// Getter.
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
	// Setter.
	void set_connection_callback(const ConnectionCallback &callback)
	{
		connection_callback_ = callback;
	}
	void set_message_callback(const MessageCallback &callback)
	{
		message_callback_ = callback;
	}
	void set_write_complete_callback(const WriteCompleteCallback &callback)
	{
		write_complete_callback_ = callback;
	}
	void set_high_water_mark_callback(const HighWaterMarkCallback &callback,
	                                  int high_water_mark)
	{
		high_water_mark_callback_ = callback;
		high_water_mark_ = high_water_mark;
	}
	// Internal use only.
	void set_close_callback(const CloseCallback &callback)
	{
		close_callback_ = callback;
	}
	// Called when TcpServer accept a new connection. Should be called only once.
	void ConnectEstablished();
	// Called when TcpServer remove this TcpConnection from its map.
	// Should be called only once.
	void ConnectDestroyed();

	// Thread safe.
	void Send(const std::string &message);
	// Thread safe.
	void Shutdown();
	void SetTcpNoDelay(bool on);

private:
	enum State {CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED};
	void set_state(State state)
	{
		state_ = state;
	}
	// Channel's *_callback_ is Handle*() here.
	void HandleRead(TimeStamp receive_time);
	void HandleWrite();
	void HandleClose();
	void HandleError();

	void SendInLoop(const std::string &message);
	void ShutdownInLoop();

	EventLoop *loop_;
	// "listen_address.ToHostPort()" + "#next_connection_id_". Set by TcpServer.
	std::string name_;
	State state_; // FIXME: Use atomic variable
	// We don't expose following classes to client.
	// TcpConnection owns TCP socket, its destructor will `::close(socket_)` in the
	// destructor of Socket object.
	std::unique_ptr<Socket> socket_;
	// TcpConnection gets the socket_'s IO events by channel_. It deals with the writable
	// events by itself, and pass readable events to user by message_callback_.
	std::unique_ptr<Channel> channel_;
	SocketAddress local_address_;
	SocketAddress peer_address_;
	// std::function<void(const TcpConnectionPtr&)>;
	// TcpServer::HandleNewConnection -> TcpConnection::ConnectEstablished() ->
	// connection_callback_.
	ConnectionCallback connection_callback_;
	// std::function<void(const TcpConnectionPtr&, const char*, int)>
	// Called in HandleRead(), that is, when the socket is readable(message arrives).
	MessageCallback message_callback_;
	// close_callback_ can be only used by TcpServer and TcpClient, to notify that
	// they should remove this TcpConnection object's shared_ptr; not used by user,
	// user still use connection_callback_. Called in HandleClose().
	// Bind to TcpServer::RemoveConnection().
	WriteCompleteCallback write_complete_callback_;
	HighWaterMarkCallback high_water_mark_callback_;
	CloseCallback close_callback_;
	int high_water_mark_;
	Buffer input_buffer_;
	Buffer output_buffer_;
};

}

#endif // NETLIB_NETLIB_TCP_CONNECTION_H_
