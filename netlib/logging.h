#ifndef NETLIB_NETLIB_LOGGING_H_
#define NETLIB_NETLIB_LOGGING_H_

#include <errno.h> // errno
#include <stdio.h> // *printf()

#include <netlib/copyable.h> // Copyable

#define LOG(level, ...) do \
{ \
	if(level >= netlib::Logger::log_level()) \
	{ \
		::snprintf(0, 0, __VA_ARGS__); \
		netlib::Logger::Log(level, errno, __VA_ARGS__); \
	} \
} \
while(false);

#define LOG_TRACE(...) LOG(netlib::Logger::TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(netlib::Logger::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LOG(netlib::Logger::INFO, __VA_ARGS__)
#define LOG_WARN(...) LOG(netlib::Logger::WARN, __VA_ARGS__)
#define LOG_ERROR(...) LOG(netlib::Logger::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG(netlib::Logger::FATAL, __VA_ARGS__)

#define SetLogLevel(level) netlib::Logger::set_log_level(netlib::Logger::level)

namespace netlib
{

class Logger: public Copyable
{
public:
	enum LogLevel
	{
		ALL = 0,
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
	static void Log(LogLevel level, int saved_errno, const char * ...);

private:
	static const char *log_level_string_[OFF];
	static LogLevel log_level_;
};

const char *ThreadSafeStrError(int saved_errno);

// Check the input is non null.  This is useful in constructor initializer lists.
#define CHECK_NOT_NULL(val) \
netlib::CheckNotNull("'" #val "' Must be non NULL", (val))

// A helper for CHECK_NOTNULL().
template <typename T>
T *CheckNotNull(const char *name, T *ptr)
{
	if(ptr == nullptr)
	{
		LOG_FATAL("%s", name);
	}
	return ptr;
}

}

#endif // NETLIB_NETLIB_LOGGING_H_
