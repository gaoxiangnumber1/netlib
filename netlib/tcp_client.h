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
	~TcpClient(); // Force outline dtor, for shared_ptr members.

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
		retryable_ = true;
	}
	void Connect();
	void Disconnect();
	void Stop();

private:
	void HandleNewConnection(int socket);
	void RemoveConnection(const TcpConnectionPtr &connection_ptr);

	EventLoop *main_loop_;
	std::shared_ptr<Connector> connector_;
	const std::string name_;
	bool retryable_; // FIXME: Atomic.
	bool connectable_; // FIXME: Atomic.
	int next_connection_id_; // FIXME: Get from server.
	MutexLock mutex_;
	TcpConnectionPtr connection_ptr_; // Guarded by mutex_.
	ConnectionCallback connection_callback_;
	MessageCallback message_callback_;
	WriteCompleteCallback write_complete_callback_;
};

}

#endif // NETLIB_NETLIB_TCP_CLIENT_H_
