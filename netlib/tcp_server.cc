#include <netlib/tcp_server.h>

#include <netlib/event_loop.h>
#include <netlib/event_loop_thread_pool.h>
#include <netlib/socket_address.h>
#include <netlib/logging.h> // CHECK_NOT_NULL
#include <netlib/acceptor.h>
#include <netlib/logging.h>
#include <netlib/socket_operation.h>
#include <netlib/tcp_connection.h>

#include <stdio.h> // snprintf()

namespace nso = netlib::socket_operation;
using std::bind;
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;
using netlib::TcpServer;

TcpServer::TcpServer(EventLoop *loop, const SocketAddress &listen_address):
	loop_(CHECK_NOT_NULL(loop)),
	name_(listen_address.ToHostPort()),
	acceptor_(new Acceptor(loop_, listen_address)),
	thread_pool_(new EventLoopThreadPool(loop_)),
	started_(false),
	next_connection_id_(1)
{
	acceptor_->set_new_connection_callback(
	    bind(&TcpServer::HandleNewConnection, this, _1, _2));
	LOG_INFO("TcpServer ctor.");
}

TcpServer::~TcpServer() {}

// Start the server if it's not listening.
void TcpServer::Start()
{
	if(started_ == false)
	{
		started_ = true;
		(*thread_pool_).Start();
	}
	if(acceptor_->listening() == false)
	{
		// u.get() returns the pointer in u. Warn: the object to which the returned pointer
		// points will disappear when the smart pointer deletes it.
		loop_->RunInLoop(
		    bind(&Acceptor::Listen, acceptor_.get()));
	}
}

void TcpServer::SetThreadNumber(int thread_number)
{
	if(thread_number >= 0)
	{
		LOG_INFO("Before thread_pool_.set_t_n");
		(*thread_pool_).set_thread_number(thread_number);
		LOG_INFO("After thread_pool_.set_t_n");
	}
}

// Create the TcpConnection object, add it to the connection map,
// set callbacks, and call `connection_object->ConnectionEstablished()`, which
// calls the user's ConnectionCallback.
void TcpServer::HandleNewConnection(int socket_fd, const SocketAddress &peer_address)
{
	loop_->AssertInLoopThread();

	char buffer[32];
	::snprintf(buffer, 32, "#%d", next_connection_id_);
	++next_connection_id_;
	string connection_name = name_ + buffer;
	LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
	         name_.c_str(), connection_name.c_str(), peer_address.ToHostPort().c_str());

	SocketAddress local_address(nso::GetLocalAddress(socket_fd));
	// FIXME poll with zero timeout to double confirm the new connection
	EventLoop *io_loop = thread_pool_->GetNextLoop();
	// TODO: use make_shared() to avoid the use of new.
	TcpConnectionPtr connection_ptr(
	    new TcpConnection(io_loop, connection_name, socket_fd, local_address, peer_address));
	connection_map_[connection_name] = connection_ptr;

	connection_ptr->set_connection_callback(connection_callback_);
	connection_ptr->set_message_callback(message_callback_);
	connection_ptr->set_write_complete_callback(write_complete_callback_);
	// Register to TcpConnection close_callback_ in order to know the connection is down.
	// Called in TcpConnection::HandleClose().
	// FIXME: unsafe.
	connection_ptr->set_close_callback(bind(&TcpServer::RemoveConnection, this, _1));
	io_loop->RunInLoop(bind(&TcpConnection::ConnectEstablished, connection_ptr));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &connection)
{
	// FIXME: unsafe.
	loop_->RunInLoop(bind(&TcpServer::RemoveConnectionInLoop, this, connection));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &connection)
{
	loop_->AssertInLoopThread();
	assert(connection_map_.erase(connection->name()) == 1);
	EventLoop *io_loop = connection->loop();
	// TODO: Must use QueueInLoop(), see 7.15.3.
	// bind() will make this TcpConnection object live to the time when calling
	// ConnectDestroyed(). See 1.10.
	io_loop->QueueInLoop(bind(&TcpConnection::ConnectDestroyed, connection));
}
