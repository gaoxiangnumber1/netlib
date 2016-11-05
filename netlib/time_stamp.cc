#include <netlib/time_stamp.h>

#include <inttypes.h> // PRId64: int64_t
#include <stdio.h> // snprintf()
#include <stdlib.h> // calloc()
#include <sys/time.h> // gettimeofday()

using netlib::TimeStamp;

char *TimeStamp::ToString() const
{
	char *buf = static_cast<char*>(malloc(32));
	int64_t second = microsecond_since_epoch_ / kMicrosecondPerSecond;
	int64_t microsecond = microsecond_since_epoch_ % kMicrosecondPerSecond;
	snprintf(buf, 32, "%" PRId64 ".%06" PRId64 "", second, microsecond);
	return buf;
}

TimeStamp TimeStamp::Now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t second = tv.tv_sec;
	return TimeStamp(second * kMicrosecondPerSecond + tv.tv_usec);
}
