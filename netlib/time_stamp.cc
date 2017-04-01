#include <netlib/time_stamp.h>

#include <time.h> // localtime_r()
#include <stdio.h> // snprintf()
#include <sys/time.h> // gettimeofday()

using std::string;
using netlib::TimeStamp;

TimeStamp TimeStamp::Now()
{
	struct timeval tv;
	::gettimeofday(&tv, NULL);
	return TimeStamp(tv.tv_sec * kMicrosecondPerSecond + tv.tv_usec);
}
string TimeStamp::ToFormattedTimeString() const
{
	time_t second = static_cast<time_t>(microsecond_ / kMicrosecondPerSecond);
	struct tm tm_time;
	::localtime_r(&second, &tm_time);
	char buffer[32];
	::snprintf(buffer, sizeof buffer, "%02d:%02d:%02d.%06ld",
	           tm_time.tm_hour,
	           tm_time.tm_min,
	           tm_time.tm_sec,
	           microsecond_ % kMicrosecondPerSecond);
	return buffer;
}

TimeStamp netlib::AddTime(const TimeStamp &base_time_stamp, double second)
{
	return TimeStamp(base_time_stamp.microsecond() +
	                 static_cast<int64_t>(second * TimeStamp::kMicrosecondPerSecond));
}
double netlib::TimeDifferenceInSecond(const TimeStamp &high, const TimeStamp &low)
{
	if(high < low)
	{
		return TimeDifferenceInSecond(low, high);
	}
	int64_t difference = high.microsecond() - low.microsecond();
	return static_cast<double>(difference) / TimeStamp::kMicrosecondPerSecond;
}
