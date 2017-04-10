#include <netlib/connector.h>

#include <unistd.h> // close()

#include <netlib/channel.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket_operation.h>

using std::bind;
using netlib::Connector;

const double Connector::kMaxRetryDelaySecond = 30.0;
const double Connector::kInitialRetryDelaySecond = 0.5;

Connector::Connector(EventLoop *owner_loop, const SocketAddress &address):
	owner_loop_(owner_loop),
	server_address_(address),
	connectable_(false),
	state_(DISCONNECTED),
	retry_delay_second_(kInitialRetryDelaySecond)
{
	LOG_DEBUG("ctor[%p]", this);
}

Connector::~Connector()
{
	LOG_DEBUG("dtor[%p]", this);
	assert(!channel_);
}

void Connector::Start()
{
	connectable_ = true;
	owner_loop_->RunInLoop(bind(&Connector::StartInLoop, this));
}
void Connector::StartInLoop()
{
	owner_loop_->AssertInLoopThread();
	assert(state_ == DISCONNECTED);

	if(connectable_ == true)
	{
		Connect();
	}
	else
	{
		LOG_DEBUG("Don't connect.");
	}
}
void Connector::Connect()
{
	int socket = nso::CreateNonblockingTcpSocket(server_address_.socket_family());
	int ret = ::connect(socket,
	                    server_address_.socket_address(),
	                    sizeof(struct sockaddr_in));
	int saved_errno = (ret == 0 ? 0 : errno);
	switch(saved_errno)
	{
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		Connecting(socket);
		break;
	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		Retry(socket);
		break;
	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_ERROR("Connect error in Connector::startInLoop %d", saved_errno);
		::close(socket);
		break;
	default:
		LOG_ERROR("Unexpected error in Connector::startInLoop %d", saved_errno);
		::close(socket);
	}
}
void Connector::Connecting(int socket)
{
	set_state(CONNECTING);
	assert(!channel_);
	channel_.reset(new Channel(owner_loop_, socket));
	channel_->set_tie(shared_from_this());
	channel_->set_event_callback(Channel::WRITE_CALLBACK,
	                             bind(&Connector::HandleWrite, this));
	channel_->set_event_callback(Channel::ERROR_CALLBACK,
	                             bind(&Connector::HandleError, this));
	channel_->set_requested_event(Channel::WRITE_EVENT);
}
void Connector::HandleWrite()
{
	LOG_TRACE("Connector::handleWrite %d", state_);
	if(state_ == CONNECTING)
	{
		int socket = RemoveAndResetChannel();
		int error = nso::GetSocketError(socket);
		if(error != 0)
		{
			LOG_WARN("Connector::handleWrite - SO_ERROR = %d %s",
			         error, ThreadSafeStrError(error));
			Retry(socket);
		}
		else if(IsSelfConnect(socket) == true)
		{
			LOG_WARN("Connector::handleWrite - Self connect");
			Retry(socket);
		}
		else
		{
			set_state(CONNECTED);
			if(connectable_ == true)
			{
				new_connection_callback_(socket);
			}
			else
			{
				::close(socket);
			}
		}
	}
}
int Connector::RemoveAndResetChannel()
{
	channel_->set_requested_event(Channel::NONE_EVENT);
	channel_->RemoveChannel();
	int socket = channel_->fd();
	// Can't reset channel_ here because we are in Channel::HandleEvent().
	owner_loop_->QueueInLoop(bind(&Connector::ResetChannel, this));
	return socket;
}
void Connector::ResetChannel()
{
	channel_.reset();
}
void Connector::Retry(int socket)
{
	::close(socket);
	set_state(DISCONNECTED);
	if(connectable_ == true)
	{
		LOG_INFO("Connector::retry - Retry connecting to %s in %f seconds",
		         server_address_.ToIpPortString().c_str(),
		         retry_delay_second_);
		owner_loop_->RunAfter(bind(&Connector::StartInLoop, shared_from_this()),
		                      retry_delay_second_);
		retry_delay_second_ *= 2;
		if(retry_delay_second_ > kMaxRetryDelaySecond)
		{
			retry_delay_second_ = kMaxRetryDelaySecond;
		}
	}
	else
	{
		LOG_DEBUG("Don't connect.");
	}
}
bool Connector::IsSelfConnect(int socket)
{
	struct sockaddr_in local_address = nso::GetLocalAddress(socket);
	struct sockaddr_in peer_address = nso::GetPeerAddress(socket);
	return local_address.sin_port == peer_address.sin_port &&
	       local_address.sin_addr.s_addr == peer_address.sin_addr.s_addr;
}
void Connector::HandleError()
{
	LOG_ERROR("Connector::handleError state = %d", state_);
	if(state_ == CONNECTING)
	{
		int socket = RemoveAndResetChannel();
		int error = nso::GetSocketError(socket);
		LOG_TRACE("SO_ERROR = %d %s", error, ThreadSafeStrError(error));
		Retry(socket);
	}
}

void Connector::Stop()
{
	connectable_ = false;
	owner_loop_->QueueInLoop(bind(&Connector::StopInLoop, this));
}
void Connector::StopInLoop()
{
	owner_loop_->AssertInLoopThread();
	if(state_ == CONNECTING)
	{
		set_state(DISCONNECTED);
		Retry(RemoveAndResetChannel());
	}
}

void Connector::Restart()
{
	owner_loop_->AssertInLoopThread();
	connectable_ = true;
	set_state(DISCONNECTED);
	retry_delay_second_ = kInitialRetryDelaySecond;
	StartInLoop();
}
