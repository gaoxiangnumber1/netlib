#include <netlib/logging.h>

#include <stdarg.h> // va_*
#include <stdlib.h> // abort()
#include <string.h> // strerror_r()
#include <sys/time.h> // gettimeofday()
#include <time.h> // localtime_r()

#include <netlib/thread.h> // ThreadId()

using netlib::Thread;

namespace netlib
{

Logger::LogLevel Logger::log_level_ = Logger::INFO;
const char *Logger::log_level_string_[OFF] =
{"ALL  ", "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL",};

void Logger::Log(LogLevel level,
                 const char *file,
                 const char *func,
                 const int line,
                 int saved_errno,
                 const char *format ...)
{
	struct timeval tv;
	::gettimeofday(&tv, NULL);
	struct tm tm_time;
	::localtime_r(&tv.tv_sec, &tm_time);
	printf("%02d:%02d:%02d.%06d %5d %s %s:%d ",
	       tm_time.tm_hour,
	       tm_time.tm_min,
	       tm_time.tm_sec,
	       static_cast<int>(tv.tv_usec),
	       Thread::ThreadId(),
	       log_level_string_[level],
	       file, line);

	if(level <= DEBUG) // For TRACE and DEBUG.
	{
		printf("%s(): ", func);
	}

	// #include <stdarg.h>
	// void va_start(va_list ap, last);
	// void va_end(va_list ap);
	// va_start() initializes ap and must be called first. `last` is the name of the last
	// argument before the variable argument list, that is, the last argument of which
	// the calling function knows the type.
	// Each va_start() must be matched by a va_end() in the same function.
	// After the call va_end(ap) the variable ap is undefined.
	va_list argument;
	va_start(argument, format);
	// int vprintf(const char *format, va_list ap);
	vprintf(format, argument);
	va_end(argument);

	if(level >= ERROR && saved_errno != 0)
	{
		printf(" %s(errno=%d)" , ThreadSafeStrError(saved_errno), saved_errno);
	}
	printf("\n");

	if(level == FATAL)
	{
		abort();
	}
}

__thread char t_errno_buffer[32];
const char *ThreadSafeStrError(int saved_errno)
{
	return ::strerror_r(saved_errno, t_errno_buffer, sizeof t_errno_buffer);
}

}
