#ifndef NETLIB_NETLIB_CHANNEL_H_
#define NETLIB_NETLIB_CHANNEL_H_

#include <string>

#include <netlib/callback.h>
#include <netlib/non_copyable.h>

namespace netlib
{

class EventLoop; // Forward declaration.

class Channel: public NonCopyable // A selectable I/O channel.
{
public:
	enum RequestedEventType
	{
		READ,
		NOT_READ,
		WRITE,
		NOT_WRITE,
		NONE
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
	EventLoop *owner_loop() const
	{
		return owner_loop_;
	}
	int fd() const
	{
		return fd_;
	}
	int requested_event() const
	{
		return requested_event_;
	}
	int state_in_epoller() const // Used by Epoller.
	{
		return state_in_epoller_;
	}

	// Setter: Since Channel's member functions can be invoked in only IO threads,
	// thus, we don't need set/release lock before/after updating data members.
	void set_requested_event(RequestedEventType type);
	void set_returned_event(int returned_event)
	{
		returned_event_ = returned_event;
	}
	void set_state_in_epoller(int new_state) // Used by Epoller.
	{
		state_in_epoller_ = new_state;
	}
	// Tie this channel to the owner object managed by shared_ptr,
	// prevent the owner object being destroyed in HandleEvent.
	void set_tie(const std::shared_ptr<void> &object);
	void set_event_callback(const EventCallback &callback, EventCallbackType type);

	bool IsRequestedReadEvent() const
	{
		return requested_event_ & kReadEvent;
	}
	bool IsRequestedWriteEvent() const
	{
		return requested_event_ & kWriteEvent;
	}
	bool IsRequestedNoneEvent() const
	{
		return requested_event_ == kNoneEvent;
	}

	// Call different callbacks based on returned_event_.
	void HandleEvent(TimeStamp receive_time);
	// Remove this channel from owner_loop_.
	void RemoveChannel();

	// For debug.
	std::string RequestedEventToString() const;
	std::string ReturnedEventToString() const;

private:
	static const int kNoneEvent; // 0
	static const int kReadEvent; // POLLIN, POLLPRI
	static const int kWriteEvent; // POLLOUT

	// -> EventLoop::AddOrUpdateChannel(Channel*) ->
	// Epoller::AddOrUpdateChannel(Channel*)
	// Add new Channel(O(logN)) or update already existing Channel(O(1))
	// in `map<int, Channel*> channel_set_`.
	void AddOrUpdateChannel();
	// Called by RequestedEventToString() and ReturnedEventToString().
	static std::string EventToString(int fd, int event);
	// Called by HandleEvent(TimeStamp receive_time);
	void HandleEventWithGuard(TimeStamp receive_time);

	// Every Channel object is owned by only one EventLoop object, so every Channel
	// object belongs to one IO thread. Channel object's life time is controlled by its
	// owner class, in this case, is the EventLoop class.
	EventLoop *owner_loop_;
	// Every Channel object is responsible for the dispatching of IO events of Only one
	// file descriptor(through different callbacks that denoted by std::function<>).
	// But it doesn't own this fd and doesn't close this fd when it destructs. The file
	// descriptor could be a socket, an event_fd, a timer_fd, or a signal_fd.
	const int fd_;
	int requested_event_; // Interested IO event, set by user.
	int returned_event_; // Active event, set by EventLoop::poller_.
	int state_in_epoller_; // This Channel object's state in the Epoller class.
	// Store TcpConnection object's shared_ptr. Set in `Channel::set_tie()`(called by
	// `TcpConnection::ConnectEstablished() {channel_->set_tie(shared_from_this());})
	// Used to judge whether the tied TcpConnection object is still alive in
	// HandEventWithGuard().
	std::weak_ptr<void> tie_;
	bool tied_;
	// Set in HandleEventWithGuard(); Used for assertion in Dtor().
	bool event_handling_;
	// Set in Update() and Remove(); Used for assertion in Dtor().
	bool added_to_loop_;
	// Different callbacks called when corresponding event happens.
	EventCallback read_callback_;
	EventCallback write_callback_;
	EventCallback close_callback_;
	EventCallback error_callback_;
};

}

#endif // NETLIB_NETLIB_CHANNEL_H_
