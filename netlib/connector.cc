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

Connector::Connector(EventLoop *loop, const SocketAddress &address):
	loop_(loop),
	server_address_(address),
	connect_(false),
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
	connect_ = true;
	loop_->RunInLoop(bind(&Connector::StartInLoop, this));
}
void Connector::StartInLoop()
{
	loop_->AssertInLoopThread();
	assert(state_ == DISCONNECTED);
	if(connect_ == true)
	{
		Connect();
	}
	else
	{
		LOG_DEBUG("Don't connect.");
	}
}
// int connect(int socket_fd, const struct sockaddr *addr, socklen_t addrlen);
// connect() connects the socket `socket_fd` to the address specified by `addr`.
// `addrlen` specifies the size of `addr`.
// If the socket is of type SOCK_STREAM, this call attempts to make
// a connection to the socket that is bound to the address specified by addr.
// Connection-based protocol sockets may successfully connect() only once.
// Return 0 on success, -1 on error and errno is set.
void Connector::Connect()
{
	int socket_fd = nso::CreateNonblockingSocket(server_address_.socket_family());
	int ret = ::connect(socket_fd,
	                    server_address_.socket_address(),
	                    static_cast<socklen_t>(sizeof(struct sockaddr_in)));
	int saved_errno = (ret == 0) ? 0 : errno;
	switch(saved_errno)
	{
	case 0:
	// The socket is nonblocking and the connection cannot be completed immediately.
	// It is possible to select(2)/poll(2) for completion by selecting the socket for writing.
	// After select(2) indicates writ-ability, use getsockopt(2) to read the SO_ERROR
	// option at level SOL_SOCKET to determine whether connect() completed
	// successfully(SO_ERROR is zero) or unsuccessfully(SO_ERROR is one of the
	// usual error codes, explaining the reason for the failure).
	case EINPROGRESS:
	// connect() is interrupted by a signal that was caught.
	case EINTR:
	// The socket is already connected.
	case EISCONN:
		Connecting(socket_fd);
		break;

	// No more free local ports or insufficient entries in the routing cache.
	// For AF_INET: see the description of /proc/sys/net/ipv4/ip_local_port_range ip(7)
	// for information on how to increase the number of local ports.
	case EAGAIN:
	// Local address is already in use.
	case EADDRINUSE:
	// Cannot assign requested address.
	case EADDRNOTAVAIL:
	// No-one listening on the remote address.
	case ECONNREFUSED:
	// Network is unreachable.
	case ENETUNREACH:
		Retry(socket_fd);
		break;

	// EACCES, EPERM
	// The user tried to connect to a broadcast address without having the socket
	// broadcast flag enabled or the connection request failed because of a local
	// firewall rule.
	case EACCES:
	case EPERM:
	// The passed address didn't have the correct address family in its sa_family field.
	case EAFNOSUPPORT:
	// The socket is nonblocking and a previous connection attempt
	// has not yet been completed.
	case EALREADY:
	// The file descriptor is not a valid index in the descriptor table.
	case EBADF:
	// The socket structure address is outside the user's address space.
	case EFAULT:
	// The file descriptor is not associated with a socket.
	case ENOTSOCK:
		LOG_ERROR("Connect error in Connector::startInLoop %d", saved_errno);
		::close(socket_fd);
		break;

	default:
		// ETIMEDOUT
		// Timeout while attempting connection. The server may be too busy to accept new
		// connections. For IP sockets the timeout is very long when syncookies are enabled
		// on the server.
		LOG_ERROR("Unexpected error in Connector::startInLoop %d", saved_errno);
		::close(socket_fd);
	}
}
void Connector::Connecting(int socket_fd)
{
	set_state(CONNECTING);
	assert(!channel_);
	channel_.reset(new Channel(loop_, socket_fd));
	// FIXME: unsafe
	channel_->set_event_callback(Channel::WRITE_CALLBACK,
	                             bind(&Connector::HandleWrite, this));
	// FIXME: unsafe
	channel_->set_event_callback(Channel::ERROR_CALLBACK,
	                             bind(&Connector::HandleError, this));
	// `channel_->set_tie(shared_from_this());` is not working,
	// because channel_ is not managed by shared_ptr.
	channel_->set_requested_event(Channel::WRITE_EVENT);
}
void Connector::HandleWrite()
{
	LOG_TRACE("Connector::handleWrite %d", state_);
	if(state_ == CONNECTING)
	{
		int socket_fd = RemoveAndResetChannel();
		int error = nso::GetSocketError(socket_fd);
		if(error != 0)
		{
			LOG_WARN("Connector::handleWrite - SO_ERROR = %d %s",
			         error, ThreadSafeStrError(error));
			Retry(socket_fd);
		}
		else if(IsSelfConnect(socket_fd) == true)
		{
			LOG_WARN("Connector::handleWrite - Self connect");
			Retry(socket_fd);
		}
		else
		{
			set_state(CONNECTED);
			if(connect_ == true)
			{
				new_connection_callback_(socket_fd);
			}
			else
			{
				::close(socket_fd);
			}
		}
	}
	else
	{
		// TODO: What happened?
		assert(state_ == DISCONNECTED);
	}
}
int Connector::RemoveAndResetChannel()
{
	channel_->set_requested_event(Channel::NONE_EVENT);
	channel_->RemoveChannel();
	int socket_fd = channel_->fd();
	// Can't reset channel_ here because we are in Channel::HandleEvent().
	loop_->QueueInLoop(bind(&Connector::ResetChannel, this)); // FIXME: unsafe.
	return socket_fd;
}
void Connector::ResetChannel()
{
	channel_.reset();
}
void Connector::Retry(int socket_fd)
{
	::close(socket_fd);
	set_state(DISCONNECTED);
	if(connect_ == true)
	{
		LOG_INFO("Connector::retry - Retry connecting to %s in %f seconds",
		         server_address_.ToIpPortString().c_str(),
		         retry_delay_second_);
		loop_->RunAfter(bind(&Connector::StartInLoop, shared_from_this()),
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
bool Connector::IsSelfConnect(int socket_fd)
{
	struct sockaddr_in local_address = nso::GetLocalAddress(socket_fd);
	struct sockaddr_in peer_address = nso::GetPeerAddress(socket_fd);
	return local_address.sin_port == peer_address.sin_port &&
	       local_address.sin_addr.s_addr == peer_address.sin_addr.s_addr;
}
void Connector::HandleError()
{
	LOG_ERROR("Connector::handleError state = %d", state_);
	if(state_ == CONNECTING)
	{
		int socket_fd = RemoveAndResetChannel();
		int error = nso::GetSocketError(socket_fd);
		LOG_TRACE("SO_ERROR = %d %s", error, ThreadSafeStrError(error));
		Retry(socket_fd);
	}
}

void Connector::Restart()
{
	loop_->AssertInLoopThread();
	connect_ = true;
	set_state(DISCONNECTED);
	retry_delay_second_ = kInitialRetryDelaySecond;
	StartInLoop();
}

void Connector::Stop()
{
	connect_ = false;
	loop_->QueueInLoop(bind(&Connector::StopInLoop, this)); // FIXME: unsafe.
	// FIXME: Cancel timer.
}
void Connector::StopInLoop()
{
	loop_->AssertInLoopThread();
	if(state_ == CONNECTING)
	{
		set_state(DISCONNECTED);
		Retry(RemoveAndResetChannel());
	}
}
