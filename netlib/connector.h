#ifndef NETLIB_NETLIB_CONNECTOR_H_
#define NETLIB_NETLIB_CONNECTOR_H_

#include <netlib/function.h>
#include <netlib/non_copyable.h>
#include <netlib/socket_address.h>

namespace netlib
{

class EventLoop;
class Channel;

// Interface:
// Ctor
// Dtor
// server_address
// set_new_connection_callback
// Start -> -StartInLoop
//			-StartInLoop -> -Connect
//						-Connect -> -Connecting -> -Retry
//									-Connecting -> -HandleWrite -> -HandleError
//												-HandleWrite -> -RemoveAndResetChannel -> -Retry -> -IsSelfConnect
//															-RemoveAndResetChannel -> -ResetChannel
//												-HandleError -> -RemoveAndResetChannel -> -Retry
//									-Retry -> -StartInLoop
// Restart -> -StartInLoop
// Stop -> -StopInLoop
//			-StopInLoop -> -RemoveAndResetChannel -> -Retry

class Connector: public NonCopyable,
	public std::enable_shared_from_this<Connector>
{
public:
	using NewConnectionCallback = std::function<void(int)>;

	Connector(EventLoop *loop, const SocketAddress &server_address);
	~Connector();

	const SocketAddress &server_address() const
	{
		return server_address_;
	}
	void set_new_connection_callback(const NewConnectionCallback &callback)
	{
		new_connection_callback_ = callback;
	}

	void Start(); // Can be called in any thread.
	void Restart(); // Must be called in loop thread.
	void Stop(); // Can be called in any thread.

private:
	enum State {DISCONNECTED, CONNECTING, CONNECTED};
	static const double kMaxRetryDelaySecond;
	static const double kInitialRetryDelaySecond;

	void set_state(State state)
	{
		state_ = state;
	}
	void StartInLoop();
	void Connect();
	void Connecting(int socket_fd);
	void Retry(int socket_fd);
	void StopInLoop();
	int RemoveAndResetChannel();
	void ResetChannel();
	void HandleWrite();
	bool IsSelfConnect(int socket_fd);
	void HandleError();

	EventLoop *loop_;
	SocketAddress server_address_;
	bool connect_; // FIXME: use atomic variable.
	State state_; // FIXME: use atomic variable.
	double retry_delay_second_;
	std::unique_ptr<Channel> channel_;
	NewConnectionCallback new_connection_callback_;
};

}

#endif // NETLIB_NETLIB_CONNECTOR_H_
