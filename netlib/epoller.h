#ifndef NETLIB_NETLIB_POLLER_H_
#define NETLIB_NETLIB_POLLER_H_

#include <set>
#include <vector>

#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

struct epoll_event; // Forward declaration, don't need include <sys/epoll.h>

namespace netlib
{

class EventLoop;
class Channel;

// Review:

// Interface:
// Ctor -> -CreateEpollFd
// Dtor
// EpollWait -> -AssertInLoopThread
// AddOrUpdateChannel -> -AssertInLoopThread -> -EpollCtl
//			-EpollCtl -> -OperationToCString
// RemoveChannel -> -AssertInLoopThread -> -EpollCtl.
// HasChannel -> -AssertInLoopThread.

class Epoller: public NonCopyable
{
public:
	using ChannelVector = std::vector<Channel*>;

	Epoller(EventLoop *owner_loop);
	~Epoller();

	TimeStamp EpollWait(int timeout_in_millisecond, ChannelVector &active_channel);
	void AddOrUpdateChannel(Channel *channel);
	void RemoveChannel(Channel *channel);
	bool HasChannel(Channel *channel) const;

private:
	using EpollEventVector = std::vector<struct epoll_event>;
	using ChannelSet = std::set<Channel*>;

	int CreateEpollFd();
	void AssertInLoopThread() const;
	void EpollCtl(int operation, Channel *channel);
	static const char *OperationToCString(int operation);

	int epoll_fd_;
	EpollEventVector active_event_vector_;
	static const int kInitialActiveEventVectorSize = 16;
	EventLoop *owner_loop_;
	ChannelSet channel_set_;
};

}

#endif // NETLIB_NETLIB_POLLER_H_
