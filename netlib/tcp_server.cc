#include <netlib/acceptor.h>
#include <netlib/event_loop.h>
#include <netlib/event_loop_thread_pool.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/socket_operation.h>
#include <netlib/tcp_connection.h>
#include <netlib/tcp_server.h>

using std::string;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using netlib::TcpServer;

// Always accept new connection in the loop thread.
// thread_number =
//	(1) 0:	All I/O in the loop thread, no thread will be created. This is the default value.
//	(2) 1:	All I/O in another thread.
//	(3) N:	Create a thread pool of N threads, new connections are assigned
//				on a round-robin basis.
TcpServer::TcpServer(EventLoop *loop,
                     const SocketAddress &listen_address,
                     const string &name,
                     int thread_number,
                     const InitialTask &initial_task,
                     bool is_reuse_port):
	loop_(CHECK_NOT_NULL(loop)),
	ip_port_(listen_address.ToIpPortString()),
	name_(name),
	acceptor_(new Acceptor(loop_, listen_address, is_reuse_port)),
	thread_pool_(new EventLoopThreadPool(loop_, initial_task, thread_number)),
	started_(false),
	next_connection_id_(0),
	connection_callback_(DefaultConnectionCallback),
	message_callback_(DefaultMessageCallback)
{
	acceptor_->set_new_connection_callback(
	    bind(&TcpServer::HandleNewConnection, this, _1, _2));
}
// Create the TcpConnection object, add it to the connection map,
// set callbacks, and call `connection_object->ConnectionEstablished()`, which
// calls the user's ConnectionCallback.
void TcpServer::HandleNewConnection(int socket_fd, const SocketAddress &peer_address)
{
	loop_->AssertInLoopThread();

	char buffer[32] = "";
	::snprintf(buffer, sizeof buffer, "-%s#%d", ip_port_.c_str(), ++next_connection_id_);
	string connection_name = name_ + buffer;
	LOG_INFO("TcpServer::HandleNewConnection [%s] - new connection [%s] from %s",
	         name_.c_str(), connection_name.c_str(), peer_address.ToIpPortString().c_str());

	SocketAddress local_address(nso::GetLocalAddress(socket_fd));
	// FIXME poll with zero timeout to double confirm the new connection
	EventLoop *io_loop = thread_pool_->GetNextLoop();
	// TODO: use make_shared() to avoid the use of new.
	TcpConnectionPtr connection(new TcpConnection(io_loop,
	                            connection_name,
	                            socket_fd,
	                            local_address,
	                            peer_address));
	connection_map_[connection_name] = connection;

	connection->set_connection_callback(connection_callback_);
	connection->set_message_callback(message_callback_);
	connection->set_write_complete_callback(write_complete_callback_);
	// Register to TcpConnection close_callback_ in order to know
	// the connection is down. Called in TcpConnection::HandleClose().
	// FIXME: unsafe.
	connection->set_close_callback(bind(&TcpServer::RemoveConnection, this, _1));
	io_loop->RunInLoop(bind(&TcpConnection::ConnectEstablished, connection));
}
void TcpServer::RemoveConnection(const TcpConnectionPtr &connection)
{
	// FIXME: unsafe.
	loop_->RunInLoop(bind(&TcpServer::RemoveConnectionInLoop, this, connection));
}
void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &connection)
{
	loop_->AssertInLoopThread();
	LOG_INFO("TcpServer::RemoveConnectionInLoop [%s] - connection %s",
	         name_.c_str(),
	         connection->name().c_str());
	assert(connection_map_.erase(connection->name()) == 1);
	// TODO: Must use QueueInLoop(), see 7.15.3.
	// bind() will make this TcpConnection object live to the time
	// when calling ConnectDestroyed(). See 1.10.
	connection->loop()->QueueInLoop(
	    bind(&TcpConnection::ConnectDestroyed, connection));
}

TcpServer::~TcpServer()
{
	loop_->AssertInLoopThread();
	LOG_TRACE("TcpServer::Dtor [%s] destructing.", name_.c_str());
	for(ConnectionMap::iterator it = connection_map_.begin();
	        it != connection_map_.end();
	        ++it)
	{
		TcpConnectionPtr connection = it->second;
		it->second.reset();
		connection->loop()->RunInLoop(
		    bind(&TcpConnection::ConnectDestroyed, connection));
		connection.reset();
	}
}

// Start the server if it's not listening.
void TcpServer::Start()
{
	if(started_ == false)
	{
		started_ = true;
		thread_pool_->Start();
		assert(acceptor_->listening() == false);
		// u.get() returns the pointer in u. Warn: the object to which the returned pointer
		// points will disappear when the smart pointer deletes it.
		loop_->RunInLoop(bind(&Acceptor::Listen, acceptor_.get()));
	}
}
