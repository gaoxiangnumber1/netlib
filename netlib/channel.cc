#include <netlib/channel.h>

#include <poll.h> // POLL*

#include <netlib/event_loop.h> // EventLoop
#include <netlib/logging.h> // Logger

using netlib::EventLoop;
using netlib::Channel;

const int Channel::kNoneEvent = 0;
// POLLIN: Data other than high-priority data can be read.
// POLLPRI: High-priority data can be read.
const int Channel::kReadEvent = POLLIN | POLLPRI;
// POLLOUT: Normal data can be written.
const int Channel::kWriteEvent = POLLOUT;

// declaration of ‘fd’ shadows a member of 'this' [-Werror=shadow]
Channel::Channel(EventLoop *loop, int file_descriptor):
	owner_loop_(loop),
	fd_(file_descriptor),
	requested_event_(kNoneEvent),
	returned_event_(kNoneEvent),
	index_(-1)
{}

// Call different callbacks based on the value of returned_event_.
// Invoked by EventLoop::Loop().
void Channel::HandleEvent()
{
	// POLLNVAL: file descriptor is not open.
	if(returned_event_ & POLLNVAL)
	{
		LOG_INFO("Channel::handle_event() POLLNVAL");
	}
	// POLLERR: An error has occurred.
	if((returned_event_ & (POLLERR | POLLNVAL)) && error_callback_)
	{
		error_callback_();
	}
	// POLLRDHUP: Shutdown on peer socket.
	if((returned_event_ & (POLLIN | POLLPRI | POLLRDHUP)) && read_callback_)
	{
		read_callback_();
	}
	if((returned_event_ & POLLOUT) && write_callback_)
	{
		write_callback_();
	}
}

void Channel::Update()
{
	owner_loop_->UpdateChannel(this);
	// Invoke `void Poller::UpdateChannel(Channel*)`
}
