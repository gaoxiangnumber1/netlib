#ifndef NETLIB_SRC_EVENTLOOP_H_
#define NETLIB_SRC_EVENTLOOP_H_

#include <sys/types.h> // pid_t

#include <memory> // unique_ptr<>
#include <vector> // vector<>

#include <callback.h>
#include <non_copyable.h>
#include <time_stamp.h>
#include <timer_id.h>

namespace netlib
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop: public NonCopyable
{
public:
	EventLoop(); // Check whether satisfy `one loop per thread`.
	~EventLoop(); // Force out-line dtor, for unique_ptr members.

	// Getter. Return the object that hold by current thread.
	static EventLoop *GetEventLoopOfCurrentThread();
	TimeStamp poll_return_time() const
	{
		return poll_return_time_;
	}
	// Setter.
	void set_quit(bool quit);

	void AssertInLoopThread();
	// Loop forever. Must be called in the same thread as creation of the object.
	void Loop();
	// Runs callback at `time_stamp`.
	TimerId RunAt(const TimerCallback &callback, const TimeStamp &time_stamp);
	// Run callback after `delay` seconds.
	TimerId RunAfter(const TimerCallback &callback, double delay);
	// Run callback every `interval` seconds.
	TimerId RunEvery(const TimerCallback &callback, double interval);
	// Internal use only
	void UpdateChannel(Channel *channel);

private:
	using ChannelVector = std::vector<Channel*>;

	bool looping_;
	bool quit_;
	const pid_t thread_id_;
	// Only one unique_ptr at a time can point to a given object. The object to which a
	// unique_ptr points is destroyed when the unique_ptr is destroyed.
	TimeStamp poll_return_time_; // Time when poll returns, usually means data arrival.
	ChannelVector active_channel_;
	std::unique_ptr<Poller> poller_;
	std::unique_ptr<TimerQueue> timer_queue_;
};

}
#endif // NETLIB_SRC_EVENTLOOP_H_
