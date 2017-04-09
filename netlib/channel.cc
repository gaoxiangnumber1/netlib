#include <netlib/channel.h>

#include <netlib/event_loop.h> // EventLoop
#include <netlib/logging.h> // Log

using std::string;
using std::shared_ptr;
using netlib::EventLoop;
using netlib::Channel;

Channel::Channel(EventLoop *loop, int file_descriptor):
	owner_loop_(loop),
	state_in_epoller_(-1), // Epoller::kRaw.
	fd_(file_descriptor),
	requested_event_(kNoneEvent),
	returned_event_(kNoneEvent),
	event_handling_(false),
	tied_(false)
{}

Channel::~Channel()
{
	assert(event_handling_ == false);
	if(owner_loop_->IsInLoopThread() == true)
	{
		assert(owner_loop_->HasChannel(this) == false);
	}
}

void Channel::set_requested_event(RequestedEventType type)
{
	if(IsRequested(type) == true)
	{
		return;
	}
	switch(type)
	{
	case READ_EVENT:
		requested_event_ |= kReadEvent;
		break;
	case NOT_READ:
		requested_event_ &= ~kReadEvent;
		break;
	case WRITE_EVENT:
		requested_event_ |= kWriteEvent;
		break;
	case NOT_WRITE:
		requested_event_ &= ~kWriteEvent;
		break;
	case NONE_EVENT:
		requested_event_ = kNoneEvent;
		break;
	default:
		LOG_FATAL("Unknown RequestedEventType");
	}
	AddOrUpdateChannel();
}
void Channel::AddOrUpdateChannel()
{
	owner_loop_->AddOrUpdateChannel(this);
}

void Channel::set_tie(const shared_ptr<void> &object)
{
	tie_ = object;
	tied_ = true;
}

void Channel::set_event_callback(EventCallbackType type, const EventCallback &callback)
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
		break;
	default:
		LOG_FATAL("Unknown EventCallbackType!");
	}
}

bool Channel::IsRequested(RequestedEventType type)
{
	switch(type)
	{
	case READ_EVENT:
		return requested_event_ & kReadEvent;
	case NOT_READ:
		return !(requested_event_ & kReadEvent);
	case WRITE_EVENT:
		return requested_event_ & kWriteEvent;
	case NOT_WRITE:
		return !(requested_event_ & kWriteEvent);
	case NONE_EVENT:
		return requested_event_ == kNoneEvent;
	default:
		LOG_FATAL("Unknown RequestedEventType");
		return true;
	}
}

void Channel::HandleEvent(const TimeStamp &receive_time)
{
	if(tied_ == true)
	{
		// Prevent the owner object destructs in HandleEvent.
		shared_ptr<void> guard = tie_.lock(); // NOTE: Not `shared_ptr<void> &`
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
void Channel::HandleEventWithGuard(const TimeStamp &receive_time)
{
	event_handling_ = true;
	LOG_TRACE("%s", ReturnedEventToString().c_str());

	if((returned_event_ & kReadEvent) && read_callback_)
	{
		read_callback_(receive_time);
	}
	if((returned_event_ & kWriteEvent) && write_callback_)
	{
		write_callback_(receive_time);
	}
	if((returned_event_ & kCloseEvent) &&
	        !(returned_event_ & EPOLLIN) &&
	        close_callback_)
	{
		close_callback_(receive_time);
	}
	if((returned_event_ & kErrorEvent ) && error_callback_)
	{
		error_callback_(receive_time);
	}
	event_handling_ = false;
}

void Channel::RemoveChannel()
{
	assert(IsRequested(NONE_EVENT) == true);
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
	char buffer[32];
	char *ptr = buffer, *buffer_end = buffer + sizeof buffer;
	ptr += snprintf(ptr, buffer_end - ptr, "%d: ", fd);
	if(event & EPOLLIN)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "IN ");
	}
	if(event & EPOLLPRI)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "PRI ");
	}
	if(event & EPOLLRDHUP)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "RDHUP ");
	}
	if(event & EPOLLOUT)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "OUT ");
	}
	if(event & EPOLLHUP)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "HUP ");
	}
	if(event & EPOLLERR)
	{
		ptr += snprintf(ptr, buffer_end - ptr, "%s", "ERR ");
	}
	return buffer;
}
