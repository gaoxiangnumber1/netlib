#ifndef NETLIB_NETLIB_CHANNEL_H_
#define NETLIB_NETLIB_CHANNEL_H_

#include <netlib/callback.h> // EventCallback type alias.
#include <netlib/non_copyable.h> // NonCopyable class

namespace netlib
{

class EventLoop; // Forward declaration.

class Channel: public NonCopyable // A selectable I/O channel.
{
public:
	using ReadEventCallback = std::function<void(TimeStamp)>;

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
	void set_requested_event_read() // Set requested events for IO read.
	{
		requested_event_ |= kReadEvent;
		Update();
	}
	void set_requested_event_write()
	{
		requested_event_ |= kWriteEvent;
		Update();
	}
	void set_requested_event_not_write()
	{
		requested_event_ &= ~kWriteEvent;
		Update();
	}
	void set_requested_event_none()
	{
		requested_event_ = kNoneEvent;
		Update();
	}
	bool IsWriting() const
	{
		return requested_event_ & kWriteEvent;
	}
	void set_returned_event(int returned_event)
	{
		returned_event_ = returned_event;
	}
	void set_state_in_epoller(int new_state) // Used by Epoller.
	{
		state_in_epoller_ = new_state;
	}
	void set_read_callback(const ReadEventCallback &callback)
	{
		read_callback_ = callback;
	}
	void set_write_callback(const EventCallback &callback)
	{
		write_callback_ = callback;
	}
	void set_close_callback(const EventCallback &callback)
	{
		close_callback_ = callback;
	}
	void set_error_callback(const EventCallback &callback)
	{
		error_callback_ = callback;
	}
	// Call different callbacks based on returned_event_.
	void HandleEvent(TimeStamp receive_time);
	bool IsNoneRequestedEvent() const
	{
		return requested_event_ == kNoneEvent;
	}
	char *RequestedEventToString();

private:
	static const int kNoneEvent; // 0
	static const int kReadEvent; // POLLIN, POLLPRI
	static const int kWriteEvent; // POLLOUT

	// -> EventLoop::UpdateChannel(Channel*) -> Epoller::UpdateChannel(Channel*)
	// Add new Channel(O(logN)) or update already existing Channel(O(1))
	// in `vector<struct pollfd> pollfd_vector_`
	void Update();

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
	bool event_handling_; // Whether is handing event? If it is, we shouldn't destruct.
	int state_in_epoller_; // This Channel object's state in the Epoller class.
	// Different callbacks called when corresponding event happens.
	ReadEventCallback read_callback_;
	EventCallback write_callback_;
	EventCallback close_callback_;
	EventCallback error_callback_;
};

}

#endif // NETLIB_NETLIB_CHANNEL_H_
