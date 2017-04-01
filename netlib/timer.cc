#include <netlib/timer.h>

using netlib::Timer;
using netlib::TimeStamp;

int64_t Timer::created_timer_number_ = 0;
Timer::Timer(const TimerCallback &callback,
             const TimeStamp &time_stamp,
             double interval):
	callback_(callback),
	expired_time_(time_stamp),
	interval_(interval),
	repeat_(interval_ > 0.0),
	sequence_(++created_timer_number_)
{}

void Timer::Restart(const TimeStamp &now)
{
	if(repeat_ == true)
	{
		expired_time_ = AddTime(now, interval_);
	}
}
void Timer::Run() const
{
	callback_();
}
