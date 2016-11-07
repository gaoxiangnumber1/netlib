#include <netlib/timer.h>

using netlib::Timer;
using netlib::TimeStamp;

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
