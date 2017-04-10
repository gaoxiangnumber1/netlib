#ifndef NETLIB_NETLIB_TCP_CLIENT_H_
#define NETLIB_NETLIB_TCP_CLIENT_H_

#include <string>

#include <netlib/function.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>
#include <netlib/tcp_connection.h>

namespace netlib
{

class EventLoop;
class SocketAddress;
class Connector;

// Interface:
// Ctor -> -HandleNewConnection
//			-HandleNewConnection -> -RemoveConnection
// Dtor
// Getter: loop
// Setter: set_connection/message/write_complete_callback
// EnableRetry
// Connect
// Disconnect
// Stop

class TcpClient: public NonCopyable
{
public:
	TcpClient(EventLoop *event_loop,
	          const SocketAddress &server_address,
	          const std::string &name);
	~TcpClient(); // Force out-line dtor, for shared_ptr members.

	EventLoop *loop() const
	{
		return main_loop_;
	}
	// All three set_*_callback are Not thread safe!
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

	void EnableRetry()
	{
		retry_ = true;
	}
	void Connect();
	void Disconnect();
	void Stop();

private:
	using ConnectorPtr = std::shared_ptr<Connector>;
	// Not thread safe, but in loop.
	void HandleNewConnection(int socket);
	// Not thread safe, but in loop.
	void RemoveConnection(const TcpConnectionPtr &connection);

	EventLoop *main_loop_;
	ConnectorPtr connector_;
	const std::string name_;
	bool retry_; // FIXME: atomic.
	bool connect_; // FIXME: atomic.
	int next_connection_id_; // FIXME: Let server pass its connection_id as client's id.
	MutexLock mutex_;
	TcpConnectionPtr connection_; // Guarded by mutex_.
	ConnectionCallback connection_callback_;
	MessageCallback message_callback_;
	WriteCompleteCallback write_complete_callback_;
};

}

#endif // NETLIB_NETLIB_TCP_CLIENT_H_
