#ifndef NETLIB_NETLIB_TIME_STAMP_H_
#define NETLIB_NETLIB_TIME_STAMP_H_

#include <stdint.h> // int64_t

#include <string> // string.

#include <netlib/copyable.h>

namespace netlib
{

// Time stamp in UTC, in microseconds resolution. This class is immutable.
// It's recommended to pass it by value, since it's passed in register on x86-64.

// Review:
// Function: Ctor(int64_t)#explicit, Now#static

// Interface:
// Ctor(), Ctor(int64_t)
// microsecond_since_epoch
// set_invalid
// IsValid
// Now
// ToFormattedTimeString
// operator<, ==
// TimeDifferenceInSecond
// AddTime

class TimeStamp: public Copyable
{
public:
	static const int kMicrosecondPerSecond = 1000000; // 1 second = 10^6 microsecond.

	// Construct an invalid time stamp.
	TimeStamp(): microsecond_since_epoch_(0) {}
	// Construct a time stamp at specific time
	explicit TimeStamp(int64_t microsecond): microsecond_since_epoch_(microsecond) {}
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
	std::string ToFormattedTimeString() const;

private:
	int64_t microsecond_since_epoch_;
};

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
	return lhs.microsecond_since_epoch() < rhs.microsecond_since_epoch();
}
inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
	return lhs.microsecond_since_epoch() == rhs.microsecond_since_epoch();
}

// Get time difference of two timestamps(high-low), result in seconds.
inline double TimeDifferenceInSecond(TimeStamp high, TimeStamp low)
{
	if(high < low)
	{
		return TimeDifferenceInSecond(low, high);
	}
	int64_t difference = high.microsecond_since_epoch() - low.microsecond_since_epoch();
	return static_cast<double>(difference) / TimeStamp::kMicrosecondPerSecond;
}

inline TimeStamp AddTime(TimeStamp base_time_stamp, double second)
{
	return TimeStamp(base_time_stamp.microsecond_since_epoch() +
	                 static_cast<int64_t>(second * TimeStamp::kMicrosecondPerSecond));
}

}

#endif // NETLIB_NETLIB_TIME_STAMP_H_
