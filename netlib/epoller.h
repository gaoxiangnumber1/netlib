#ifndef NETLIB_NETLIB_POLLER_H_
#define NETLIB_NETLIB_POLLER_H_

#include <map>
#include <vector>

#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

struct epoll_event; // Forward declaration, don't need include <sys/epoll.h>

namespace netlib
{

class EventLoop;
class Channel;

class Epoller: public NonCopyable // Encapsulate IO multiplexing epoll(4).
{
public:
	using ChannelVector = std::vector<Channel*>;

	Epoller(EventLoop *owner_loop);
	// Close this epoll instance's file descriptor epoll_fd_.
	~Epoller();

	void AssertInLoopThread() const; // Must be in IO thread.

	// Invoke ::epoll_wait(2) to get the number of active IO events and
	// invoke FillActiveChannel to fill active_channel_.
	// Must be called in the loop thread.
	TimeStamp EpollWait(int timeout, ChannelVector *active_channel);

	// Called: Channel::~Dtor()(for assertion) -> EventLoop::HasChannel() -> here.
	bool HasChannel(Channel *channel) const;
	// Add new Channel(O(logN)) or update already existing Channel(O(1))
	// in `vector<struct epoll_event> returned_epoll_event_vector_`
	// `void Channel::Update()` -> `void EventLoop::AddOrUpdateChannel(Channel*)` -> here.
	// Must be called in the loop thread.
	void AddOrUpdateChannel(Channel *channel);
	// Remove the channel when it destructs. Must be called in the loop thread.
	void RemoveChannel(Channel *channel);

private:
	using ChannelMap = std::map<int, Channel*>; // Map file_descriptor to Channel.

	// Traverse epoll_event vector to find active fd; Fetch this fd's corresponding channel;
	// EpollCtl its returned_events and add it to active_channel_.
	void FillActiveChannel(int event_number, ChannelVector *active_channel) const;
	// Called for LOG_TRACE.
	static const char *OperationToString(int operation);
	//
	void EpollCtl(int operation, Channel *channel);

	int epoll_fd_; // This epoll instance's file descriptor.
	// 1. Ctor(): Initialize to kInitialReturnedEpollEventVectorSize.
	// 2. EpollWait(): Store all the `struct epoll_event` that has active IO events.
	// 3. FillActiveChannel(): Used to get active Channel* and corresponding events.
	std::vector<struct epoll_event> returned_epoll_event_vector_;
	static const int kInitialReturnedEpollEventVectorSize = 16;
	// Epoller is an indirect member of EventLoop, it is invoked by its owner
	// EventLoop in IO thread. Thus, we don't need use MutexLock to protect it.
	// Epoller's life time is the same as its owner EventLoop.
	EventLoop *owner_loop_;
	// Epoller doesn't own Channel, so Channel object must unregister by itself calling
	// `EventLoop::RemoveChannel()` to avoid dangling pointer.
	// 1.	AddOrUpdateChannel(): add new <fd:channel> pair if before state is kRaw, that is,
	//		kRaw->kAdded.
	// 2.	RemoveChannel(): erase <fd:channel> pair. The before state must be either
	//		kAdded or kDeleted, that is, kAdded/kDeleted -> kRaw.
	// 3.	Other uses are for assertion.
	// Note:	kRaw state Channel isn't in any Epoller's channel_map_; kDeleted state
	//				channel still in channel_map_, but not in epoll instance.
	ChannelMap channel_map_;
};

}

#endif // NETLIB_NETLIB_POLLER_H_
