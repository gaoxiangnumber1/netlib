#ifndef NETLIB_NETLIB_CHANNEL_H_
#define NETLIB_NETLIB_CHANNEL_H_

#include <sys/epoll.h> // EPOLL*

#include <string>

#include <netlib/function.h>
#include <netlib/non_copyable.h>

namespace netlib
{

class EventLoop;

// Interface:
// Ctor
// Dtor
// Getter: owner_loop, fd, requested_event, state_in_epoller
// Setter:	set_requested_event -> -AddOrUpdateChannel
//				set_returned_event, set_state_in_epoller, set_tie, set_event_callback
// IsRequested
// HandleEvent -> -HandleEventWithGuard
// RemoveChannel
// Requested/ReturnedEventToString -> -EventToString

class Channel: public NonCopyable // A selectable I/O channel.
{
public:
	enum RequestedEventType
	{
		READ_EVENT,
		NOT_READ,
		WRITE_EVENT,
		NOT_WRITE,
		NONE_EVENT
	};
	enum EventCallbackType
	{
		READ_CALLBACK,
		WRITE_CALLBACK,
		CLOSE_CALLBACK,
		ERROR_CALLBACK
	};

	Channel(EventLoop *loop, int file_descriptor);
	~Channel();

	// Getter.
	EventLoop *owner_loop() const
	{
		return owner_loop_;
	}
	int state_in_epoller() const
	{
		return state_in_epoller_;
	}
	int fd() const
	{
		return fd_;
	}
	int requested_event() const
	{
		return requested_event_;
	}

	// Setter
	void set_state_in_epoller(int new_state)
	{
		state_in_epoller_ = new_state;
	}
	void set_requested_event(RequestedEventType type);
	void set_returned_event(int returned_event)
	{
		returned_event_ = returned_event;
	}
	void set_event_callback(EventCallbackType type, const EventCallback &callback);
	void set_tie(const std::shared_ptr<void> &object);

	bool IsRequested(RequestedEventType type);
	void HandleEvent(const TimeStamp &receive_time);
	void RemoveChannel();

	std::string RequestedEventToString() const;
	std::string ReturnedEventToString() const;

private:
	void AddOrUpdateChannel();
	void HandleEventWithGuard(const TimeStamp &receive_time);
	static std::string EventToString(int fd, int event);

	static const int kNoneEvent = 0;
	static const int kReadEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
	static const int kWriteEvent = EPOLLOUT;
	static const int kCloseEvent = EPOLLHUP;
	static const int kErrorEvent = EPOLLERR;

	EventLoop *owner_loop_;
	int state_in_epoller_;
	int fd_;
	int requested_event_;
	int returned_event_;
	EventCallback read_callback_;
	EventCallback write_callback_;
	EventCallback close_callback_;
	EventCallback error_callback_;
	// Used to assert that this channel won't destruct in the process of event handling.
	bool event_handling_;
	// Store TcpConnection object's weak_ptr. Set in set_tie(); Used to judge whether
	// the tied TcpConnection object is alive in HandEventWithGuard().
	std::weak_ptr<void> tie_;
	bool tied_;
};

}

#endif // NETLIB_NETLIB_CHANNEL_H_
