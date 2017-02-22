#ifndef NETLIB_NETLIB_LOGGING_H_
#define NETLIB_NETLIB_LOGGING_H_

#include <errno.h> // errno
#include <stdio.h> // *printf()

#include <netlib/copyable.h> // Copyable

// Review:
// NonFunction: LogLevel#OFF, log_level_#static#.cc, log_level_string_#.cc
// Function: Log#level, CheckNotNull#name

// Interface:
// log_level
// set_log_level
// Log
// ThreadSafeStrError
// CheckNotNull

// Macros:
// SetLogLevel -> +set_log_level
// LOG -> +Log
// LOG_XXX -> LOG
// CHECK_NOT_NULL -> CheckNotNull

namespace netlib
{

class Logger: public Copyable
{
public:
	// By default, enumerator values start at 0 and each enumerator
	// has a value 1 greater than the preceding one.
	enum LogLevel
	{
		ALL,
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		OFF
	};

	static LogLevel log_level()
	{
		return log_level_;
	}
	static void set_log_level(LogLevel level)
	{
		log_level_ = level;
	}
	static void Log(LogLevel level, const char *file, const char *func, const int line,
	                int saved_errno, const char *format ...);

private:
	static LogLevel log_level_;
	static const char *log_level_string_[OFF];
};

const char *ThreadSafeStrError(int saved_errno);

}

#define SetLogLevel(level) netlib::Logger::set_log_level(netlib::Logger::level)

#define LOG(level, ...) do \
{ \
	/* `level` has added `netlib::Logger::` in the expansion of LOG_XXX */ \
	if(level >= netlib::Logger::log_level()) \
	{ \
		::snprintf(0, 0, __VA_ARGS__); /* Check whether the format is matched. */ \
		netlib::Logger::Log(level, __FILE__, __func__, __LINE__, errno, __VA_ARGS__); \
	} \
} \
while(false)

#define LOG_TRACE(...) LOG(netlib::Logger::TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(netlib::Logger::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LOG(netlib::Logger::INFO, __VA_ARGS__)
#define LOG_WARN(...) LOG(netlib::Logger::WARN, __VA_ARGS__)
#define LOG_ERROR(...) LOG(netlib::Logger::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG(netlib::Logger::FATAL, __VA_ARGS__)

template <typename T>
T *CheckNotNull(const char *name, T *ptr)
{
	if(ptr == nullptr)
	{
		LOG_FATAL("%s", name);
	}
	return ptr;
}

// Check the input is not null.  This is useful in constructor initializer lists.
#define CHECK_NOT_NULL(arg) CheckNotNull("'" #arg "' Must not be NULL", (arg))

#endif // NETLIB_NETLIB_LOGGING_H_
