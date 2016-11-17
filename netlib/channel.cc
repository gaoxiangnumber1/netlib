#include <netlib/channel.h>

#include <poll.h> // POLL*

#include <netlib/event_loop.h> // EventLoop
#include <netlib/logging.h> // Log

using std::string;
using std::shared_ptr;
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
	state_in_epoller_(-1), // Epoller::kRaw.
	tied_(false),
	event_handling_(false),
	added_to_loop_(false)
{}

Channel::~Channel()
{
	assert(event_handling_ == false);
	assert(added_to_loop_ == false);
	if(owner_loop_->IsInLoopThread() == true)
	{
		assert(owner_loop_->HasChannel(this) == false);
	}
}

void Channel::set_requested_event(RequestedEventType type)
{
	switch(type)
	{
	case READ:
		requested_event_ |= kReadEvent;
		break;
	case NOT_READ:
		requested_event_ &= ~kReadEvent;
		break;
	case WRITE:
		requested_event_ |= kWriteEvent;
		break;
	case NOT_WRITE:
		requested_event_ &= ~kWriteEvent;
		break;
	case NONE:
		requested_event_ = kNoneEvent;
	}
	AddOrUpdateChannel();
}
void Channel::set_event_callback(const EventCallback &callback, EventCallbackType type)
{
	switch(type)
	{
	case READ_CALLBACK:
		read_callback_ = callback;
		break;
	case WRITE_CALLBACK:
		write_callback_ = callback;
		break;
	case CLOSE_CALLBACK:
		close_callback_ = callback;
		break;
	case ERROR_CALLBACK:
		error_callback_ = callback;
	}
}
void Channel::set_tie(const shared_ptr<void> &object)
{
	tie_ = object;
	tied_ = true;
}

void Channel::HandleEvent(TimeStamp receive_time)
{
	if(tied_)
	{
		shared_ptr<void> guard = tie_.lock();
		if(guard)
		{
			HandleEventWithGuard(receive_time);
		}
	}
	else
	{
		HandleEventWithGuard(receive_time);
	}
}

// Call different callbacks based on the value of returned_event_.
// Invoked by EventLoop::Loop().
void Channel::HandleEventWithGuard(TimeStamp receive_time)
{
	event_handling_ = true;
	LOG_TRACE("%s", ReturnedEventToString().c_str());

	// POLLNVAL: file descriptor is not open.
	if(returned_event_ & POLLNVAL)
	{
		LOG_WARN("fd = %d, Channel::handle_event() POLLNVAL.", fd_);
	}

	// POLLRDHUP: Shutdown on peer socket.
	if((returned_event_ & (POLLIN | POLLPRI | POLLRDHUP)) && read_callback_)
	{
		read_callback_(receive_time);
	}
	if((returned_event_ & POLLOUT) && write_callback_)
	{
		write_callback_(receive_time);
	}
	// POLLHUP: hangup has occurred.
	if((returned_event_ & POLLHUP) &&
	        !(returned_event_ & POLLIN) &&
	        close_callback_)
	{
		close_callback_(receive_time);
	}
	// POLLERR: An error has occurred.
	if((returned_event_ & (POLLERR | POLLNVAL)) && error_callback_)
	{
		error_callback_(receive_time);
	}
	event_handling_ = false;
}

void Channel::AddOrUpdateChannel()
{
	added_to_loop_ = true;
	owner_loop_->AddOrUpdateChannel(this);
	// Invoke `void Epoller::AddOrUpdateChannel(Channel*)`
}

void Channel::RemoveChannel()
{
	assert(IsRequestedNoneEvent() == true);
	added_to_loop_ = false;
	owner_loop_->RemoveChannel(this);
}

string Channel::RequestedEventToString() const
{
	return EventToString(fd_, requested_event_);
}

string Channel::ReturnedEventToString() const
{
	return EventToString(fd_, returned_event_);
}

string Channel::EventToString(int fd, int event)
{
	char buffer[32] = "";
	char *ptr = buffer, *buffer_end = buffer + sizeof buffer;
	ptr += snprintf(ptr, buffer_end - ptr, "%d: ", fd);
	if(event & POLLIN)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "IN ");
	}
	if(event & POLLPRI)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "PRI ");
	}
	if(event & POLLOUT)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "OUT ");
	}
	if(event & POLLHUP)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "HUP ");
	}
	if(event & POLLRDHUP)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "RDHUP ");
	}
	if(event & POLLERR)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "ERR ");
	}
	if(event & POLLNVAL)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "NVAL ");
	}
	return buffer;
}
