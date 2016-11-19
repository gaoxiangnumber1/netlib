#ifndef NETLIB_NETLIB_EVENTLOOP_H_
#define NETLIB_NETLIB_EVENTLOOP_H_

#include <vector> // vector<>

#include <netlib/callback.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>
#include <netlib/mutex.h>
#include <netlib/timer_id.h>

namespace netlib
{

class Channel;
class Epoller;
class TimerQueue;

// Interface:
// Ctor -> -CreateWakeupFd -> -HandleRead.
// Dtor.
// IsInLoopThread.
// AssertInLoopThread -> +IsInLoopThread. Called in: Loop, AOU/R/H-Channel.
// Loop -> +AssertInLoopThread -> -PrintActiveChannel -> -DoPendingFunctor.
// Quit.
// RunAt.
// RunAfter -> +RunAt.
// RunEvery.
// AddOrUpdateChannel -> +AssertInLoopThread.
// RemoveChannel -> +AssertInLoopThread.
// HasChannel -> +AssertInLoopThread.
// RunInLoop -> +QueueInLoop.
// QueueInLoop -> -Wakeup.
// CancelTimer.
class EventLoop: public NonCopyable
{
public:
	using Functor = std::function<void()>;

	EventLoop(); // Check whether satisfy `one loop per thread`.
	~EventLoop(); // Force out-line dtor, for unique_ptr members.

	void Quit();

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

	// Invoke its epoller_'s AddOrUpdateChannel()
	void AddOrUpdateChannel(Channel *channel);
	void RemoveChannel(Channel *channel);
	bool HasChannel(Channel *channel);

	// Run callback immediately in the loop thread.
	// 1.	If called in the same loop thread: callback is run within the function.
	// 2.	If not in loop thread: wake up the loop, and run the callback.
	// Safe to call from other threads.
	void RunInLoop(const Functor &callback);
	// Queue callback in the loop thread. Run after finish pooling.
	// Safe to call from other threads.
	void QueueInLoop(const Functor &callback);

	void CancelTimer(TimerId timer_id);

private:
	using ChannelVector = std::vector<Channel*>;

	// Create a wakeup_fd_ by calling `eventfd()`
	int CreateWakeupFd();
	void HandleRead();
	void DoPendingFunctor();
	// Wakeup the IO thread by writing to the wakeup_fd_.
	void Wakeup();
	void PrintActiveChannel() const;

	bool looping_; // Atomic.
	bool quit_; // Atomic.
	// Used for assertion in RemoveChannel() which is called by: TimerQueue::Dtor() ->
	// Channel::RemoveChannel() -> EventLoop::RemoveChannel().
	bool event_handling_;
	// Set in DoPendingFunctor(); Used in QueueInLoop().
	bool calling_pending_functor_; // Atomic.
	const int thread_id_; // The loop thread's id.
	TimeStamp poll_return_time_; // Time when poll returns, usually means data arrival.
	// Only one unique_ptr at a time can point to a given object. The object to which a
	// unique_ptr points is destroyed when the unique_ptr is destroyed.
	std::unique_ptr<Epoller> epoller_;
	std::unique_ptr<TimerQueue> timer_queue_;
	int wakeup_fd_; // An eventfd.
	// wakeup_fd_channel_ monitor the IO events(readable) of wakeup_fd_,
	// and dispatch these IO events to HandleRead().
	std::unique_ptr<Channel> wakeup_fd_channel_;
	ChannelVector active_channel_vector_;
	Channel *current_active_channel_; // Used for assertion.
	MutexLock mutex_; // Encapsulate pthread_mutex_t.
	// Since p_f_v_ is exposed to other threads, protect it by mutex_.
	std::vector<Functor> pending_functor_vector_;
};

}
#endif // NETLIB_NETLIB_EVENTLOOP_H_
