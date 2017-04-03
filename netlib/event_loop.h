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
// AssertInLoopThread -> +IsInLoopThread.
// RunInLoop -> +QueueInLoop.
// QueueInLoop -> -Wakeup.
// RunAt.
// RunAfter.
// RunEvery.
// CancelTimer.
// AddOrUpdateChannel -> +AssertInLoopThread.
// RemoveChannel -> +AssertInLoopThread.
// HasChannel -> +AssertInLoopThread.
// Loop -> +AssertInLoopThread -> -PrintActiveChannel -> -DoTaskCallback
// Quit -> -Wakeup

class EventLoop: public NonCopyable
{
public:
	using TaskCallback = std::function<void()>;
	using TaskCallbackVector = std::vector<TaskCallback>;

	EventLoop();
	~EventLoop(); // Force outline dtor, for unique_ptr members.
	void AssertInLoopThread();
	bool IsInLoopThread() const
	{
		return thread_id_ == Thread::ThreadId();
	}

	void RunInLoop(const TaskCallback&);
	void QueueInLoop(const TaskCallback&);

	TimerId RunAt(const TimerCallback &callback, const TimeStamp &time_stamp);
	TimerId RunAfter(const TimerCallback &callback, double delay);
	TimerId RunEvery(const TimerCallback &callback, double interval);
	void CancelTimer(const TimerId &timer_id);

	void AddOrUpdateChannel(Channel *channel);
	void RemoveChannel(Channel *channel);
	bool HasChannel(Channel *channel);

	void Loop();
	void Quit();

private:
	using ChannelVector = std::vector<Channel*>;

	int CreateEventFd();
	void HandleRead();
	void Wakeup();
	void PrintActiveChannel() const;
	void DoTaskCallback();

	bool looping_; // FIXME: Atomic.
	bool quit_; // FIXME: Atomic.
	const int thread_id_; // TID of thread that creates this EventLoop object.
	std::unique_ptr<Epoller> epoller_;
	ChannelVector active_channel_vector_;
	TimeStamp epoll_return_time_;
	std::unique_ptr<TimerQueue> timer_queue_;
	int event_fd_;
	std::unique_ptr<Channel> event_fd_channel_;
	MutexLock mutex_;
	TaskCallbackVector task_callback_vector_; // Guarded by mutex_.
	bool doing_task_callback_; // FIXME: Atomic.
};

}
#endif // NETLIB_NETLIB_EVENTLOOP_H_
