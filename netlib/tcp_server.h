#ifndef NETLIB_NETLIB_TCP_SERVER_H_
#define NETLIB_NETLIB_TCP_SERVER_H_

#include <map>
#include <string>

#include <netlib/function.h>
#include <netlib/non_copyable.h>
#include <netlib/tcp_connection.h>

namespace netlib
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class SocketAddress;

// Interface:
// Ctor -> -HandleNewConnection
//			-HandleNewConnection -> -RemoveConnection
//						-RemoveConnection -> -RemoveConnectionInLoop
// Dtor.
// Setter: connection_ptr, message, write_complete
// Start.

class TcpServer: public NonCopyable
{
public:
	TcpServer(EventLoop *main_loop,
	          const SocketAddress &server_address,
	          const std::string &name,
	          int loop_number = 0);
	~TcpServer(); // Force outline destructor, for smart_ptr members.

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

	void Start();

private:
	using ConnectionNamePtrMap = std::map<std::string, TcpConnectionPtr>;

	void HandleNewConnection(int socket, const SocketAddress &client_address);
	void RemoveConnection(const TcpConnectionPtr &connection_ptr);
	void RemoveConnectionInLoop(const TcpConnectionPtr &connection_ptr);

	EventLoop *main_loop_;
	const std::string server_ip_port_;
	const std::string server_name_;
	std::shared_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> loop_pool_;
	bool started_; // FIXME: Atomic.
	ConnectionNamePtrMap connection_name_ptr_map_;
	int next_connection_id_; // Start from 1.
	ConnectionCallback connection_callback_;
	MessageCallback message_callback_;
	WriteCompleteCallback write_complete_callback_;
};

}

#endif // NETLIB_NETLIB_TCP_SERVER_H_
