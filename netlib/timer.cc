#include <netlib/timer.h>

using std::atomic;
using netlib::Timer;
using netlib::TimeStamp;

atomic<int64_t> Timer::created_timer_number_(0);

// Restart timer from now on if interval_ > 0.0.
// Called: TimerQueue::HandleRead()->TimerQueue::Refresh()
void Timer::Restart(TimeStamp now)
{
	if(repeat_) // true if interval_ > 0.0
	{
		expired_time_ = AddTime(now, interval_); // expired_time_ = now + interval_;
	}
	else
	{
		expired_time_.set_invalid(); // expired_time_ = 0;
	}
}
