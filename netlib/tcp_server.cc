#include <netlib/tcp_server.h>

#include <netlib/event_loop.h>
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
	started_(false),
	next_connection_id_(1){}/*
{
	acceptor_->set_new_connection_callback(
	    bind(&TcpServer::NewConnection, this, _1, _2));
}*/

TcpServer::~TcpServer() {}

void TcpServer::Start()
{
	if(started_ == false)
	{
		started_ = true;
	}
	if(acceptor_->listening() == false)
	{
		// u.get() returns the pointer in u. Warn: the object to which the returned pointer
		// points will disappear when the smart pointer deletes it.
		loop_->RunInLoop(
		    bind(&Acceptor::Listen, acceptor_.get()));
	}
}

// Create the TcpConnection object, add it to the connection map,
// set callbacks, and call `connection_object->ConnectionEstablished()`, which
// calls the user's ConnectionCallback.
void TcpServer::NewConnection(int socket_fd, const SocketAddress &peer_address)
{
	loop_->AssertInLoopThread();

	char buffer[32];
	::snprintf(buffer, 32, "#%d", next_connection_id_);
	++next_connection_id_;
	string connection_name = name_ + buffer;
	LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
	         name_.c_str(), connection_name.c_str(), peer_address.ToHostPort().c_str());

	SocketAddress local_address(nso::GetLocalAddress(socket_fd));
	// TODO: use make_shared() to avoid the use of new.
	TcpConnectionPtr connection_ptr(
	    new TcpConnection(loop_, connection_name, socket_fd, local_address, peer_address));
	connection_map_[connection_name] = connection_ptr;

	connection_ptr->set_connection_callback(connection_callback_);
	connection_ptr->set_message_callback(message_callback_);
	connection_ptr->ConnectEstablished();
}
