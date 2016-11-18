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

// Interface:
// Ctor.
// Dtor.
// EpollWait -> -AssertInLoopThread.
// AddOrUpdateChannel -> -AssertInLoopThread -> -EpollCtl.
// RemoveChannel -> -AssertInLoopThread -> -EpollCtl.
// HasChannel -> -AssertInLoopThread.

class Epoller: public NonCopyable // Encapsulate IO multiplexing epoll(4).
{
public:
	using ChannelVector = std::vector<Channel*>;

	Epoller(EventLoop *owner_loop);
	// Close this epoll instance's file descriptor epoll_fd_.
	~Epoller();

	// Invoke ::epoll_wait to get the active fds and fill active_channel.
	TimeStamp EpollWait(int timeout, ChannelVector &active_channel);

	// Add new Channel*(O(logN)) or update existing Channel*(O(1)) in channel_set_.
	// `Channel::AOUC(this)` -> `EventLoop::AOUC(Channel*)` -> here.
	void AddOrUpdateChannel(Channel *channel);
	// Remove the channel when it destructs.
	void RemoveChannel(Channel *channel);
	// Called: Channel::Dtor()(for assertion) -> EventLoop::HasChannel() -> here.
	bool HasChannel(Channel *channel) const;

private:
	using EpollEventVector = std::vector<struct epoll_event>;
	using ChannelSet = std::set<Channel*>; // Map file_descriptor to Channel.

	// Used for assertion in functions that must be called in loop thread.
	// EpollWait(), HasChannel(), AddOrUpdateChannel(), RemoveChannel().
	void AssertInLoopThread() const;
	// Wrapper of epoll_ctl(). Called by AddOrUpdateChannel() and RemoveChannel().
	void EpollCtl(int operation, Channel *channel);
	// Used for Debug.
	static const char *OperationToCString(int operation);

	int epoll_fd_; // This epoll instance's file descriptor.
	// 1. Ctor(): Initialize to kInitialReturnedEpollEventVectorSize.
	// 2. EpollWait(): Store all the `struct epoll_event` that has active IO events.
	// 3. FillActiveChannel(): Used to get active Channel* and corresponding events.
	EpollEventVector returned_epoll_event_vector_;
	static const int kInitialReturnedEpollEventVectorSize = 16;
	// Epoller is an indirect member of EventLoop, it is invoked by its owner
	// EventLoop in IO thread. Thus, we don't need use MutexLock to protect it.
	// Epoller's life time is the same as its owner EventLoop.
	EventLoop *owner_loop_;
	// Epoller doesn't own Channel, so Channel object must unregister by itself calling
	// `Channel::RemoveChannel()` -> `EventLoop::RemoveChannel()` ->
	// `Epoller::RemoveChannel()` to avoid dangling pointer.
	// 1.	AddOrUpdateChannel(): add new <fd:channel> pair if before state is kRaw,
	//		that is, kRaw->kAdded.
	// 2.	RemoveChannel(): erase <fd:channel> pair. The before state must be either
	//		kAdded or kDeleted, that is, kAdded/kDeleted -> kRaw.
	// 3.	Other uses are for assertion.
	// Note:	kRaw state Channel isn't in any Epoller's channel_set_; kDeleted state
	//				channel still in channel_set_, but not in epoll instance.
	ChannelSet channel_set_;
};

}

#endif // NETLIB_NETLIB_POLLER_H_
