#include <netlib/epoller.h>

#include <assert.h> // assert()
#include <strings.h> // bzero()
#include <sys/epoll.h> // epoll()

#include <netlib/channel.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/socket_operation.h>

using netlib::Epoller;
using netlib::TimeStamp;

namespace
{
// Meaning: The Channel object isn't in any Epoller object's channel_set_.
const int kRaw = -1;
// Meaning: The Channel object is in this Epoller's channel_set_ and is waiting for
// IO events(added into this Epoller's epoll instance).
const int kAdded = 1;
// Meaning: The Channel object is in this Epoller's channel_set_ and is not waiting
// for IO events(deleted from this Epoller's epoll instance).
const int kDeleted = 0;
// Total SIX kinds of states conversions:
// kRaw ->
// 1.	kAdded: in AddOrUpdateChannel(); add new <fd:channel> into channel_set_ -> ADD.
// kAdded ->
// 1.	kRaw: in RemoveChannel(); channel_set_.erase(fd) -> DEL.
// 2.	kAdded: in UC(); if(IsRequestedNoneEvent() == false), then MOD.
// 3.	kDeleted: in UC(); if(INRE == true), then DEL.
// kDeleted ->
// 1.	kRaw: in RC(); channel_set_.erase(fd).
// 2.	kAdded: in UC(); ADD.
}

// #include <sys/epoll.h>
// int epoll_create(int size);
// int epoll_create1(int flags);
// ----------------------
// epoll_create() returns a file descriptor referring to the new epoll instance.
// This file descriptor is used for all the subsequent calls to the epoll interface.
// When not needed, this file descriptor should be closed by close(2). When all file
// descriptors referring to an epoll instance have been closed, the kernel destroys the
// instance and releases the associated resources for reuse.
// ----------------------
// `size` is ignored, but must be greater than zero. In the initial implementation,
// `size` informed the kernel of the number of file descriptors that the caller expected
// to add to the epoll instance. The kernel used this information as a hint for the
// amount of space to initially allocate in internal data structures describing events.
// Now, the kernel dynamically sizes the required data structures without needing the
// hint, but size must be greater than zero to ensure backward compatibility when new
// epoll applications are run on older kernels.
// ----------------------
// If `flags` is 0, then epoll_create1() is the same as epoll_create(). EPOLL_CLOEXEC
// Set the close-on-exec(FD_CLOEXEC) flag on the new file descriptor.
// Return a nonnegative file descriptor on success; -1 on error and errno is set.
Epoller::Epoller(EventLoop *owner_loop):
	epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
	returned_epoll_event_vector_(kInitialReturnedEpollEventVectorSize),
	owner_loop_(owner_loop)
{
	if(epoll_fd_ == -1)
	{
		LOG_FATAL("Epoller(): create epoll_fd_ fails.");
	}
}

Epoller::~Epoller()
{
	nso::Close(epoll_fd_);
}

// int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
// epoll_wait() waits for events on the epoll(7) instance referred to by `epfd`.
// The memory pointed to by `events` contains the events.
// `maxevents` must be greater than zero and up to `maxevents` are returned.
// `timeout` is the milliseconds that it will block(-1: block indefinitely; 0: return
// immediately, even if no events are available).
// Return the number of file descriptors ready for I/O on success
// (0 if no file descriptor is ready). -1 on error and errno is set.
// 		typedef union epoll_data
// 		{
// 			void    *ptr;
// 			int      fd;
// 			uint32_t u32;
// 			uint64_t u64;
// 		} epoll_data_t;
// 		struct epoll_event
// 		{
// 			uint32_t     events;    /* Epoll events */
// 			epoll_data_t data;      /* User data variable */
// 		};
// The data of each returned structure contains the same data the user set with an
// epoll_ctl(2)(EPOLL_CTL_ADD, EPOLL_CTL_MOD);
// the events member contains the returned event bit field.

// For returned_epoll_event_vector_[0, event_number - 1]: Get [].data.fd's Channel*
// from [].data.ptr(set in EpollCtl()) ->
// set_returned_event([].events) -> add this Channel* into active_channel(supplied
// by the EventLoop object owner_loop_, it clears active_channel each time before
// calling EpollWait() in EventLoop::Loop()).
// O(1) since event_number is always a constant no matter how big it is.
TimeStamp Epoller::EpollWait(int timeout, ChannelVector &active_channel)
{
	AssertInLoopThread();
	LOG_TRACE("total added channel number = %lu", channel_set_.size());
	// 1.	Call epoll_wait() to get the `epoll_event` whose fd has active IO events
	//		and store them in returned_epoll_event_vector_.
	// vector.data() <-> &(*vector.begin())
	int event_number = ::epoll_wait(epoll_fd_,
	                                returned_epoll_event_vector_.data(),
	                                static_cast<int>(returned_epoll_event_vector_.size()),
	                                timeout);
	TimeStamp now(TimeStamp::Now());
	int saved_errno = errno;
	if(event_number > 0)
	{
		LOG_TRACE("%d events happened.", event_number);
		// 2. There has ready fds and use their Channel* to fill active channel.
		for(int index = 0; index < event_number; ++index)
		{
			Channel *channel = // Store Channel* in [].data.ptr in EpollCtl().
			    static_cast<Channel*>(returned_epoll_event_vector_[index].data.ptr);
			channel->set_returned_event(returned_epoll_event_vector_[index].events);
			active_channel.push_back(channel);
		}
		if(event_number == static_cast<int>(returned_epoll_event_vector_.size()))
		{
			returned_epoll_event_vector_.resize(returned_epoll_event_vector_.size() * 2);
		}
	}
	else if(event_number == 0)
	{
		LOG_TRACE("Nothing happened.");
	}
	else
	{
		if(saved_errno != EINTR)
		{
			errno = saved_errno;
			LOG_ERROR("Epoller::EpollWait() error");
		}
	}
	return now;
}

void Epoller::AssertInLoopThread() const // Must be in IO thread.
{
	owner_loop_->AssertInLoopThread();
}

// Add new Channel(O(logN)) or update already existing Channel(O(1)).
// Called: `void Channel::set_requested_event_*()` -> `void Channel::Update()` ->
// `void EventLoop::AddOrUpdateChannel(Channel*)` -> here.
void Epoller::AddOrUpdateChannel(Channel *channel)
{
	AssertInLoopThread();
	int fd = channel->fd(), channel_state = channel->state_in_epoller();
	LOG_TRACE("channel_fd = %d, requested_event = %d, channel_state = %d",
	          fd, channel->requested_event(), channel_state);

	// A new one, add with EPOLL_CTL_ADD.
	if(channel_state == kRaw || channel_state == kDeleted)
	{
		if(channel_state == kRaw) // The initial value for index_ of every Channel object.
		{
			assert(channel_set_.find(channel) == channel_set_.end());
			channel_set_.insert(channel); // Add new channel.
		}
		else
		{
			assert(channel_set_.find(channel) != channel_set_.end());
		}
		channel->set_state_in_epoller(kAdded);
		EpollCtl(EPOLL_CTL_ADD, channel);
	}
	// EpollCtl existing one with EPOLL_CTL_MOD/DEL.
	else
	{
		assert(channel_set_.find(channel) != channel_set_.end());
		assert(channel_state = kAdded);
		// If this channel doesn't interest in any events.
		if(channel->IsRequestedNoneEvent() == true)
		{
			channel->set_state_in_epoller(kDeleted);
			// Remove its file descriptor from this epoll instance(referred to by epoll_fd_).
			EpollCtl(EPOLL_CTL_DEL, channel);
		}
		else
		{
			EpollCtl(EPOLL_CTL_MOD, channel);
		}
	}
}

// #include <sys/epoll.h>
// int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// Perform the operation `op` on the epoll(7) instance referred to by `epfd`
// for the file descriptor `fd`.
// The `event` argument describes the object linked to the file descriptor fd.
// EPOLL_CTL_ADD	Register the target file descriptor fd on the epoll instance
// referred to by `epfd` and associate the `event` with the internal file linked to fd.
// EPOLL_CTL_MOD	Change the `event` associated with the target file descriptor fd.
// EPOLL_CTL_DEL	Remove(unregister) the target file descriptor fd from the epoll
// instance referred to by `epfd`. The event is ignored and can be NULL.
// Return 0 on success; -1 on error and errno is set.
void Epoller::EpollCtl(int operation, Channel *channel)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->requested_event();
	event.data.ptr = channel;
	int fd = channel->fd();
	LOG_TRACE("epoll_ctl operation = %s, fd = %d, requested_event = {%s}",
	          OperationToCString(operation), fd, channel->RequestedEventToString().c_str());
	if(::epoll_ctl(epoll_fd_, operation, fd, &event) == -1)
	{
		if(operation == EPOLL_CTL_DEL)
		{
			LOG_ERROR("epoll_ctl operation = %s fd = %d",
			          OperationToCString(operation), fd);
		}
		else
		{
			LOG_FATAL("epoll_ctl operation = %s fd = %d",
			          OperationToCString(operation), fd);
		}
	}
}

const char *Epoller::OperationToCString(int operation)
{
	switch(operation)
	{
	case EPOLL_CTL_ADD:
		return "ADD";
	case EPOLL_CTL_DEL:
		return "DEL";
	case EPOLL_CTL_MOD:
		return "MOD";
	default:
		assert(false && "ERROR operation");
		return "Unknown Operation";
	}
}

// Remove `channel` from channel_set_.
// Called: Every TcpConnection object owns a `socket_` and use a `channel_` to
// monitor its socket's IO events. `channel_.close_callback_` is
// `TcpConnection::HandleClose()` -> `TcpConnection::close_callback_` is
// `TcpServer::RemoveConnection()` -> `TcpConnection::ConnectDestroyed()` ->
// `EventLoop::RemoveChannel()` -> here.
void Epoller::RemoveChannel(Channel *channel)
{
	AssertInLoopThread();
	int fd = channel->fd(), channel_state = channel->state_in_epoller();
	LOG_TRACE("channel_fd = %d, channel_state = %d", fd, channel_state);
	assert(channel_set_.find(channel) != channel_set_.end());
	// Always `channel->set_requested_event_none()` before RemoveChannel().
	assert(channel->IsRequestedNoneEvent() == true);
	assert(channel_state == kAdded || channel_state == kDeleted);
	// Remove this Channel from channel_set_:
	assert(channel_set_.erase(channel) == 1);
	if(channel_state == kAdded)
	{
		EpollCtl(EPOLL_CTL_DEL, channel);
	}
	channel->set_state_in_epoller(kRaw);
}

bool Epoller::HasChannel(Channel *channel) const
{
	AssertInLoopThread();
	ChannelSet::const_iterator it = channel_set_.find(channel);
	return it != channel_set_.end() && *it == channel;
}
