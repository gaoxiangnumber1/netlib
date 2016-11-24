#ifndef NETLIB_NETLIB_CHANNEL_H_
#define NETLIB_NETLIB_CHANNEL_H_

#include <string>

#include <netlib/callback.h>
#include <netlib/non_copyable.h>

namespace netlib
{

class EventLoop;

// Interface:
// Ctor(EventLoop*, int).
// Dtor.
// Getter: owner_loop, fd, requested_event, state_in_channel.
// Setter:	set_requested_event -> -AddOrUpdateChannel.
//				set_returned_event, set_state_in_epoller, set_tie, set_event_callback.
// IsRequestedArgumentEvent.
// HandleEvent -> -HandleEventWithGuard.
// RemoveChannel.
// RequestedEventToString/Returned -> -EventToString.

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

	Channel(EventLoop *owner_loop, int file_descriptor);
	~Channel();

	// Getter.
	// Called: EventLoop::AddOrUpdateChannel()/RemoveChannel() for assertion.
	EventLoop *owner_loop() const
	{
		return owner_loop_;
	}
	int fd() const
	{
		return fd_;
	}
	int requested_event() const // Called: Epoller::EpollCtl().
	{
		return requested_event_;
	}
	int state_in_epoller() const // Called: Epoller::AddOrUpdate/RemoveChannel().
	{
		return state_in_epoller_;
	}

	// Setter: Since Channel's member functions can be invoked in only loop thread,
	// thus, we don't need set/release lock before/after updating data members.
	// Used by Channel object.
	void set_requested_event(RequestedEventType type);
	// Called: Epoller::EpollWait().
	void set_returned_event(int returned_event)
	{
		returned_event_ = returned_event;
	}
	// Called: Epoller::AddOrUpdateChannel()/RemoveChannel().
	void set_state_in_epoller(int new_state)
	{
		state_in_epoller_ = new_state;
	}
	// Tie this channel to the owner object managed by shared_ptr,
	// prevent the owner object being destroyed in HandleEvent.
	// `TcpConnection::ConnectEstablished() {channel_->set_tie(shared_from_this());}
	void set_tie(const std::shared_ptr<void> &object);
	void set_event_callback(EventCallbackType type, const EventCallback &callback);

	// Used for assertion.
	bool IsRequestedArgumentEvent(RequestedEventType type);

	// Call different callbacks based on returned_event_.
	// Called: EventLoop::Loop()
	void HandleEvent(TimeStamp receive_time);
	// Remove this channel from owner_loop_.
	// Called: TimerQueue::Dtor().
	void RemoveChannel();

	// For debug.
	std::string RequestedEventToString() const;
	std::string ReturnedEventToString() const;

private:
	// Used in `set_requested_event()`, `IsRequestedEvent()`
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	static const int kCloseEvent;
	static const int kErrorEvent;

	// set_requested_event() -> here -> EventLoop::AOUC(Channel*) ->
	// Epoller::AOUC(Channel*)
	// Add new Channel(O(logN)) or update already existing Channel(O(1))
	// in `channel_set_`.
	void AddOrUpdateChannel();
	// Called by HandleEvent(TimeStamp receive_time);
	void HandleEventWithGuard(TimeStamp receive_time);
	// Called by RequestedEventToString() and ReturnedEventToString().
	static std::string EventToString(int fd, int event);

	// Every Channel object is owned by only one EventLoop, so every Channel
	// object belongs to one loop thread. Channel object's life time is controlled by its
	// owner class, in this case, is the EventLoop class.
	EventLoop *owner_loop_;
	// Every Channel object is responsible for the dispatching of IO events of Only one
	// file descriptor(through different callbacks that denoted by std::function<>).
	// But it doesn't own this fd and doesn't close this fd when it destructs. The file
	// descriptor could be a socket, an event_fd, a timer_fd, or a signal_fd.
	const int fd_;
	int requested_event_; // Interested IO event, set by user.
	int returned_event_; // Active event, set by EventLoop::epoller_.
	int state_in_epoller_; // This Channel object's state in the Epoller class.
	// Store TcpConnection object's shared_ptr. Set in `Channel::set_tie()`
	// Used to judge whether the tied TcpConnection object is still alive in
	// HandEventWithGuard().
	std::weak_ptr<void> tie_;
	bool tied_;
	// NOTE: Set in HandleEventWithGuard(); Used for assertion in Dtor().
	bool event_handling_;
	// NOTE: Set in AddOrUpdateC() and RemoveC(); Used for assertion in Dtor().
	bool added_to_loop_;
	// Different callbacks called when corresponding event happens.
	EventCallback read_callback_;
	EventCallback write_callback_;
	EventCallback close_callback_;
	EventCallback error_callback_;
};

}

#endif // NETLIB_NETLIB_CHANNEL_H_
