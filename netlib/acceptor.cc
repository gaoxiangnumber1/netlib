#include <assert.h> // assert()
#include <fcntl.h> // open()
#include <sys/socket.h> // accept()
#include <sys/stat.h> // open()
#include <sys/types.h> // open()
#include <unistd.h> // close()

#include <netlib/acceptor.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket_address.h>
#include <netlib/socket_operation.h> // CreateNonblockingTcpSocket()

using std::bind;
using netlib::Acceptor;

Acceptor::Acceptor(EventLoop *owner_loop, const SocketAddress &server_address):
	owner_loop_(owner_loop),
	server_socket_(nso::CreateNonblockingTcpSocket(server_address.socket_family())),
	server_channel_(owner_loop_, server_socket_.socket()),
	listening_(false),
	idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	assert(idle_fd_ >= 0);
	server_socket_.SetReuseAddress(true);
	server_socket_.SetReusePort(true);
	server_socket_.Bind(server_address);
	server_channel_.set_event_callback(Channel::READ_CALLBACK,
	                                   bind(&Acceptor::HandleRead, this));
}
void Acceptor::HandleRead()
{
	owner_loop_->AssertInLoopThread();

	SocketAddress client_address;
	int connected_socket = server_socket_.Accept(client_address);
	if(connected_socket >= 0)
	{
		if(new_connection_callback_)
		{
			new_connection_callback_(connected_socket, client_address);
		}
		else
		{
			::close(connected_socket);
		}
	}
	else
	{
		LOG_ERROR("server_socket_.Accept()");
		if(errno == EMFILE)
		{
			::close(idle_fd_);
			idle_fd_ = ::accept(server_socket_.socket(), NULL, NULL);
			::close(idle_fd_);
			idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
	}
}

Acceptor::~Acceptor()
{
	server_channel_.set_requested_event(Channel::NONE_EVENT);
	server_channel_.RemoveChannel();
	::close(idle_fd_);
}

void Acceptor::Listen()
{
	owner_loop_->AssertInLoopThread();

	server_channel_.set_requested_event(Channel::READ_EVENT);
	server_socket_.Listen();
	listening_ = true;
}
