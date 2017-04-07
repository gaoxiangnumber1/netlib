#ifndef NETLIB_NETLIB_TCP_CONNECTION_H_
#define NETLIB_NETLIB_TCP_CONNECTION_H_

#include <string>

#include <netlib/buffer.h>
#include <netlib/function.h>
#include <netlib/non_copyable.h>
#include <netlib/socket_address.h>

namespace netlib
{

class EventLoop;
class Socket;
class Channel;

// Interface:
// Ctor -> -HandleRead -> -HandleWrite -> -HandleClose -> -HandleError
//			-HandleRead -> -HandleClose -> -HandleError
//			-HandleWrite -> -ShutdownInLoop
// Dtor
// Getter: loop, name, local_address, peer_address, input_buffer, output_buffer, context
// Setter: connection/message/high_water_mark/write_complete/close_callback/context
// Connected/Disconnected
// SetTcpNoDelay
// ConnectEstablished -> -set_state
// Send(const void*, int)/(const string&)/(Buffer*) -> -SendInLoop
// Shutdown -> -ShutdownInLoop.
// ForceClose -> -ForceCloseInLoop
//			-ForceCloseInLoop -> -HandleClose
// ConnectDestroyed

// TCP connection, for both client and server usage.
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
	              const std::string &string_name,
	              int socket,
	              const SocketAddress &client,
	              const SocketAddress &server);
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
	const SocketAddress &client_address() const
	{
		return client_address_;
	}
	const SocketAddress &server_address() const
	{
		return server_address_;
	}
	Buffer *input_buffer()
	{
		return &input_buffer_;
	}
	Buffer *output_buffer()
	{
		return &output_buffer_;
	}
	void *context()
	{
		return context_;
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
	void set_high_water_mark_callback(const HighWaterMarkCallback &callback,
	                                  int high_water_mark)
	{
		high_water_mark_callback_ = callback;
		high_water_mark_ = high_water_mark;
	}
	void set_write_complete_callback(const WriteCompleteCallback &callback)
	{
		write_complete_callback_ = callback;
	}
	// Internal use only.
	void set_close_callback(const CloseCallback &callback)
	{
		close_callback_ = callback;
	}
	void set_context(void *context_arg)
	{
		context_ = context_arg;
	}

	bool Connected() const
	{
		return state_ == CONNECTED;
	}
	bool Disconnected() const
	{
		return state_ == DISCONNECTED;
	}
	void SetTcpNoDelay(bool on);

	// Called when TcpServer accept a new connection. Should be called only once.
	void ConnectEstablished();
	// Thread safe.
	void Send(const void *data, int length);
	void Send(const std::string &message);
	void Send(Buffer *message); // Swap data
	// TODO: NOT thread safe, no simultaneous calling.
	void Shutdown();
	void ForceClose();
	// Called when TcpServer remove this TcpConnection from its map.
	// Should be called only once.
	void ConnectDestroyed();

private:
	enum State
	{
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
		DISCONNECTED
	};
	void set_state(State state)
	{
		state_ = state;
	}
	const char * StateToCString() const;

	// Channel's *_callback_ is Handle*() here.
	void HandleRead(TimeStamp receive_time);
	void HandleWrite();
	void HandleClose();
	void HandleError();

	void ShutdownInLoop();
	void SendInLoop(const char *data, int length);
	void ForceCloseInLoop();

	EventLoop *loop_;
	// "listen_address.ToIpPortString()" + "#next_connection_id_". Set by TcpServer.
	const std::string name_;
	State state_; // FIXME: Use atomic variable
	void *context_;
	// TcpConnection owns the connection_fd returned by `accept_socket_.Accept(p_a)`,
	// its destructor will `::close(socket_)` in the destructor of the Socket object.
	std::unique_ptr<Socket> socket_;
	// TcpConnection deals with the writable IO events by itself,
	// and pass readable events to user by message_callback_.
	std::unique_ptr<Channel> channel_;
	const SocketAddress client_address_;
	const SocketAddress server_address_;
	Buffer input_buffer_;
	Buffer output_buffer_; // FIXME: Use list<Buffer> as output buffer.
	// TcpServer::HandleNewConnection -> TcpConnection::ConnectEstablished() ->
	// connection_callback_.
	ConnectionCallback connection_callback_;
	MessageCallback message_callback_; // Called in HandleRead().
	HighWaterMarkCallback high_water_mark_callback_;
	int high_water_mark_;
	static const int kInitialHighWaterMark = 64 * 1024 * 1024;
	WriteCompleteCallback write_complete_callback_;
	// close_callback_ can be only used by TcpServer and TcpClient, to notify that
	// they should remove this TcpConnection object's shared_ptr; not used by user,
	// user only use connection_callback_.
	// Called in HandleClose(). Bind to TcpServer/TcpClient::RemoveConnection().
	CloseCallback close_callback_;
};

void DefaultConnectionCallback(const TcpConnectionPtr&);
void DefaultMessageCallback(const TcpConnectionPtr&, Buffer*, const TimeStamp&);

}

#endif // NETLIB_NETLIB_TCP_CONNECTION_H_
