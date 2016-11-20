#include <netlib/event_loop.h>

#include <assert.h> // assert()
#include <sys/eventfd.h> // eventfd()
#include <unistd.h> // read(), close(), write()
#include <stdint.h> // uint64_t
#include <signal.h> // signal()

#include <algorithm> // find()

#include <netlib/channel.h>
#include <netlib/logging.h>
#include <netlib/epoller.h>
#include <netlib/thread.h>
#include <netlib/timer_queue.h>

using std::vector;
using std::bind;
using netlib::EventLoop;
using netlib::Thread;
using netlib::TimerId;
using netlib::TimerCallback;
using netlib::TimeStamp;

class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};
IgnoreSigPipe ignore_sig_pipe_object;

// Every thread has its own instance of __thread variable.
__thread EventLoop *t_loop_in_this_thread = nullptr;
const int kTimeoutInMillisecond = 10 * 1000;

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	event_handling_(false),
	calling_pending_functor_(false),
	thread_id_(Thread::ThreadId()),
	epoller_(new Epoller(this)),
	timer_queue_(new TimerQueue(this)),
	wakeup_fd_(CreateWakeupFd()),
	wakeup_fd_channel_(new Channel(this, wakeup_fd_)),
	current_active_channel_(nullptr)
{
	LOG_DEBUG("EventLoop created %p in thread %d", this, thread_id_);
	// One loop per thread means that every thread can have only one EventLoop object.
	// If this thread already has another EventLoop object, abort this thread.
	if(t_loop_in_this_thread != nullptr)
	{
		LOG_FATAL("Another EventLoop %p exists in this thread %d",
		          t_loop_in_this_thread, thread_id_);
	}
	else
	{
		// The thread that creates EventLoop object is the loop thread,
		// whose main function is running EventLoop::Loop().
		t_loop_in_this_thread = this;
	}
	wakeup_fd_channel_->set_requested_event(Channel::READ_EVENT);
	wakeup_fd_channel_->set_event_callback(Channel::READ_CALLBACK,
	                                       bind(&EventLoop::HandleRead, this));
}
// int eventfd(unsigned int initval, int flags);
// eventfd() creates an "eventfd object" that can be used as an event wait/notify
// mechanism by user-space applications, and by the kernel to notify user-space
// applications of events. The object contains an unsigned 64-bit integer(uint64_t)
// counter that is maintained by the kernel. This counter is initialized with `initval`.
// EFD_CLOEXEC		Set close-on-exec(FD_CLOEXEC) flag on the returned fd.
// EFD_NONBLOCK	Set the O_NONBLOCK file status flag.
// Return a new file descriptor on success; -1 on error and errno is set.
int EventLoop::CreateWakeupFd()
{
	int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(event_fd == -1)
	{
		LOG_FATAL("Failed in eventfd");
	}
	return event_fd;
}
// The value returned by read(2) is in host byte order.
// 1.	If EFD_SEMAPHORE was not specified and the eventfd counter has a nonzero
//		value, then a read(2) returns 8 bytes containing that value, and the counter's
//		value is reset to zero.
// 2.	If the eventfd counter is zero, the call fails with the error EAGAIN if the file
//		descriptor has been made nonblocking.
void EventLoop::HandleRead()
{
	uint64_t counter = 0;
	int read_byte = static_cast<int>(::read(wakeup_fd_, &counter, 8));
	if(read_byte != 8)
	{
		LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", read_byte);
	}
}

EventLoop::~EventLoop()
{
	LOG_DEBUG("EventLoop %p of thread %d destructs in thread %d",
	          this, thread_id_, Thread::ThreadId());
	wakeup_fd_channel_->set_requested_event(Channel::NONE_EVENT);
	wakeup_fd_channel_->RemoveChannel();
	::close(wakeup_fd_);
	t_loop_in_this_thread = nullptr;
}

void EventLoop::AssertInLoopThread()
{
	if(IsInLoopThread() == false) // Must run Loop in IO thread.
	{
		LOG_FATAL("EventLoop %p was created in thread = %d, current thread = %d",
		          this, thread_id_, Thread::ThreadId());
	}
}

void EventLoop::Loop()
{
	// Check invariants by assert().
	assert(looping_ == false); // Not in looping.
	AssertInLoopThread(); // Must run Loop in IO thread.

	looping_ = true;
	quit_ = false; // FIXME: what if someone calls quit() before loop() ?
	LOG_TRACE("EventLoop %p start looping.", this);
	// Loop forever unless quit_ is set to `true` by current IO thread or other thread.
	while(quit_ == false)
	{
		active_channel_vector_.clear(); // Clear old active channel vector.
		poll_return_time_ =
		    epoller_->EpollWait(kTimeoutInMillisecond, active_channel_vector_);
		if(Logger::LogLevel() <= Logger::TRACE)
		{
			PrintActiveChannel();
		}
		// TODO sort channel by priority
		event_handling_ = true;
		for(ChannelVector::iterator it = active_channel_vector_.begin();
		        it != active_channel_vector_.end(); ++it)
		{
			current_active_channel_ = *it;
			current_active_channel_->HandleEvent(poll_return_time_);
		}
		current_active_channel_ = nullptr;
		event_handling_ = false;
		DoPendingFunctor(); // Do callbacks of pending_functor_vector_.
	}

	LOG_TRACE("EventLoop %p Stop looping.", this);
	looping_ = false;
}
void EventLoop::PrintActiveChannel() const
{
	for(ChannelVector::const_iterator it = active_channel_vector_.begin();
	        it != active_channel_vector_.end(); ++it)
	{
		LOG_TRACE("{%s}", (*it)->ReturnedEventToString().c_str());
	}
}
void EventLoop::DoPendingFunctor()
{
	vector<Functor> pending_functor; // Local variable.
	calling_pending_functor_ = true;

	// Critical Section: Swap this empty local vector and pending_functor_vector_.
	{
		MutexLockGuard lock(mutex_);
		pending_functor.swap(pending_functor_vector_);
	}
	// We don't run each functor in critical section, instead we only swap the
	// pending_functor_vector_ with the local variable pending_functor, and
	// run each functor outside of critical section by using the local variable
	// pending_functor. Reasons:
	// 1.	Shorten the length of critical section, so we won't block other threads calling
	//		QueueInLoop(). Since we use one mutex `mutex_` to guard
	//		`pending_functor_vector_`, when we are in this critical section, the mutex_ is
	//		locked, so the threads that calls QueueInLoop() will block in its critical section.
	// 2.	Avoid deadlock. Because the calling functor may call QueueInLoop() again, and
	//		in QueueInLoop(), we lock the mutex_ again, that is, we have locked a mutex,
	//		but we still request ourself mutex, this will cause a deadlock.

	int functor_number = static_cast<int>(pending_functor.size());
	for(int index = 0; index < functor_number; ++index)
	{
		pending_functor[index]();
	}
	// We don't repeat above loop until the pending_functor is empty, otherwise the loop
	// thread may go into infinite loop, can't handle IO events.
	calling_pending_functor_ = false;
}

// Quit() set quit_ to be true to terminate loop. But the actual quit happen when
// EventLoop::Loop() check `while(quit_ == false)`. If Quit() happens in other threads
// (not in IO thread), we wakeup the IO thread and it will check `while(quit_ == false)`,
// so it stops looping instantly.
void EventLoop::Quit()
{
	quit_ = true;
	// There is a chance that Loop() just executes `while(quit_ == true)` and exits,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using mutex_ in both places.
	if(IsInLoopThread() == false)
	{
		Wakeup(); // Wakeup IO thread when we want to quit in other threads.
	}
}

// Runs callback at `time_stamp`.
TimerId EventLoop::RunAt(const TimerCallback &callback, const TimeStamp &time)
{
	return timer_queue_->AddTimer(callback, time, 0.0);
}
// Run callback after `delay` seconds.
TimerId EventLoop::RunAfter(const TimerCallback &callback, double delay)
{
	return timer_queue_->AddTimer(callback,
	                              AddTime(TimeStamp::Now(), delay),
	                              0.0);
}
// Run callback every `interval` seconds.
TimerId EventLoop::RunEvery(const TimerCallback &callback, double interval)
{
	return timer_queue_->AddTimer(callback,
	                              AddTime(TimeStamp::Now(), interval),
	                              interval);
}

void EventLoop::AddOrUpdateChannel(Channel *channel)
{
	// Only can update channel that this EventLoop owns.
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	epoller_->AddOrUpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	// TODO: What's meaning?
	if(event_handling_ == true)
	{
		assert(current_active_channel_ == channel ||
		       find(active_channel_vector_.begin(),
		            active_channel_vector_.end(),
		            channel) == active_channel_vector_.end());
	}
	epoller_->RemoveChannel(channel);
}
bool EventLoop::HasChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	return epoller_->HasChannel(channel);
}

void EventLoop::RunInLoop(const Functor &callback)
{
	if(IsInLoopThread() == true)
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
		pending_functor_vector_.push_back(callback); // Add this functor to functor queue.
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
// fd is readable(select: readfds argument; poll: POLLIN flag; epoll: EPOLLIN flag)
// if the counter is greater than 0.
void EventLoop::Wakeup()
{
	uint64_t one = 1;
	int write_byte = static_cast<int>(::write(wakeup_fd_, &one, 8));
	if(write_byte != 8)
	{
		LOG_ERROR("EventLoop::Wakeup() write %d bytes instead of 8", write_byte);
	}
}

void EventLoop::CancelTimer(TimerId timer_id)
{
	timer_queue_->CancelTimer(timer_id);
}
