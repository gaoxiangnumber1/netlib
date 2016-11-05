#ifndef NETLIB_NETLIB_POLLER_H_
#define NETLIB_NETLIB_POLLER_H_

#include <vector>
#include <map>

#include <netlib/event_loop.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

struct pollfd; // Forward declaration, don't need include <poll.h>

namespace netlib
{

class Channel;

class Poller: public NonCopyable // Encapsulate IO multiplexing(poll(2), epoll(4))
{
public:
	using ChannelVector = std::vector<Channel*>;

	Poller(EventLoop *owner_loop);
	~Poller();

	void AssertInLoopThread() // Must be in IO thread.
	{
		owner_loop_->AssertInLoopThread(); // Need include <event_loop.h>
	}
	// Invoke ::poll() to get the number of active IO events and
	// invoke FillActiveChannel to fill active_channel_.
	// TODO: return the time when poll(2) return.
	TimeStamp Poll(int timeout, ChannelVector *active_channel_);
	// Add new Channel(O(logN)) or update already existing Channel(O(1))
	// in `vector<struct pollfd> pollfd_vector_`
	// `void Channel::Update()` -> `void EventLoop::UpdateChannel(Channel*)` -> here.
	void UpdateChannel(Channel *channel);

private:
	using PollfdVector = std::vector<struct pollfd>; // The cached pollfd array.
	using ChannelMap = std::map<int, Channel*>; // Map file_descriptor to Channel.

	// Traverse pollfd vector to find active fd; Fetch this fd's corresponding channel;
	// Update its returned_events and add it to active_channel_.
	void FillActiveChannel(int event_number, ChannelVector *active_channel) const;

	// Poller is an indirect member of EventLoop, it is invoked by its owner EventLoop
	// in IO thread. Thus, we don't need set lock.
	// Poller's life time is the same as its owner EventLoop.
	EventLoop *owner_loop_;
	// 1.	UpdateChannel(Channel*): Store or update specified channel's
	//		monitoring file descriptor's `struct pollfd`.
	// 2.	Poll(int, ChannelVector*): Modified by kernel and it stores all the IO events
	//		bits in its element(struct pollfd).
	// 3.	FillActiveChannel(int, ChannelVector*): We use pollfd_vector_ to fetch the file
	//		descriptor that has IO events happened. Then we use this fd in channel map to
	//		find the channel that monitors this fd, and we update this channel's returned
	//		events. Finally, we add this updated channel to active channel vector.
	PollfdVector pollfd_vector_;
	// Poller doesn't own Channel, so Channel object must unregister by itself calling
	// `EventLoop::RemoveChannel()` to avoid dangling pointer.
	// 1.	UpdateChannel(Channel*): Store new fd:channel pair; find specified fd.
	// 2.	Poll(int, ChannelVector*): No use.
	// 3.	FillActiveChannel(int, ChannelVector*): Fetch the channel that monitors the
	//		file descriptor which has IO events happened.
	ChannelMap channel_map_;
};

}

#endif // NETLIB_NETLIB_POLLER_H_
