#ifndef NETLIB_NETLIB_TIME_STAMP_H_
#define NETLIB_NETLIB_TIME_STAMP_H_

#include <stdint.h>

#include <netlib/copyable.h>

namespace netlib
{

// Time stamp in UTC, in microseconds resolution. This class is immutable.
// It's recommended to pass it by value, since it's passed in register on x64.
class TimeStamp: public Copyable
{
public:
	static const int kMicrosecondPerSecond = 1000 * 1000;

	// Construct an invalid time stamp.
	TimeStamp(): microsecond_since_epoch_(0) {}
	// Construct a time stamp at specific time
	explicit TimeStamp(int64_t microsecond_since_epoch_arg):
		microsecond_since_epoch_(microsecond_since_epoch_arg) {}
	// Default copy-ctor/assignment/dtor is okay.
	// Getter
	int64_t microsecond_since_epoch() const
	{
		return microsecond_since_epoch_;
	}
	// Setter
	void set_invalid()
	{
		microsecond_since_epoch_ = 0;
	}

	bool IsValid() const
	{
		return microsecond_since_epoch_ > 0;
	}
	static TimeStamp Now();
	char *ToString() const;

private:
	int64_t microsecond_since_epoch_; // 1 second = 10^6 microsecond.
};

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
	return lhs.microsecond_since_epoch() < rhs.microsecond_since_epoch();
}

inline TimeStamp AddTime(TimeStamp time_stamp, double second)
{
	int64_t microsecond =
	    static_cast<int64_t>(second * TimeStamp::kMicrosecondPerSecond);
	return TimeStamp(time_stamp.microsecond_since_epoch() + microsecond);
}

}

#endif // NETLIB_NETLIB_TIME_STAMP_H_
