#ifndef NETLIB_NETLIB_EVENTLOOP_H_
#define NETLIB_NETLIB_EVENTLOOP_H_

#include <vector> // vector<>

#include <netlib/callback.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>
#include <netlib/timer_id.h>
#include <netlib/time_stamp.h>

namespace netlib
{

class Channel;
class Epoller;
class TimerQueue;

// Review:
// NonFunction: all
// Function:	CreateWakeupFd, Dtor#t_l_i_t_t_, Loop#looping_#a_c_v_
//						DoPendingFunctor#c_p_f_, Quit#if, *Channel#assert

// Interface:
// IgnoreSigPipe
// Ctor -> -CreateWakeupFd -> -HandleRead
// Dtor
// IsInLoopThread
// AssertInLoopThread -> +IsInLoopThread. Called in: Loop, *Channel.
// Loop -> +AssertInLoopThread -> -PrintActiveChannel -> -DoPendingFunctor
// Quit -> -Wakeup
// AddOrUpdateChannel -> +AssertInLoopThread.
// RemoveChannel -> +AssertInLoopThread.
// HasChannel -> +AssertInLoopThread.
// RunInLoop -> +QueueInLoop.
// QueueInLoop -> -Wakeup.
// RunAt.
// RunAfter.
// RunEvery.
// CancelTimer.

class EventLoop: public NonCopyable
{
public:
	using Functor = std::function<void()>;
	using FunctorVector = std::vector<Functor>;

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
	void CancelTimer(TimerId timer_id);

	// Invoke its epoller_'s AddOrUpdateChannel()
	void AddOrUpdateChannel(Channel *channel);
	void RemoveChannel(Channel *channel);
	bool HasChannel(Channel *channel);

	// Run callback immediately in the loop thread.
	// 1.	If called in the same loop thread: callback is run within the function.
	// 2.	If not in loop thread: wake up the loop, and run the callback.
	// Safe to call from other threads.
	void RunInLoop(const Functor&);
	// Queue callback in the loop thread. Run after finish pooling.
	// Safe to call from other threads.
	void QueueInLoop(const Functor&);

private:
	using ChannelVector = std::vector<Channel*>;

	// Create a wakeup_fd_ by calling `eventfd()`
	int CreateWakeupFd();
	void HandleRead();
	void DoPendingFunctor();
	// Wakeup the loop thread by writing to the wakeup_fd_.
	// Called: Quit(), QueueInLoop().
	void Wakeup();
	void PrintActiveChannel() const;

	bool looping_; // Atomic.
	bool quit_; // Atomic.
	const int thread_id_; // The loop thread's id.
	// Only one unique_ptr at a time can point to a given object. The object to which a
	// unique_ptr points is destroyed when the unique_ptr is destroyed.
	std::unique_ptr<Epoller> epoller_;
	ChannelVector active_channel_vector_;
	TimeStamp epoll_return_time_; // Time when poll returns, usually means data arrival.
	// Set in DoPendingFunctor(); Used for judge in QueueInLoop().
	MutexLock mutex_;
	// Since p_f_v_ is exposed to other threads, protect it by mutex_.
	FunctorVector pending_functor_vector_;
	bool calling_pending_functor_; // Atomic.
	int wakeup_fd_; // An eventfd. Closed in Dtor().
	std::unique_ptr<Channel> wakeup_fd_channel_;
	std::unique_ptr<TimerQueue> timer_queue_;
};

}
#endif // NETLIB_NETLIB_EVENTLOOP_H_
