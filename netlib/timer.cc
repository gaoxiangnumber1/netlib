#include <netlib/timer.h>

using std::atomic;
using netlib::Timer;
using netlib::TimeStamp;

atomic<int64_t> Timer::created_number_(0);

// Restart timer from now on if interval_ > 0.0.
// Called: TimerQueue::HandleRead()->TimerQueue::Refresh()
void Timer::Restart(TimeStamp now)
{
	if(repeat_) // true if interval_ > 0.0
	{
		expiration_ = AddTime(now, interval_); // expiration_ = now + interval_;
	}
	else
	{
		expiration_.set_invalid(); // expiration_ = 0;
	}
}
