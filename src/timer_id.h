#ifndef NETLIB_SRC_TIMER_ID_H_
#define NETLIB_SRC_TIMER_ID_H_

namespace netlib
{

class Timer;

// An opaque identifier, for canceling Timer.
class TimerId: public Copyable
{
public:
	explicit TimerId(Timer *timer): timer_(timer) {}

private:
	Timer *timer_;
};

}

#endif // NETLIB_SRC_TIMER_ID_H_
