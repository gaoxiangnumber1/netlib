#include <netlib/timer.h>

using std::atomic;
using netlib::Timer;
using netlib::TimeStamp;

atomic<int64_t> Timer::created_timer_number_(0);
Timer::Timer(const TimerCallback &callback, TimeStamp time_stamp, double interval):
	callback_(callback),
	expired_time_(time_stamp),
	interval_(interval),
	repeat_(interval_ > 0.0),
	sequence_(++created_timer_number_) // Increment and get.
{}


// Restart timer from now on if interval_ > 0.0.
// Called: TimerQueue::HandleRead()->TimerQueue::Refresh()
void Timer::Restart(TimeStamp now)
{
	if(repeat_ == true) // true if interval_ > 0.0
	{
		expired_time_ = AddTime(now, interval_); // expired_time_ = now + interval_;
	}
	else
	{
		expired_time_.set_invalid(); // expired_time_ = 0;
	}
}

void Timer::Run() const
{
	callback_();
}
