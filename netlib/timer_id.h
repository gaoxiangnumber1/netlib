#ifndef NETLIB_NETLIB_TIMER_ID_H_
#define NETLIB_NETLIB_TIMER_ID_H_

#include <netlib/copyable.h>

namespace netlib
{

class Timer;

// An opaque不透明的 identifier, for canceling Timer.
class TimerId: public Copyable
{
public:
	// Called in `TimerQueue::AddTimer(const TimerCallback&, TimeStamp, double);
	explicit TimerId(Timer *timer): timer_(timer) {}

private:
	Timer *timer_;
};

}

#endif // NETLIB_NETLIB_TIMER_ID_H_
