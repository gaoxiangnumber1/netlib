#include <netlib/time_stamp.h>

#include <time.h> // localtime_r()
#include <stdio.h> // snprintf()
#include <sys/time.h> // gettimeofday()

using std::string;
using netlib::TimeStamp;

// Return a TimeStamp object that holds the microseconds since epoch for now.
TimeStamp TimeStamp::Now()
{
	// #include <sys/time.h>
	// int gettimeofday(struct timeval *tv, struct timezone *tz);
	// Get the time and a timezone. The tv argument is a struct timeval(<sys/time.h>) that
	// gives the number of seconds and microseconds since the Epoch(see time(2)).
	// struct timeval
	// {
	//		time_t tv_sec; // seconds
	//		suseconds_t tv_usec; // microseconds
	// };
	// If either tv or tz is NULL, the corresponding structure is not set or returned.
	// The tz argument should be specified as NULL.
	// Return 0 for success, or -1 for failure and errno is set.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return TimeStamp(tv.tv_sec * kMicrosecondPerSecond + tv.tv_usec);
}

string TimeStamp::ToFormattedTimeString() const
{
	char buffer[32];
	time_t second =
	    static_cast<time_t>(microsecond_since_epoch_ / kMicrosecondPerSecond);
	int microsecond =
	    static_cast<int>(microsecond_since_epoch_ % kMicrosecondPerSecond);
	// #include <time.h>
	// struct tm *localtime_r(const time_t *timep, struct tm *result);
	// timep represents calendar time, i.e., the number of seconds elapsed since Epoch(UTC).
	// Broken-down time is stored in the structure tm which is defined in <time.h>:
	// struct tm
	// {
	// 	int tm_sec;         /* seconds */
	// 	int tm_min;         /* minutes */
	// 	int tm_hour;        /* hours */
	// 	int tm_mday;        /* day of the month */
	// 	int tm_mon;         /* month */
	// 	int tm_year;        /* year */
	// };
	// localtime_r() converts the calendar time timep to broken-down time representation,
	// expressed relative to the user's specified timezone; and stores the data in a user-supplied
	// struct. Return NULL on error.
	struct tm tm_time;
	localtime_r(&second, &tm_time);
	snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%06d",
	         tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microsecond);
	return buffer;
}
