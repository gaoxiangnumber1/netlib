#include <netlib/time_stamp.h>

#include <inttypes.h> // PRId64: int64_t
#include <stdio.h> // snprintf()
#include <stdlib.h> // calloc()
#include <sys/time.h> // gettimeofday()

using std::string;
using netlib::TimeStamp;

string TimeStamp::ToString() const
{
	char buffer[32];
	int64_t second = microsecond_since_epoch_ / kMicrosecondPerSecond;
	int64_t microsecond = microsecond_since_epoch_ % kMicrosecondPerSecond;
	snprintf(buffer, 32, "%" PRId64 ".%06" PRId64 "", second, microsecond);
	return buffer;
}

string TimeStamp::ToFormattedString() const
{
	char buffer[32] = {0};
	time_t second =
	    static_cast<time_t>(microsecond_since_epoch_ / kMicrosecondPerSecond);
	int microsecond =
	    static_cast<int>(microsecond_since_epoch_ % kMicrosecondPerSecond);
	struct tm tm_time;
	gmtime_r(&second, &tm_time);

	snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d.%06d",
	         tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
	         tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec,
	         microsecond);
	return buffer;
}

TimeStamp TimeStamp::Now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t second = tv.tv_sec;
	return TimeStamp(second * kMicrosecondPerSecond + tv.tv_usec);
}
