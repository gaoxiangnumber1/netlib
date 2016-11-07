#include <netlib/tcp_connection.h>

#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket.h>
#include <netlib/socket_operation.h>
#include <netlib/channel.h>

using std::string;
using std::bind;
using std::placeholders::_1;
using netlib::TcpConnection;

TcpConnection::TcpConnection(EventLoop *event_loop,
                             const string string_name,
                             int socket,
                             const SocketAddress &local,
                             const SocketAddress &peer):
	loop_(CHECK_NOT_NULL(event_loop)),
	name_(string_name),
	state_(CONNECTING),
	socket_(new Socket(socket)),
	channel_(new Channel(loop_, socket)),
	local_address_(local),
	peer_address_(peer)
{
	LOG_INFO("TcpConnection::ctor[%s] at %p fd=%d", name_.c_str(), this, socket);
	channel_->set_read_callback(bind(&TcpConnection::HandleRead, this, _1));
	channel_->set_write_callback(bind(&TcpConnection::HandleWrite, this));
	channel_->set_close_callback(bind(&TcpConnection::HandleClose, this));
	channel_->set_error_callback(bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection()
{
	LOG_INFO("TcpConnection::dtor[%s] at %p fd=%d",
	         name_.c_str(), this, channel_->fd());
}

// Called by TcpServer::HandleNewConnection().
void TcpConnection::ConnectEstablished()
{
	loop_->AssertInLoopThread();
	assert(state_ == CONNECTING);
	set_state(CONNECTED);
	// Monitor the IO readable events on socket_.
	channel_->set_requested_event_read();
	// std::function<void(const TcpConnectionPtr&)>;
	connection_callback_(shared_from_this());
}

// This is the last member function called by TcpConnection object before destructing.
// It notifies the user that the connection is down.
void TcpConnection::ConnectDestroyed()
{
	loop_->AssertInLoopThread();
	assert(state_ == CONNECTED || state_ == DISCONNECTING);
	set_state(DISCONNECTED);
	// This line is repeated as in HandleClose() since we may call ConnectDestroyed()
	// directly, not through HandleClose().
	channel_->set_requested_event_none();
	connection_callback_(shared_from_this());
	loop_->RemoveChannel(channel_.get());
}

// Called when socket_ is readable.
void TcpConnection::HandleRead(TimeStamp receive_time)
{
	int saved_errno = 0;
	int read_byte = input_buffer_.ReadFd(channel_->fd(), &saved_errno);
	if(read_byte > 0)
	{
		message_callback_(shared_from_this(), &input_buffer_, receive_time);
	}
	else if(read_byte == 0)
	{
		HandleClose();
	}
	else
	{
		errno = saved_errno;
		HandleError();
	}
}

void TcpConnection::HandleWrite() {}

void TcpConnection::HandleClose()
{
	loop_->AssertInLoopThread();
	assert(state_ == CONNECTED || state_ == DISCONNECTING);
	// We don't close fd, leave it to dtor, so we can find leaks easily.
	channel_->set_requested_event_none();
	close_callback_(shared_from_this());
}

void TcpConnection::HandleError()
{
	int error = nso::GetSocketError(channel_->fd());
	LOG_INFO("TcpConnection::handleError [%s] - SO_ERROR = %d %s",
	         name_.c_str(), error, ThreadSafeStrError(error));
}

void TcpConnection::Shutdown()
{
	// FIXME: use compare and swap
	if(state_ == CONNECTED)
	{
		set_state(DISCONNECTING);
		// FIXME: shared_from_this()?
		loop_->RunInLoop(bind(&TcpConnection::ShutdownInLoop, this));
	}
}

void TcpConnection::ShutdownInLoop()
{
	loop_->AssertInLoopThread();
	if(channel_->IsWriting() == false)
	{
		socket_->ShutdownWrite();
	}
}

void TcpConnection::Send(const std::string &message)
{
	if(state_ == CONNECTED)
	{
		if(loop_>IsInLoopThread() == true)
		{
			SendInLoop(message);
		}
		else
		{
			// TODO: Use C++11::move(string&&) to avoid the copy of string content.
			loop_->RunInLoop(bind(&TcpConnection::SendInLoop, this, message));
		}
	}
}

void TcpConnection::SendInLoop(const std::string &message)
{
	loop_->AssertInLoopThread();

	int write_byte = 0;
	// If we are not writing and there is no data in the output_buffer_,
	// try writing directly. If we still send data when output_buffer_ is not empty,
	// the data may be out of order.
	if (channel_->IsWriting() == false && output_buffer_.ReadableByte() == 0)
	{
		write_byte = static_cast<int>(::write(channel_->fd(), message.data(), message.size()));
		if(write_byte >= 0)
		{
			if (write_byte < message.size())
			{
				LOG_INFO("Not send all data.");
			}
		}
		else
		{
			write_byte = 0;
			if(errno != EWOULDBLOCK)
			{
				LOG_INFO("TcpConnection::SendInLoop error");
			}
		}
	}

	assert(write_byte >= 0);
	// Only send partial data, store left data in output_buffer_ and monitor
	// IO writable events. Send left data in HandleWrite().
	if(write_byte < message.size())
	{
		output_buffer_.Append(message.data() + write_byte, message.size() - write_byte);
		if (channel_->IsWriting() == false)
		{
			channel_->set_requested_event_write();
		}
	}
}
