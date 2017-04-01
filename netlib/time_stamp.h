#ifndef NETLIB_NETLIB_TIME_STAMP_H_
#define NETLIB_NETLIB_TIME_STAMP_H_

#include <stdint.h> // int64_t

#include <string> // string.

#include <netlib/copyable.h>

namespace netlib
{

// Interface:
// Ctor(), Ctor(int64_t)
// microsecond
// Now
// ToFormattedTimeString
// operator<, ==
// AddTime
// TimeDifferenceInSecond

class TimeStamp: public Copyable
{
public:
	static const int kMicrosecondPerSecond = 1000000; // 1 second = 10^6 microsecond.

	TimeStamp(): microsecond_(0) {}
	explicit TimeStamp(int64_t microsecond_arg): microsecond_(microsecond_arg) {}
	// Default copy-ctor/assignment/dtor is okay.

	int64_t microsecond() const
	{
		return microsecond_;
	}
	static TimeStamp Now();
	std::string ToFormattedTimeString() const;

private:
	int64_t microsecond_; // since Epoch, 1970-01-01 00:00:00 +0000 (UTC)
};

inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs)
{
	return lhs.microsecond() < rhs.microsecond();
}
inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs)
{
	return lhs.microsecond() == rhs.microsecond();
}

TimeStamp AddTime(const TimeStamp &base_time_stamp, double second);
double TimeDifferenceInSecond(const TimeStamp &high, const TimeStamp &low);

}

#endif // NETLIB_NETLIB_TIME_STAMP_H_
