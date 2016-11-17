#include <netlib/event_loop.h>

#include <assert.h> // assert()
#include <poll.h> // poll()
#include <sys/eventfd.h> // eventfd()
#include <unistd.h> // read(), close(), write()
#include <stdint.h> // uint64_t
#include <signal.h> // signal()

#include <netlib/channel.h>
#include <netlib/logging.h>
#include <netlib/poller.h>
#include <netlib/thread.h>
#include <netlib/timer_queue.h>

using std::vector;
using std::bind;
using netlib::EventLoop;
using netlib::Thread;
using netlib::TimerId;
using netlib::TimerCallback;
using netlib::TimerQueue;
using netlib::TimeStamp;

// Every thread has its own instance of __thread variable.
__thread EventLoop *t_loop_in_this_thread = nullptr;
const int kPollTimeout = 10 * 1000;

class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigPipe ignore_sig_pipe_object;

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	thread_id_(Thread::ThreadId()),
	poller_(new Epoller(this)), // Set poller_'s owner_loop_ to this EventLoop.
	timer_queue_(new TimerQueue(this)),
	wakeup_fd_(CreateWakeupFd()),
	wakeup_fd_channel_(new Channel(this, wakeup_fd_)),
	calling_pending_functor_(false)
{
//	LOG_INFO("EventLoop created %p in thread %d", this, thread_id_);
	// One loop per thread means that every thread can have only one EventLoop object.
	// If this thread already has another EventLoop object, abort this thread.
	if(t_loop_in_this_thread != nullptr)
	{
		LOG_FATAL("Another EventLoop %p exists in this thread %d",
		          t_loop_in_this_thread, thread_id_);
	}
	else
	{
		// The thread that creates EventLoop object is the IO thread,
		// whose main function is running EventLoop::Loop().
		t_loop_in_this_thread = this;
	}
	wakeup_fd_channel_->set_requested_event_read();
	wakeup_fd_channel_->set_read_callback(bind(&EventLoop::HandleWakeupFd, this));
}

EventLoop::~EventLoop()
{
	assert(looping_ == false);
	::close(wakeup_fd_);
	t_loop_in_this_thread = nullptr;
}

// Return the object that hold by current thread.
// nullptr means current thread is not the IO thread.
EventLoop *EventLoop::GetEventLoopOfCurrentThread() // static member function.
{
	return t_loop_in_this_thread;
}

// Quit() set quit_ to be true to terminate loop. But the actual quit happen when
// EventLoop::Loop() check `while(quit_ == false)`. If Quit() happens in other threads
// (not in IO thread), we wakeup the IO thread and it will check `while(quit_ == false)`,
// so it stops looping instantly.
void EventLoop::Quit()
{
	quit_ = true;
	if(IsInLoopThread() == false)
	{
		Wakeup(); // Wakeup IO thread when we want to quit in other threads.
	}
}

void EventLoop::AssertInLoopThread()
{
	if(IsInLoopThread() == false) // Must run Loop in IO thread.
	{
		LOG_FATAL("EventLoop %p was created in tid = %d, current tid = %d",
		          this, thread_id_, Thread::ThreadId());
	}
}

void EventLoop::Loop()
{
	// Check invariants by assert().
	assert(looping_ == false); // Not in looping.
	AssertInLoopThread(); // Must run Loop in IO thread.

	looping_ = true;
	quit_ = false;
	// Loop forever unless quit_ is set to `true` by current IO thread or other thread.
	while(quit_ == false)
	{
		active_channel_.clear(); // Clear old active channel vector.
		// Invoke ::poll() to get the number of active IO events and
		// invoke FillActiveChannel to fill active_channel_.
		poll_return_time_ = poller_->Poll(kPollTimeout, &active_channel_);
		// Now, each channel in active_channel_ has IO events happened.
		for(ChannelVector::iterator it = active_channel_.begin();
		        it != active_channel_.end(); ++it)
		{
			(*it)->HandleEvent(poll_return_time_);
		}
		DoPendingFunctor(); // Do callbacks of pending_functor_.
	}

	LOG_INFO("EventLoop %p Stop looping.", this);
	looping_ = false;
}

// Runs callback at `time_stamp`.
TimerId EventLoop::RunAt(const TimerCallback &callback,
                         const TimeStamp &time_stamp)
{
	// TimerId AddTimer(const TimerCallback &callback,
	//                  TimeStamp expired_time,
	//                  double interval);
	return timer_queue_->AddTimer(callback, time_stamp, 0.0);
}

// Run callback after `delay` seconds.
TimerId EventLoop::RunAfter(const TimerCallback &callback, double delay)
{
	TimeStamp time_stamp(AddTime(TimeStamp::Now(), delay));
	return RunAt(callback, time_stamp);
}

// Run callback every `interval` seconds.
TimerId EventLoop::RunEvery(const TimerCallback &callback, double interval)
{
	TimeStamp time_stamp(AddTime(TimeStamp::Now(), interval));
	return timer_queue_->AddTimer(callback, time_stamp, interval);
}

void EventLoop::AddOrUpdateChannel(Channel *channel)
{
	// Only can update channel that this EventLoop owns.
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	poller_->AddOrUpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	poller_->RemoveChannel(channel);
}

int EventLoop::CreateWakeupFd() // Create a wakeup_fd_ by calling `eventfd()`
{
	// int eventfd(unsigned int initval, int flags);
	// eventfd() creates an "eventfd object" that can be used as an event wait/notify
	// mechanism by user-space applications, and by the kernel to notify user-space
	// applications of events. The object contains an unsigned 64-bit integer(uint64_t)
	// counter that is maintained by the kernel. This counter is initialized with the
	// value specified in the argument initval.
	// Return a new eventfd file descriptor on success; -1 on error and errno is set.
	int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (event_fd < 0)
	{
		LOG_FATAL("Failed in eventfd");
	}
	return event_fd;
}

// Wakeup the IO thread by writing to the wakeup_fd_.
void EventLoop::Wakeup()
{
	uint64_t one = 1;
	int written = static_cast<int>(::write(wakeup_fd_, &one, 8));
	if(written != 8)
	{
		LOG_INFO("EventLoop::Wakeup() write %d bytes instead of 8", written);
	}
}

void EventLoop::RunInLoop(const Functor &callback)
{
	if(IsInLoopThread())
	{
		callback();
	}
	else
	{
		QueueInLoop(callback);
	}
}

void EventLoop::QueueInLoop(const Functor &callback)
{
	{
		// lock is a stack variable, MutexLockGuard constructor calls `mutex_.Lock()`.
		MutexLockGuard lock(mutex_);
		pending_functor_.push_back(callback); // Add this functor to functor queue.
		// lock is about to destruct, its destructor calls `mutex_.Unlock()`.
	}
	// Wakeup IO thread when either of following conditions satisfy:
	//		1. This thread(i.e., the calling thread) is not the IO thread.
	//		2. This thread is IO thread, but now it is calling pending functor.
	//			`calling_pending_functor_` is true only in DoPendingFunctor() when
	//			we call each pending functor. When the pending functor calls QueueInLoop()
	//			again, we must Wakeup() IO thread, otherwise the newly added callbacks
	//			won't be called on time.
	// That is, only calling QueueInLoop in the EventCallback(s) of IO thread, can
	// we not Wakeup IO thread.
	if(IsInLoopThread() == false || calling_pending_functor_ == true)
	{
		Wakeup();
	}
}

void EventLoop::DoPendingFunctor()
{
	vector<Functor> pending_functor; // Local variable.
	calling_pending_functor_ = true;

	// Critical Section: Swap this empty local vector and pending_functor_.
	{
		MutexLockGuard lock(mutex_);
		pending_functor.swap(pending_functor_);
	}
	// We don't run each functor in critical section, instead we only swap the
	// pending_functor_ with the local variable pending_functor, and run each functor
	// outside of critical section by using the local variable pending_functor. Reasons:
	// 1.	Shorten the length of critical section, so we won't block other threads calling
	//		QueueInLoop(). Note that we use only one mutex `mutex_` to guard
	//		`pending_functor_`, when we are in this critical section, the mutex_ is locked,
	//		so the threads that calls QueueInLoop() will block in its critical section.
	// 2.	Avoid deadlock. Because the calling functor may call QueueInLoop() again, and
	//		in QueueInLoop(), we lock the mutex_ again, that is, we have locked a mutex,
	//		but we still request ourself mutex, this will cause a deadlock.

	int functor_number = static_cast<int>(pending_functor.size());
	for(int index = 0; index < functor_number; ++index)
	{
		pending_functor[index]();
	}
	// We don't repeat above loop until the pending_functor is empty, otherwise the IO
	// thread may go into infinite loop, can't handle IO events.
	calling_pending_functor_ = false;
}

void EventLoop::HandleWakeupFd()
{
	// The value returned by read(2) is in host byte order.
	// 1.	If EFD_SEMAPHORE was not specified and the eventfd counter has a nonzero
	//		value, then a read(2) returns 8 bytes containing that value, and the counter's
	//		value is reset to zero.
	// 2.	If the eventfd counter is zero, the call fails with the error EAGAIN if the file
	//		descriptor has been made nonblocking.
	uint64_t counter = 0;
	int readn = static_cast<int>(::read(wakeup_fd_, &counter, 8));
	if(readn != 8)
	{
		LOG_INFO("EventLoop::HandleWakeupFd() reads %d bytes instead of 8", readn);
	}
}

void EventLoop::Cancel(TimerId timer_id)
{
	(*timer_queue_).Cancel(timer_id);
}
