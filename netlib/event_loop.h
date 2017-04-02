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

// Interface:
// IgnoreSigPipe
// Ctor -> -CreateEventFd -> -HandleRead
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

	EventLoop();
	~EventLoop(); // Force outline dtor, for unique_ptr members.

	void Quit();
	void AssertInLoopThread();
	bool IsInLoopThread() const
	{
		return thread_id_ == Thread::ThreadId();
	}
	void Loop();

	// Run callback at `time_stamp`. Safe to call from other threads.
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

	int CreateEventFd();
	void HandleRead();
	void DoPendingFunctor();
	void Wakeup();
	void PrintActiveChannel() const;

	bool looping_; // FIXME: Atomic.
	bool quit_; // FIXME: Atomic.
	const int thread_id_; // TID of thread that creates this EventLoop object.
	std::unique_ptr<Epoller> epoller_;
	ChannelVector active_channel_vector_;
	TimeStamp epoll_return_time_;
	MutexLock mutex_;
	FunctorVector pending_functor_vector_; // Guarded by mutex_.
	bool calling_pending_functor_; // FIXME: Atomic.
	int event_fd_;
	std::unique_ptr<Channel> event_fd_channel_;
	std::unique_ptr<TimerQueue> timer_queue_;
};

}
#endif // NETLIB_NETLIB_EVENTLOOP_H_
