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

TcpServer::TcpServer(EventLoop *main_loop,
                     const SocketAddress &server_address,
                     const string &name,
                     int loop_number):
	main_loop_(CHECK_NOT_NULL(main_loop)),
	server_ip_port_(server_address.ToIpPortString()),
	server_name_(name),
	acceptor_(new Acceptor(main_loop_, server_address)),
	loop_pool_(new EventLoopThreadPool(main_loop_, loop_number)),
	started_(false),
	next_connection_id_(0),
	connection_callback_(DefaultConnectionCallback),
	message_callback_(DefaultMessageCallback)
{
	acceptor_->set_new_connection_callback(
	    bind(&TcpServer::HandleNewConnection, this, _1, _2));
}
void TcpServer::HandleNewConnection(int connected_socket,
                                    const SocketAddress &client_address)
{
	main_loop_->AssertInLoopThread();

	char buf[32];
	snprintf(buf, sizeof buf, "-%s#%d", server_ip_port_.c_str(), ++next_connection_id_);
	string connection_name = server_name_ + buf;
	LOG_INFO("TcpServer::HandleNewConnection [%s] - new connection [%s] from %s",
	         server_name_.c_str(),
	         connection_name.c_str(),
	         client_address.ToIpPortString().c_str());

	SocketAddress server_address(nso::GetLocalAddress(connected_socket));
	EventLoop *sub_loop = loop_pool_->GetNextLoop();
	// FIXME poll with zero timeout to double confirm the new connection.
	// FIXME use make_shared if necessary.
	TcpConnectionPtr connection_ptr(new TcpConnection(sub_loop,
	                                connection_name,
	                                connected_socket,
	                                client_address,
	                                server_address));
	connection_ptr->set_connection_callback(connection_callback_);
	connection_ptr->set_message_callback(message_callback_);
	connection_ptr->set_write_complete_callback(write_complete_callback_);
	connection_ptr->set_close_callback(bind(&TcpServer::RemoveConnection, this, _1));

	connection_name_ptr_map_[connection_name] = connection_ptr;
	sub_loop->RunInLoop(bind(&TcpConnection::ConnectEstablished, connection_ptr));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &connection_ptr)
{
	main_loop_->RunInLoop(bind(&TcpServer::RemoveConnectionInLoop,
	                           this,
	                           connection_ptr));
}
void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &connection_ptr)
{
	main_loop_->AssertInLoopThread();
	LOG_INFO("TcpServer::RemoveConnection [%s] - connection %s",
	         server_name_.c_str(),
	         connection_ptr->name().c_str());

	assert(connection_name_ptr_map_.erase(connection_ptr->name()) == 1);
	connection_ptr->loop()->RunInLoop(
	    bind(&TcpConnection::ConnectDestroyed, connection_ptr));
}

TcpServer::~TcpServer()
{
	main_loop_->AssertInLoopThread();
	LOG_TRACE("TcpServer::Dtor [%s] destructing.", server_name_.c_str());

	for(ConnectionNamePtrMap::iterator it = connection_name_ptr_map_.begin();
	        it != connection_name_ptr_map_.end();
	        ++it)
	{
		it->second->loop()->RunInLoop(
		    bind(&TcpConnection::ConnectDestroyed, it->second));
		it->second.reset();
	}
}

void TcpServer::Start()
{
	if(started_ == false)
	{
		started_ = true;
		loop_pool_->Start();
		main_loop_->RunInLoop(bind(&Acceptor::Listen, acceptor_));
	}
}
