#include <netlib/tcp_connection.h>

#include <unistd.h> // write()

#include <netlib/channel.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket.h>
#include <netlib/socket_operation.h>

using std::string;
using std::bind;
using std::placeholders::_1;
using netlib::TcpConnection;

void netlib::DefaultConnectionCallback(const TcpConnectionPtr &connection_ptr)
{
	LOG_INFO("%s -> %s is %s",
	         connection_ptr->client_address().ToIpPortString().c_str(),
	         connection_ptr->server_address().ToIpPortString().c_str(),
	         (connection_ptr->Connected() ? "UP" : "DOWN"));
}
void netlib::DefaultMessageCallback(const TcpConnectionPtr &connection_ptr,
                                    Buffer *buffer,
                                    const TimeStamp &time_stamp)
{
	buffer->RetrieveAll();
}

TcpConnection::TcpConnection(EventLoop *event_loop,
                             const string &string_name,
                             int socket,
                             const SocketAddress &client,
                             const SocketAddress &server):
	loop_(event_loop),
	name_(string_name),
	state_(CONNECTING),
	context_(nullptr),
	socket_(new Socket(socket)),
	channel_(new Channel(loop_, socket)),
	client_address_(client),
	server_address_(server),
	high_water_mark_(kInitialHighWaterMark)
{
	LOG_DEBUG("TcpConnection::ctor[%s] at %p fd=%d", name_.c_str(), this, socket);

	channel_->set_event_callback(Channel::READ_CALLBACK,
	                             bind(&TcpConnection::HandleRead, this, _1));
	channel_->set_event_callback(Channel::WRITE_CALLBACK,
	                             bind(&TcpConnection::HandleWrite, this));
	channel_->set_event_callback(Channel::CLOSE_CALLBACK,
	                             bind(&TcpConnection::HandleClose, this));
	channel_->set_event_callback(Channel::ERROR_CALLBACK,
	                             bind(&TcpConnection::HandleError, this));
	socket_->SetTcpKeepAlive(true);
}
void TcpConnection::HandleRead(const TimeStamp &receive_time)
{
	loop_->AssertInLoopThread();

	int saved_errno = 0;
	int read_byte = input_buffer_.ReadFd(channel_->fd(), saved_errno);
	if(read_byte > 0 && message_callback_)
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
		LOG_ERROR("TcpConnection::HandleRead()");
		HandleError();
	}
}
void TcpConnection::HandleClose()
{
	loop_->AssertInLoopThread();
	LOG_TRACE("fd = %d, state = %s", channel_->fd(), StateToCString());

	assert(state_ == CONNECTED || // No Shutdown() or ForceClose() before.
	       state_ == DISCONNECTING);
	set_state(DISCONNECTED);
	channel_->set_requested_event(Channel::NONE_EVENT);
	TcpConnectionPtr guard(shared_from_this());
	connection_callback_(guard);
	close_callback_(guard);
}
void TcpConnection::HandleError()
{
	int error = nso::GetSocketError(channel_->fd());
	LOG_INFO("TcpConnection::HandleError [%s] - SO_ERROR = %d %s",
	         name_.c_str(), error, ThreadSafeStrError(error));
}

void TcpConnection::HandleWrite()
{
	loop_->AssertInLoopThread();

	if(channel_->IsRequested(Channel::WRITE_EVENT) == true)
	{
		int write_byte = static_cast<int>(::write(channel_->fd(),
		                                  output_buffer_.ReadableBegin(),
		                                  output_buffer_.ReadableByte()));
		if(write_byte > 0)
		{
			output_buffer_.Retrieve(write_byte);
			if(output_buffer_.ReadableByte() == 0)
			{
				channel_->set_requested_event(Channel::NOT_WRITE);
				if(write_complete_callback_)
				{
					loop_->QueueInLoop(bind(write_complete_callback_, shared_from_this()));
				}
				if(state_ == DISCONNECTING)
				{
					ShutdownInLoop();
				}
			}
		}
		else
		{
			LOG_ERROR("TcpConnection::HandleWrite");
		}
	}
	else
	{
		LOG_TRACE("Connection fd = %d is down, no more writing.", channel_->fd());
	}
}
void TcpConnection::ShutdownInLoop()
{
	loop_->AssertInLoopThread();

	if(channel_->IsRequested(Channel::WRITE_EVENT) == false)
	{
		socket_->ShutdownOnWrite();
	}
}

TcpConnection::~TcpConnection()
{
	LOG_DEBUG("TcpConnection::dtor[%s] at %p fd = %d state = %s",
	          name_.c_str(),
	          this,
	          channel_->fd(),
	          StateToCString());
	assert(state_ == DISCONNECTED);
}
const char *TcpConnection::StateToCString() const
{
	switch(state_)
	{
	case DISCONNECTED:
		return "Disconnected";
	case CONNECTING:
		return "Connecting";
	case CONNECTED:
		return "Connected";
	case DISCONNECTING:
		return "Disconnecting";
	default:
		return "Unknown state";
	}
}

void TcpConnection::SetTcpNoDelay(bool on)
{
	socket_->SetTcpNoDelay(on);
}

void TcpConnection::ConnectEstablished()
{
	loop_->AssertInLoopThread();
	assert(state_ == CONNECTING);
	set_state(CONNECTED);
	channel_->set_tie(shared_from_this());
	channel_->set_requested_event(Channel::READ_EVENT);
	connection_callback_(shared_from_this());
}

void TcpConnection::Send(const void *data, int length)
{
	if(state_ == CONNECTED)
	{
		loop_->RunInLoop(bind(&TcpConnection::SendInLoop,
		                      this,
		                      static_cast<const char*>(data),
		                      length));
	}
}
void TcpConnection::Send(const string &message)
{
	if(state_ == CONNECTED)
	{
		// TODO: Use std::move(string&&) to avoid copy.
		loop_->RunInLoop(bind(&TcpConnection::SendInLoop,
		                      this,
		                      message.data(),
		                      static_cast<int>(message.size())));
	}
}
void TcpConnection::Send(Buffer *buffer)
{
	if(state_ == CONNECTED)
	{
		// TODO: Use Buffer::Swap() to avoid copy.
		loop_->RunInLoop(bind(&TcpConnection::SendInLoop,
		                      this,
		                      buffer->ReadableBegin(),
		                      buffer->ReadableByte()));
		buffer->RetrieveAll();
	}
}
void TcpConnection::SendInLoop(const char *data, int length)
{
	loop_->AssertInLoopThread();

	if(state_ == DISCONNECTED)
	{
		LOG_WARN("Disconnected, Give up writing.");
		return;
	}
	int write_byte = 0, remaining_byte = length;
	bool has_error = false;
	if(output_buffer_.ReadableByte() == 0)
	{
		write_byte = static_cast<int>(::write(channel_->fd(), data, length));
		if(write_byte > 0)
		{
			remaining_byte -= write_byte;
			if(remaining_byte == 0 && write_complete_callback_)
			{
				loop_->QueueInLoop(bind(write_complete_callback_, shared_from_this()));
			}
		}
		else
		{
			write_byte = 0; // Used in `output_buffer_.Append()`
			// socket is nonblocking and the write would block.
			if(errno != EWOULDBLOCK && errno != EAGAIN)
			{
				LOG_ERROR("TcpConnection::SendInLoop");
				if(errno == EPIPE || errno == ECONNRESET)
				{
					has_error = true;
				}
			}
		}
	}

	if(has_error == false && remaining_byte > 0)
	{
		int buffered_byte = output_buffer_.ReadableByte();
		if(high_water_mark_callback_ &&
		        buffered_byte < high_water_mark_ &&
		        buffered_byte + remaining_byte >= high_water_mark_)
		{
			loop_->QueueInLoop(bind(high_water_mark_callback_,
			                        shared_from_this(),
			                        buffered_byte + remaining_byte));
		}
		output_buffer_.Append(data + write_byte, remaining_byte);
		channel_->set_requested_event(Channel::WRITE_EVENT);
	}
	// if has_error = true: discard unsent data.
}

void TcpConnection::Shutdown()
{
	if(state_ == CONNECTED)
	{
		set_state(DISCONNECTING);
		loop_->RunInLoop(bind(&TcpConnection::ShutdownInLoop, shared_from_this()));
	}
}

void TcpConnection::ForceClose()
{
	if(state_ == CONNECTED || state_ == DISCONNECTING)
	{
		set_state(DISCONNECTING);
		loop_->QueueInLoop(bind(&TcpConnection::ForceCloseInLoop,
		                        shared_from_this()));
	}
}
void TcpConnection::ForceCloseInLoop()
{
	loop_->AssertInLoopThread();
	assert(state_ == DISCONNECTING);
	HandleClose();
}

// The last member function called by TcpConnection object before destructing.
void TcpConnection::ConnectDestroyed()
{
	loop_->AssertInLoopThread();

	if(state_ == CONNECTED)
	{
		// Repeated as in HandleClose(): we may call ConnectDestroyed() directly.
		set_state(DISCONNECTED);
		channel_->set_requested_event(Channel::NONE_EVENT);
		connection_callback_(shared_from_this());
	}
	channel_->RemoveChannel();
}
