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

TcpClient::TcpClient(EventLoop *main_loop,
                     const SocketAddress &server_address,
                     const string &name):
	main_loop_(CHECK_NOT_NULL(main_loop)),
	connector_(new Connector(main_loop_, server_address)),
	name_(name),
	retryable_(false),
	connectable_(true),
	next_connection_id_(0),
	connection_callback_(DefaultConnectionCallback),
	message_callback_(DefaultMessageCallback)
{
	LOG_INFO("TcpClient::TcpClient[%s] - connector %p",
	         name_.c_str(),
	         connector_.get());

	connector_->set_new_connection_callback(
	    bind(&TcpClient::HandleNewConnection, this, _1));
}
void TcpClient::HandleNewConnection(int socket)
{
	main_loop_->AssertInLoopThread();
	SocketAddress server_address(nso::GetPeerAddress(socket));
	char buffer[32];
	::snprintf(buffer, sizeof buffer, ":%s#%d",
	           server_address.ToIpPortString().c_str(),
	           ++next_connection_id_);
	string connection_name = name_ + buffer;
	LOG_INFO("TcpClient::HandleNewConnection [%s] - new connection [%s] to %s",
	         name_.c_str(), connection_name.c_str(), server_address.ToIpPortString().c_str());

	SocketAddress client_address(nso::GetLocalAddress(socket));
	TcpConnectionPtr connection_ptr(new TcpConnection(main_loop_,
	                                connection_name,
	                                socket,
	                                client_address,
	                                server_address));
	connection_ptr->set_connection_callback(connection_callback_);
	connection_ptr->set_message_callback(message_callback_);
	connection_ptr->set_write_complete_callback(write_complete_callback_);
	connection_ptr->set_close_callback(bind(&TcpClient::RemoveConnection, this, _1));

	{
		MutexLockGuard lock(mutex_);
		connection_ptr_ = connection_ptr;
	}
	connection_ptr->ConnectEstablished();
}
void TcpClient::RemoveConnection(const TcpConnectionPtr &connection_ptr)
{
	main_loop_->AssertInLoopThread();
	assert(main_loop_ == connection_ptr->loop());

	{
		MutexLockGuard lock(mutex_);
		assert(connection_ptr_ == connection_ptr);
		connection_ptr_.reset();
	}
	main_loop_->QueueInLoop(bind(&TcpConnection::ConnectDestroyed,
	                             connection_ptr));
	if(retryable_ == true && connectable_ == true)
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
	TcpConnectionPtr connection_ptr;
	{
		MutexLockGuard lock(mutex_);
		is_unique = connection_ptr_.unique();
		connection_ptr = connection_ptr_;
	}
	if(connection_ptr)
	{
		assert(main_loop_ == connection_ptr->loop());
		// TODO: Already set_close_callback in HandleNewConnection(), why set it again?
		// FIXME: not 100% safe, if we are in different thread
		// CloseCallback cb = bind(&detail::removeConnection, main_loop_, _1);
		// main_loop_->runInLoop(bind(&TcpConnection::setCloseCallback, connection_ptr, cb));
		if(is_unique == true)
		{
			connection_ptr->ForceClose();
		}
	}
	else
	{
		connector_->Stop();
		// FIXME: hack.
		// main_loop_->RunAfter(1, bind(&detail::removeConnector, connector_));
	}
}

void TcpClient::Connect()
{
	// FIXME: check state
	LOG_INFO("TcpClient::connect[%s] - connecting to %s",
	         name_.c_str(),
	         connector_->server_address().ToIpPortString().c_str());
	connectable_ = true;
	connector_->Start();
}

void TcpClient::Disconnect()
{
	connectable_ = false;
	{
		MutexLockGuard lock(mutex_);
		if(connection_ptr_)
		{
			connection_ptr_->Shutdown();
		}
	}
}

void TcpClient::Stop()
{
	connectable_ = false;
	connector_->Stop();
}
