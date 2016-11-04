#include <timer_queue.h>

#include <assert.h>
#include <string.h> // strerror()
#include <sys/timerfd.h> // timerfd_*
#include <unistd.h> // close(), read()

#include <algorithm>

#include <channel.h>
#include <event_loop.h>
#include <logging.h>
#include <timer.h>
#include <timer_id.h>

using std::bind;
using std::copy;
using std::make_pair;
using std::pair;
using std::unique_ptr;
using std::vector;
using netlib::Channel;
using netlib::EventLoop;
using netlib::TimerId;
using netlib::TimerQueue;

namespace netlib
{

namespace detail
{

// Create a new timer fd. Called by TimerQueue::TimerQueue(EventLoop *loop).
int CreateTimerFd()
{
	int timer_fd = ::timerfd_create(CLOCK_MONOTONIC,
	                                TFD_NONBLOCK | TFD_CLOEXEC);
	if(timer_fd < 0)
	{
		LOG_FATAL("CreateTimerFd() failed.");
	}
	return timer_fd;
}

// Call ::read to read from `timer_fd` at `time_stamp` time.
void ReadTimerFd(int timer_fd, TimeStamp time_stamp)
{
	// If the timer has expired one or more times since its settings were last modified using
	// timerfd_settime(), or since the last successful read(2), then the buffer given to read(2)
	// returns an unsigned 8-byte integer(uint64_t) containing the number of expirations
	// that have occurred. The returned value is in host byte order.
	uint64_t expiration_number = 0;
	int readn = static_cast<int>(::read(timer_fd, &expiration_number, 8));
//	LOG_INFO("expiration number = %d, time = %s",
//	         static_cast<int>(expiration_number), time_stamp.ToString());
	if(readn != 8)
	{
		LOG_INFO("TimerQueue::ReadCallback read %d bytes instead of 8.", readn);
	}
}

}

}

TimerQueue::TimerQueue(EventLoop *owner_loop):
	owner_loop_(owner_loop),
	timer_fd_(netlib::detail::CreateTimerFd()), // Create a new timer.
	timer_fd_channel_(owner_loop, timer_fd_),
	expired_timer_(), // set<> container.
	timer_pair_set_()
{
	timer_fd_channel_.set_requested_event_read(); // Monitor IO read event.
	timer_fd_channel_.set_read_callback(bind(&TimerQueue::ReadCallback, this));
}

TimerQueue::~TimerQueue()
{
	::close(timer_fd_);
	// TODO: Do not remove channel, since we're in EventLoop::dtor();
	// TODO: If we don't use shared_ptr(since it cost too much) or unique_ptr
	// (since the const property of set element), we should use what to avoid
	// `delete Timer*;` by ourself?
	for(TimerPairSet::iterator it = timer_pair_set_.begin();
	        it != timer_pair_set_.end(); ++it)
	{
		delete it->second;
		// it->second = nullptr; error: assignment of member ‘pair<TimeStamp, Timer*>
		// ::second’ in read-only object
	}
}

// Construct a new timer based on the arguments; Insert it to timer set;
// Return a TimerId object that encapsulates this timer.
TimerId TimerQueue::AddTimer(const TimerCallback &callback,
                             TimeStamp expired_time,
                             double interval)
{
	owner_loop_->AssertInLoopThread();

	// 1. Create a Timer object based on arguments.
	Timer *timer = new Timer(callback, expired_time, interval);
	// 2. Insert this timer into timer set. Return true if this timer will expire first.
	bool is_first_expired = InsertIntoTimerPairSet(timer);
	// 3. If this timer will expire first, update timer_fd_'s expiration time.
	if(is_first_expired)
	{
		LOG_INFO("SET: AddTimer");
		SetExpirationTime(timer->expiration());
	}
	// 4. Return the inserted timer as a TimerId object.
	return TimerId(timer);
}

// Get the expired timers relative to `now` and store them in expired_time_ vector.
void TimerQueue::GetExpiredTimer(TimeStamp now)
{
	// 1. Clear the old expired timer vector.
	expired_timer_.clear();

	// 2. Set sentry to search the set and get the first not expired timer iterator.
	// sentry is the biggest timer-pair whose time-stamp value is `now`.
	TimerPair sentry = make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	TimerPairSet::iterator first_not_expired = timer_pair_set_.lower_bound(sentry);
	//(1)	Not find the iterator that we can insert sentry into timer set, that is, all timers
	//		in timer set are already expired.
	//(2)	If can find, the first not expired timer's expiration time must be greater than
	//		`now` since sentry is the biggest timer whose expiration time is `now`.
	assert(first_not_expired == timer_pair_set_.end() || now < first_not_expired->first);

	// 3. Copy all the expired timer's pointer from timer_pair_set_ to expired_timer_.
	for(TimerPairSet::iterator it = timer_pair_set_.begin(); it != first_not_expired; ++it)
	{
		expired_timer_.push_back(it->second); // Pass by value: copy a pointer.
	}

	// 4. Erase the expired timers in the timer_pair_set_, this don't destruct the timer.
	// TODO: what happens when set.erase(iterator)?
	timer_pair_set_.erase(timer_pair_set_.begin(), first_not_expired);
}

// Insert the specified timer into timer set. Return true if this timer will expire first.
bool TimerQueue::InsertIntoTimerPairSet(Timer *timer)
{
	bool is_first_expired = false;
	TimeStamp expired_time = timer->expiration();
	TimerPairSet::iterator it = timer_pair_set_.begin();
	// When one of the following conditions satisfies, this timer expires first:
	// 1. Timer set has no timer.
	// 2. This timer's expiration time is less than the smallest expiration time in timer set.
	if(it == timer_pair_set_.end() || expired_time < it->first)
	{
		is_first_expired = true;
	}
	// Make a new pair for this timer and insert it to the timer set.
	pair<TimerPairSet::iterator, bool> insert_result =
	    timer_pair_set_.insert(make_pair(expired_time, timer));
	assert(insert_result.second == true);
	return is_first_expired;
}

// The callback for IO read event, in this case, the timer fd alarms.
void TimerQueue::ReadCallback()
{
	owner_loop_->AssertInLoopThread();
	// 1. Get now time and Invoke `ReadTimerFd()` to read form timer_fd_.
	TimeStamp now(TimeStamp::Now());
	netlib::detail::ReadTimerFd(timer_fd_, now);
	// 2.	Get the expired timers relative to `now` and store them in expired_time_ vector.
	//		Run each timer's callback_.
	GetExpiredTimer(now);
	for(TimerVector::iterator it = expired_timer_.begin(); it != expired_timer_.end(); ++it)
	{
		(*it)->Run(); // Timer_Pointer->Run() runs timer object's callback_.
	}
	// 3. Refresh state for the next expiration.
	Refresh(now);
}

// Restart or delete expired timer and update timer_fd_'s expiration time.
void TimerQueue::Refresh(TimeStamp now)
{
	// 1. For expired timer: (1). Restart if it can repeat. (2). Delete it otherwise.
	for(TimerVector::iterator it = expired_timer_.begin(); it != expired_timer_.end(); ++it)
	{
		// Can repeat: restart timer and insert the updated Timer* into timer pair set again.
		if((*it)->repeat())
		{
			(*it)->Restart(now);
			InsertIntoTimerPairSet(*it);
		}
		else
		{
			delete (*it); // This timer can't repeat, delete it explicitly.
			(*it) = nullptr;
			// Since we always `expired_timer_.clear()` before GetExpiredTimer(),
			// we don't need erase this deleted Timer* in expired_time_ vector.
		}
	}
	// 2. Set next expiration time.
	TimeStamp next_expiration;
	if(timer_pair_set_.empty() == false)
	{
		// set<> is ordered by itself.
		next_expiration = (*(timer_pair_set_.begin()->second)).expiration();
	}
	if(next_expiration.IsValid() == true)
	{
		LOG_INFO("SET-Refresh");
		SetExpirationTime(next_expiration);
	}
}

// Set the expiration time to `expiration - Now()`. The argument `expiration` is the
// absolute expiration time, that is, there will have timers that expire at `expiration`
// seconds since Epoch.
void TimerQueue::SetExpirationTime(TimeStamp expiration)
{
	// 1. Convert `expiration` to a `struct timespec`
	int64_t microsecond = expiration.microsecond_since_epoch()
	                      - TimeStamp::Now().microsecond_since_epoch();
	// Suppose:
	//					RunEvery(callback, 1);	// Timer 1
	//					RunEvery(callback, 2);	// Timer 2
	//					RunAfter(callback, 2);	// Timer 3
	// At 1 second: timer 1 expires -> poll return -> callback() -> timer 1 restart to expire
	// at 2 second. Roughly speaking, timer 1, 2 and 3 will all expire at 2 second. But since
	// timer 1 runs callback and then set expiration time to 1 second later, its expiration
	// time is a little later than timer 2 and 3's expiration time, about tens of microseconds,
	// which is the following case. When we call `Refresh()->SetExpirationTime();`, the
	// timer 1 may have already expired(i.e., microsecond < 0) or about to expire
	// (i.e., 0 <= microsecond < 100). For both cases, we set the expiration time to 100
	// microsecond to let the timer expire almost instantly, thus poll will return and runs
	// this timer's callback instantly.
	if(microsecond < 100)
	{
		LOG_INFO("actual length = %lld\n", static_cast<long long int>(microsecond))
		microsecond = 100;
	}
	LOG_INFO("length = %lld\n", static_cast<long long int>(microsecond));
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microsecond / TimeStamp::kMicrosecondPerSecond);
	ts.tv_nsec =
	    static_cast<long>((microsecond % TimeStamp::kMicrosecondPerSecond) * 1000);

	// 2. Call timerfd_settime to set the new expiration time for the timer set.
	struct itimerspec new_value;
	bzero(&new_value, sizeof(new_value));
	new_value.it_value = ts;
	// int timerfd_settime(int fd, int flags, const struct itimerspec *new_value,
	//                     struct itimerspec *old_value);
	// flags = 0: start a relative timer. new_value.it_value specifies a time relative to
	// the current value of the clock specified by clockid.
	int ret = ::timerfd_settime(timer_fd_, 0, &new_value, NULL);
	// Return 0 on success; -1 on error and set errno to indicate the error.
	if(ret)
	{
		LOG_FATAL("timerfd_settime error: %s", strerror(errno));
	}
}
