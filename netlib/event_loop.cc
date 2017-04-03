#include <netlib/event_loop.h>

#include <assert.h> // assert()
#include <sys/eventfd.h> // eventfd()
#include <unistd.h> // read(), close(), write()
#include <stdint.h> // uint64_t
#include <signal.h> // signal()

#include <netlib/channel.h>
#include <netlib/logging.h>
#include <netlib/epoller.h>
#include <netlib/thread.h>
#include <netlib/timer_queue.h>

using std::bind;
using netlib::EventLoop;
using netlib::Thread;
using netlib::TimerId;
using netlib::TimerCallback;
using netlib::TimeStamp;

struct IgnoreSigPipe
{
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN); // signal() is non-portable. Use sigaction(2) instead.
	}
};
IgnoreSigPipe ignore_sig_pipe_object;

__thread EventLoop *t_loop_in_this_thread = nullptr;
EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	thread_id_(Thread::ThreadId()),
	epoller_(new Epoller(this)),
	epoll_return_time_(),
	timer_queue_(new TimerQueue(this)),
	event_fd_(CreateEventFd()),
	event_fd_channel_(new Channel(this, event_fd_)),
	mutex_(),
	doing_task_callback_(false)
{
	LOG_DEBUG("EventLoop created %p in thread %d", this, thread_id_);
	// One loop per thread: every thread can have only one EventLoop object.
	if(t_loop_in_this_thread != nullptr)
	{
		LOG_FATAL("Another EventLoop %p exists in this thread %d",
		          t_loop_in_this_thread, thread_id_);
	}
	t_loop_in_this_thread = this;
	event_fd_channel_->set_event_callback(Channel::READ_CALLBACK,
	                                      bind(&EventLoop::HandleRead, this));
	event_fd_channel_->set_requested_event(Channel::READ_EVENT);
}
int EventLoop::CreateEventFd()
{
	int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(event_fd < 0)
	{
		LOG_FATAL("eventfd(): FATAL");
	}
	return event_fd;
}
void EventLoop::HandleRead()
{
	uint64_t value;
	int read_byte = static_cast<int>(::read(event_fd_, &value, sizeof value));
	if(read_byte != sizeof value)
	{
		LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", read_byte);
	}
}

EventLoop::~EventLoop()
{
	assert(looping_ == false);
	LOG_DEBUG("EventLoop %p of thread %d destructs in thread %d",
	          this, thread_id_, Thread::ThreadId());

	t_loop_in_this_thread = nullptr;
	event_fd_channel_->set_requested_event(Channel::NONE_EVENT);
	event_fd_channel_->RemoveChannel();
	::close(event_fd_);
}

void EventLoop::AssertInLoopThread()
{
	if(IsInLoopThread() == false)
	{
		LOG_FATAL("EventLoop %p was created in thread = %d, current thread = %d",
		          this, thread_id_, Thread::ThreadId());
	}
}

void EventLoop::RunInLoop(const TaskCallback &task_callback)
{
	if(IsInLoopThread() == true)
	{
		task_callback();
	}
	else
	{
		QueueInLoop(task_callback);
	}
}
void EventLoop::QueueInLoop(const TaskCallback &task_callback)
{
	{
		MutexLockGuard lock(mutex_);
		task_callback_vector_.push_back(task_callback);
	}
	if(IsInLoopThread() == false || doing_task_callback_ == true)
	{
		Wakeup();
	}
}
void EventLoop::Wakeup()
{
	uint64_t one = 1;
	int write_byte = static_cast<int>(::write(event_fd_, &one, sizeof one));
	if(write_byte != sizeof one)
	{
		LOG_ERROR("EventLoop::Wakeup() write %d bytes instead of 8", write_byte);
	}
}

TimerId EventLoop::RunAt(const TimerCallback &callback, const TimeStamp &time)
{
	return timer_queue_->AddTimer(callback, time, 0.0);
}
TimerId EventLoop::RunAfter(const TimerCallback &callback, double delay)
{
	return timer_queue_->AddTimer(callback,
	                              AddTime(TimeStamp::Now(), delay),
	                              0.0);
}
TimerId EventLoop::RunEvery(const TimerCallback &callback, double interval)
{
	return timer_queue_->AddTimer(callback,
	                              AddTime(TimeStamp::Now(), interval),
	                              interval);
}
void EventLoop::CancelTimer(const TimerId &timer_id)
{
	timer_queue_->CancelTimer(timer_id);
}

void EventLoop::AddOrUpdateChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	epoller_->AddOrUpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	epoller_->RemoveChannel(channel);
}
bool EventLoop::HasChannel(Channel *channel)
{
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	return epoller_->HasChannel(channel);
}

void EventLoop::Loop()
{
	AssertInLoopThread();
	assert(looping_ == false);
	LOG_TRACE("EventLoop %p start looping.", this);

	looping_ = true;
	while(quit_ == false)
	{
		active_channel_vector_.clear();
		epoll_return_time_ = epoller_->EpollWait(-1, active_channel_vector_);
		PrintActiveChannel();
		for(ChannelVector::iterator it = active_channel_vector_.begin();
		        it != active_channel_vector_.end();
		        ++it)
		{
			(*it)->HandleEvent(epoll_return_time_);
		}
		DoTaskCallback();
	}
	looping_ = false;

	LOG_TRACE("EventLoop %p Stop looping.", this);
}
void EventLoop::PrintActiveChannel() const
{
	for(ChannelVector::const_iterator it = active_channel_vector_.begin();
	        it != active_channel_vector_.end();
	        ++it)
	{
		LOG_TRACE("{%s}", (*it)->ReturnedEventToString().c_str());
	}
}
void EventLoop::DoTaskCallback()
{
	doing_task_callback_ = true;
	TaskCallbackVector local_task_callback_vector;
	{
		MutexLockGuard lock(mutex_);
		local_task_callback_vector.swap(task_callback_vector_);
	}
	for(TaskCallbackVector::iterator it = local_task_callback_vector.begin();
	        it != local_task_callback_vector.end();
	        ++it)
	{
		(*it)();
	}
	doing_task_callback_ = false;
}

void EventLoop::Quit()
{
	quit_ = true;
	if(IsInLoopThread() == false)
	{
		Wakeup();
	}
}
