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

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// int open(const char *pathname, int flags);
// O_CLOEXEC: Set the close-on-exec flag for the new fd. Specifying this flag saves
// one fcntl(2) F_SETFD operation to set the FD_CLOEXEC flag. This flag is essential in
// multithreaded program since using a separate fcntl(2) F_SETFD operation to set the
// FD_CLOEXEC flag does not avoid race condition where one thread opens a file
// descriptor at the same time as another thread does a fork(2) plus execve(2).
// Return the new fd, -1 on error and errno is set.
Acceptor::Acceptor(EventLoop *owner_loop,
                   const SocketAddress &server_address):
	owner_loop_(owner_loop),
	// Create an IPv4/6, nonblocking, TCP socket file descriptor, abort if any error.
	server_socket_(nso::CreateNonblockingTcpSocket(server_address.socket_family())),
	server_channel_(owner_loop_, server_socket_.socket()),
	listening_(false),
	idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	assert(idle_fd_ >= 0);
	server_socket_.SetReuseAddress(true); // Enable SO_REUSEADDR
	server_socket_.SetReusePort(true);
	server_socket_.Bind(server_address);
	server_channel_.set_event_callback(Channel::READ_CALLBACK,
	                                   bind(&Acceptor::HandleRead, this));
}
// When the listening socket(i.e., the server_socket_) is readable, Poller::Poll()
// will return and calls Channel::HandleEvent(), which in turn calls this function.
// Call server_socket_.Accept() to accept(2) new connections
// and call new connection callback if user supply.
void Acceptor::HandleRead()
{
	owner_loop_->AssertInLoopThread();
	SocketAddress peer_address(0); // Construct an endpoint with given port number.
	// FIXME: Here we accept(2) one socket each time, which is suitable for long
	// connection. There two strategies for short-connection:
	// 1.	accept(2) until no more new connections arrive.
	// 2.	accept(2) N connections at a time, N normally is 10.
	int connection_fd = server_socket_.Accept(peer_address);
	// TODO: See 7.7 "How to avoid the use all of file descriptors?"
	if(connection_fd >= 0)
	{
		if(new_connection_callback_) // void(int, const SocketAddress&)
		{
			// FIXME: Here pass the connection_fd by value, which may not free memory
			// properly. C++11: create Socket object -> use std::move() to move this object
			// to new_connection_callback. This Guarantee the safe memory release.
			new_connection_callback_(connection_fd, peer_address);
		}
		else
		{
			::close(connection_fd);
		}
	}
	else
	{
		LOG_ERROR("server_socket_.Accept()");
		// TODO: "The special problem of accept()ing when you can't" - libev's doc.
		// The per-process limit of open file descriptors has been reached.
		if(errno == EMFILE)
		{
			::close(idle_fd_);
			// #include <sys/socket.h>
			// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
			// addr = NULL: nothing is filled in, and addrlen should also be NULL.
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

// Let server_channel_ monitor IO readable events and let server_socket_ to listen().
// Called by `TcpServer::Start()`
void Acceptor::Listen()
{
	owner_loop_->AssertInLoopThread();
	listening_ = true;
	// Monitor IO readable events. The trigger for Channel::HandleEvent() is
	// that the listening socket is readable, which indicates new connection arrives.
	server_channel_.set_requested_event(Channel::READ_EVENT);
	server_socket_.Listen();
}

