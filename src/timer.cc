#include <timer.h>

void Timer::Restart(Timestamp now)
{
	if(repeat_)
	{
		expiration_ = AddTime(now, interval_); // Timestamp::
	}
	else
	{
		expiration_ = Timestamp::Invalid();
	}
}
