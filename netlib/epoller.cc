#include <netlib/epoller.h>

#include <assert.h> // assert()
#include <strings.h> // bzero()
#include <sys/epoll.h> // epoll()
#include <unistd.h> // close()

#include <netlib/channel.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>

using netlib::Epoller;
using netlib::TimeStamp;

Epoller::Epoller(EventLoop *owner_loop):
	epoll_fd_(CreateEpollFd()),
	active_event_vector_(kInitialActiveEventVectorSize),
	owner_loop_(owner_loop)
{}
int Epoller::CreateEpollFd()
{
	int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
	if(epoll_fd < 0)
	{
		LOG_FATAL("epoll_create1(): FATAL");
	}
	return epoll_fd;
}

Epoller::~Epoller()
{
	::close(epoll_fd_);
}

TimeStamp Epoller::EpollWait(int timeout, ChannelVector &active_channel)
{
	AssertInLoopThread();
	LOG_TRACE("total added channel number = %lu", channel_set_.size());
	int max_event_number = static_cast<int>(active_event_vector_.size());
	int event_number = ::epoll_wait(epoll_fd_,
	                                active_event_vector_.data(),
	                                max_event_number,
	                                timeout);
	TimeStamp epoll_return_time(TimeStamp::Now());
	int saved_errno = errno; // NOTE: Must handle error.
	if(event_number > 0)
	{
		LOG_TRACE("%d events happened.", event_number);
		for(int index = 0; index < event_number; ++index)
		{
			Channel *channel = // Store Channel* in [].data.ptr in EpollCtl().
			    static_cast<Channel*>(active_event_vector_[index].data.ptr);
			channel->set_returned_event(active_event_vector_[index].events);
			active_channel.push_back(channel);
		}
		if(event_number == max_event_number)
		{
			active_event_vector_.resize(max_event_number * 2);
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
			LOG_ERROR("epoll_wait(): ERROR");
		}
	}
	return epoll_return_time;
}
void Epoller::AssertInLoopThread() const // Must be in IO thread.
{
	owner_loop_->AssertInLoopThread();
}

namespace
{
const int kRaw = -1; // Not in channel_set_.
const int kAdded = 1; // In channel_set_ and Added into epoll's RB-Tree.
const int kDeleted = 0; // In channel_set_ and Deleted from epoll's RB-Tree.
// kRaw ->
// 1.	kAdded. AOUC() {channel_set_.insert() -> ADD}
// kAdded ->
// 1.	kRaw. RC() {DEL -> channel_set_.erase()}
// 2.	kAdded. AOUC() {if(IsRequested(NoneEvent) == false) MOD}
// 3.	kDeleted. AOUC() {if(IR(NE) == true) DEL}
// kDeleted ->
// 1.	kRaw. RC() {channel_set_.erase()}
// 2.	kAdded. AOUC() {ADD}
}
void Epoller::AddOrUpdateChannel(Channel *channel)
{
	AssertInLoopThread();
	int channel_state = channel->state_in_epoller();
	LOG_TRACE("channel_fd = %d, requested_event = %d, channel_state = %d",
	          channel->fd(), channel->requested_event(), channel_state);
	switch(channel_state)
	{
	case kRaw:
		assert(channel_set_.find(channel) == channel_set_.end());
		channel_set_.insert(channel);
	// NO `break;` to avoid duplicate codes!
	case kDeleted:
		assert(channel_set_.find(channel) != channel_set_.end());
		EpollCtl(EPOLL_CTL_ADD, channel);
		channel->set_state_in_epoller(kAdded);
		break;
	case kAdded:
		assert(channel_set_.find(channel) != channel_set_.end());
		if(channel->IsRequested(Channel::NONE_EVENT) == true)
		{
			EpollCtl(EPOLL_CTL_DEL, channel);
			channel->set_state_in_epoller(kDeleted);
		}
		else
		{
			EpollCtl(EPOLL_CTL_MOD, channel);
		}
		break;
	default:
		LOG_FATAL("Unknown channel_state!");
	}
}
void Epoller::EpollCtl(int operation, Channel *channel)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.data.ptr = channel;
	event.events = channel->requested_event();
	int fd = channel->fd();
	LOG_TRACE("epoll_ctl operation = %s, fd = %d, requested_event = {%s}",
	          OperationToCString(operation), fd, channel->RequestedEventToString().c_str());
	if(::epoll_ctl(epoll_fd_, operation, fd, &event) == -1)
	{
		LOG_FATAL("epoll_ctl(): FATAL. operation = %s fd = %d",
		          OperationToCString(operation), fd);
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

void Epoller::RemoveChannel(Channel *channel)
{
	AssertInLoopThread();
	assert(channel->IsRequested(Channel::NONE_EVENT) == true);

	if(channel->state_in_epoller() == kAdded)
	{
		EpollCtl(EPOLL_CTL_DEL, channel);
	}
	assert(channel_set_.erase(channel) == 1);
	channel->set_state_in_epoller(kRaw);
}

bool Epoller::HasChannel(Channel *channel) const
{
	AssertInLoopThread();
	return channel_set_.find(channel) != channel_set_.end();
}
