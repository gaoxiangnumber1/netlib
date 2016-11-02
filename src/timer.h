#ifndef NETLIB_SRC_TIMER_H_
#define NETLIB_SRC_TIMER_H_

namespace netlib
{

// Internal class for timer event.
class Timer: public NonCopyable
{
public:
	Timer(const TimerCallback &callback, Timestamp when, double interval):
		callback_(callback), expiration_(when), interval_(interval), repeat_(interval > 0.0)
	{}
	// Getter
	Timestamp expiration() const
	{
		return expiration_;
	}
	bool repeat() const
	{
		return repeat_;
	}

	void Run() const
	{
		callback_();
	}
	void Restart(Timestamp now);

private:
	const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
};

}

#endif // NETLIB_SRC_TIMER_H_
