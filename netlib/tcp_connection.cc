#include <netlib/tcp_connection.h>

#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket.h>
#include <netlib/channel.h>

using std::string;
using std::bind;
using netlib::TcpConnection;

TcpConnection::TcpConnection(EventLoop *event_loop,
                             const string string_name,
                             int socket,
                             const SocketAddress &local,
                             const SocketAssress &peer):
	loop_(CHECK_NOT_NULL(event_loop)),
	name_(string_name),
	state_(CONNECTING),
	socket_(new Socket(socket)),
	channel_(new Channel(loop_, socket_)),
	local_address_(local),
	peer_address_(peer)
{
	LOG_INFO("TcpConnection::ctor[%s] at %p fd=%d", name_.c_str(), this, socket_);
	channel_->set_read_callback(bind(&TcpConnection::ReadCallback, this));
}

TcpConnection::~TcpConnection()
{
	LOG_INFO("TcpConnection::dtor[%s] at %p fd=%d",
	         name_.c_str(), this, channel_->fd());
}

void TcpConnection::ConnectEstablished()
{
	loop_->AssertInLoopThread();
	assert(state_ == CONNECTING);
	set_state(CONNECTED);
	channel_->set_requested_event_read();
	// std::function<void(const TcpConnection&)>;
	connection_callback_(shared_form_this());
}

void TcpConnection::ReadCallback()
{
	char buffer[65536];
	int readn = static_cast<int>(::read(channel_->fd(), buffer, sizeof(buffer)));
	message_callback_(shared_from_this(), buffer, readn);
	// FIXME: close connection if n == 0
}
