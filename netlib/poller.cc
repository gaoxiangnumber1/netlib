#include <netlib/poller.h>

#include <assert.h> // assert()
#include <poll.h> // poll()

#include <netlib/channel.h>
#include <netlib/logging.h>
#include <netlib/time_stamp.h>

using netlib::Channel;
using netlib::EventLoop;
using netlib::Poller;
using netlib::TimeStamp;

Poller::Poller(EventLoop *owner_loop): owner_loop_(owner_loop) {}
Poller::~Poller() {}

// Invoke ::poll() to get the number of active IO events and
// invoke FillActiveChannel to fill active_channel_.
TimeStamp Poller::Poll(int timeout, ChannelVector *active_channel_)
{
	// int poll(struct pollfd *fds, nfds_t nfds, int timeout); timeout is in millisecond.
	// pollfd_vector_.data() <-> &(*pollfd_vector_.begin())
	int event_number = ::poll(pollfd_vector_.data(), pollfd_vector_.size(), timeout);
	TimeStamp now(TimeStamp::Now());
	if(event_number > 0)
	{
		//LOG_INFO("%d events happened.", event_number);
		FillActiveChannel(event_number, active_channel_);
	}
	else if(event_number == 0)
	{
		LOG_INFO("Nothing happened.");
	}
	else
	{
		LOG_INFO("Poller::Poll()::poll() error");
	}
	return now;
}

// Add new Channel(O(logN)) or update already existing Channel(O(1))
// in `vector<struct pollfd> pollfd_vector_`
// TODO: why O(logN) or O(1)?
// `void Channel::Update()` -> `void EventLoop::UpdateChannel(Channel*)` -> here.
void Poller::UpdateChannel(Channel *channel)
{
	AssertInLoopThread();
//	LOG_INFO("channel_fd = %d, requested_event = %d",
//	         channel->fd(), channel->requested_event());
	if(channel->index() < 0) // A new channel
	{
		assert(channel_map_.find(channel->fd()) == channel_map_.end()); // Check invariant.
		// Construct a new pollfd that represents this new channel.
		struct pollfd new_pollfd;
		new_pollfd.fd = channel->fd();
		new_pollfd.events = static_cast<short>(channel->requested_event());
		new_pollfd.revents = 0;
		// Add this pollfd to the cached pollfd array(pollfd_vector_)
		pollfd_vector_.push_back(new_pollfd);
		// Set this channel's index_ data member.
		channel->set_index(static_cast<int>(pollfd_vector_.size()) - 1);
		// Add this new file_descriptor:channel to map.
		channel_map_[new_pollfd.fd] = channel;
	}
	else // Update already existing channel.
	{
		assert(channel_map_.find(channel->fd()) != channel_map_.end());
		assert(channel_map_[channel->fd()] == channel);
		int index = channel->index(); // The index of this channel's pollfd in pollfd_vector_.
		assert(0 <= index && index < static_cast<int>(pollfd_vector_.size()));

		struct pollfd &pfd = pollfd_vector_[index]; // Fetch pollfd by reference.
		// If `pfd.fd != channel->fd()`, then pfd.fd must be -1(see below), otherwise error.
		assert(pfd.fd == channel->fd() || pfd.fd == -1);
		// Update this pollfd's events and reset revents.
		pfd.events = static_cast<short>(channel->requested_event());
		pfd.revents = 0;
		// If this channel doesn't interest in any events, set pfd.fd to -1 to make poll(2)
		// ignore this file descriptor. Note that we can't set pfd.events to 0 since this can't
		// mask POLLERR event.
		if(channel->IsNoneEvent())
		{
			pfd.fd = -1; // Ignore this pollfd.
		}
	}
}

// Traverse pollfd vector to find active fd; Fetch this fd's corresponding channel;
// Update its returned_events and add it to active_channel_.
// O(N), N is the length of pollfd_vector_ vector, i.e., the number of file descriptors.
void Poller::FillActiveChannel(int event_number, ChannelVector *active_channel) const
{
	for(PollfdVector::const_iterator pfd = pollfd_vector_.begin();
	        pfd != pollfd_vector_.end() && event_number > 0; ++pfd)
	{
		if(pfd->revents > 0) // This struct pollfd's file-descriptor has IO events.
		{
			--event_number; // Remove one event. Don't need traverse all pollfd_vector_.
			// Fetch the fd:channel pair in channel map by this pollfd's fd.
			ChannelMap::const_iterator ch = channel_map_.find(pfd->fd);
			assert(ch != channel_map_.end());
			Channel *channel = ch->second; // The channel that monitoring this fd.
			assert(channel->fd() == pfd->fd);
			// Update this channel's returned events: used in Channel::HandleEvent().
			channel->set_returned_event(pfd->revents);
			// Add this updated channel to the active channel vector.
			active_channel->push_back(channel);
		}
	}
}
