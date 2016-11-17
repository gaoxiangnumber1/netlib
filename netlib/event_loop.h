#ifndef NETLIB_NETLIB_EVENTLOOP_H_
#define NETLIB_NETLIB_EVENTLOOP_H_

#include <sys/types.h> // pid_t

#include <memory> // unique_ptr<>
#include <vector> // vector<>

#include <netlib/callback.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>
#include <netlib/timer_id.h>
#include <netlib/mutex.h>
#include <netlib/timer_id.h>

namespace netlib
{

class Channel;
class Epoller;
class TimerQueue;

class EventLoop: public NonCopyable
{
public:
	using Functor = std::function<void()>;

	EventLoop(); // Check whether satisfy `one loop per thread`.
	~EventLoop(); // Force out-line dtor, for unique_ptr members.

	// Getter. Return the object that hold by current thread.
	static EventLoop *GetEventLoopOfCurrentThread();
	TimeStamp poll_return_time() const
	{
		return poll_return_time_;
	}
	// Setter.
	void Quit(); // set_quit(true);

	void AssertInLoopThread();
	// Loop forever. Must be called in the same thread as creation of the object.
	bool IsInLoopThread() const
	{
		return thread_id_ == Thread::ThreadId();
	}
	void Loop();

	// Runs callback at `time_stamp`. Safe to call from other threads.
	TimerId RunAt(const TimerCallback &callback, const TimeStamp &time_stamp);
	// Run callback after `delay` seconds. Safe to call from other threads.
	TimerId RunAfter(const TimerCallback &callback, double delay);
	// Run callback every `interval` seconds. Safe to call from other threads.
	TimerId RunEvery(const TimerCallback &callback, double interval);

	// Invoke its poller_'s AddOrUpdateChannel()
	void AddOrUpdateChannel(Channel *channel);
	void RemoveChannel(Channel *channel);
	bool HasChannel(Channel *channel);

	// Create a wakeup_fd_ by calling `eventfd()`
	int CreateWakeupFd();
	// Wakeup the IO thread by writing to the wakeup_fd_.
	void Wakeup();

	// Run callback immediately in the loop thread.
	// 1.	If called in the same loop thread: callback is run within the function.
	// 2.	If not in loop thread: wake up the loop, and run the callback.
	// Safe to call from other threads.
	void RunInLoop(const Functor &callback);
	// Queue callback in the loop thread. Run after finish pooling.
	// Safe to call from other threads.
	void QueueInLoop(const Functor &callback);

	void Cancel(TimerId timer_id);

private:
	using ChannelVector = std::vector<Channel*>;

	void DoPendingFunctor();
	void HandleWakeupFd();

	bool looping_; // Atomic.
	bool quit_; // Atomic.
	const pid_t thread_id_;
	// Only one unique_ptr at a time can point to a given object. The object to which a
	// unique_ptr points is destroyed when the unique_ptr is destroyed.
	TimeStamp poll_return_time_; // Time when poll returns, usually means data arrival.
	std::unique_ptr<Epoller> poller_;
	ChannelVector active_channel_;
	std::unique_ptr<TimerQueue> timer_queue_;
	int wakeup_fd_; // A Linux eventfd.
	// wakeup_fd_channel_ monitors the IO events(readable) of wakeup_fd_,
	// and dispatch these IO events to HandleWakeupFd().
	std::unique_ptr<Channel> wakeup_fd_channel_;
	MutexLock mutex_; // An object encapsulates pthread_mutex_t.
	// Only pending_functor_ is exposed to other threads, so we protect it by mutex_.
	std::vector<Functor> pending_functor_; // Guard by mutex_.
	bool calling_pending_functor_; // Atomic.
};

}
#endif // NETLIB_NETLIB_EVENTLOOP_H_
