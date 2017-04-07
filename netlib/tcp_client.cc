#include <netlib/tcp_client.h>
#include <netlib/event_loop.h>
#include <netlib/connector.h>
#include <netlib/tcp_connection.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/socket_operation.h>

using std::bind;
using std::string;
using std::placeholders::_1;
using netlib::TcpClient;

TcpClient::TcpClient(EventLoop *event_loop,
                     const SocketAddress &server_address,
                     const string &name):
	loop_(CHECK_NOT_NULL(event_loop)),
	connector_(new Connector(loop_, server_address)),
	name_(name),
	retry_(false),
	connect_(true),
	next_connection_id_(0),
	connection_callback_(DefaultConnectionCallback),
	message_callback_(DefaultMessageCallback)
{
	connector_->set_new_connection_callback(
	    bind(&TcpClient::HandleNewConnection, this, _1));
	LOG_INFO("TcpClient::TcpClient[%s] - connector %p",
	         name_.c_str(),
	         connector_.get());
}
void TcpClient::HandleNewConnection(int socket_fd)
{
	loop_->AssertInLoopThread();
	SocketAddress server_address(nso::GetPeerAddress(socket_fd));
	char buffer[32];
	::snprintf(buffer, sizeof buffer, ":%s#%d",
	           server_address.ToIpPortString().c_str(),
	           ++next_connection_id_);
	string connection_name = name_ + buffer;
	LOG_INFO("TcpClient::HandleNewConnection [%s] - new connection [%s] to %s",
	         name_.c_str(), connection_name.c_str(), server_address.ToIpPortString().c_str());

	SocketAddress client_address(nso::GetLocalAddress(socket_fd));
	// FIXME poll with zero timeout to double confirm the new connection.
	// FIXME use make_shared if necessary.
	TcpConnectionPtr connection(new TcpConnection(loop_,
	                            connection_name,
	                            socket_fd,
	                            client_address,
	                            server_address));

	connection->set_connection_callback(connection_callback_);
	connection->set_message_callback(message_callback_);
	connection->set_write_complete_callback(write_complete_callback_);
	// FIXME: unsafe
	connection->set_close_callback(bind(&TcpClient::RemoveConnection, this, _1));
	{
		MutexLockGuard lock(mutex_);
		connection_ = connection;
	}
	connection->ConnectEstablished();
}
void TcpClient::RemoveConnection(const TcpConnectionPtr &connection)
{
	loop_->AssertInLoopThread();
	assert(loop_ == connection->loop());
	{
		MutexLockGuard lock(mutex_);
		assert(connection_ == connection);
		connection_.reset();
	}
	loop_->QueueInLoop(bind(&TcpConnection::ConnectDestroyed, connection));
	if(retry_ == true && connect_ == true)
	{
		LOG_INFO("TcpClient::connect[%s] - Reconnecting to %s",
		         name_.c_str(),
		         connector_->server_address().ToIpPortString().c_str());
		connector_->Restart();
	}
}

TcpClient::~TcpClient()
{
	LOG_INFO("TcpClient::~TcpClient[%s] - connector %p",
	         name_.c_str(),
	         connector_.get());
	bool is_unique = false;
	TcpConnectionPtr connection;
	{
		MutexLockGuard lock(mutex_);
		is_unique = connection_.unique();
		connection = connection_;
	}
	if(connection)
	{
		assert(loop_ == connection->loop());
		// TODO: Already set_close_callback in HandleNewConnection(), why set it again?
		// FIXME: not 100% safe, if we are in different thread
		// CloseCallback cb = bind(&detail::removeConnection, loop_, _1);
		// loop_->runInLoop(bind(&TcpConnection::setCloseCallback, connection, cb));
		if(is_unique == true)
		{
			connection->ForceClose();
		}
	}
	else
	{
		connector_->Stop();
		// FIXME: hack.
		// loop_->RunAfter(1, bind(&detail::removeConnector, connector_));
	}
}

void TcpClient::Connect()
{
	// FIXME: check state
	LOG_INFO("TcpClient::connect[%s] - connecting to %s",
	         name_.c_str(),
	         connector_->server_address().ToIpPortString().c_str());
	connect_ = true;
	connector_->Start();
}

void TcpClient::Disconnect()
{
	connect_ = false;
	{
		MutexLockGuard lock(mutex_);
		if(connection_)
		{
			connection_->Shutdown();
		}
	}
}

void TcpClient::Stop()
{
	connect_ = false;
	connector_->Stop();
}
