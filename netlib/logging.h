#ifndef NETLIB_NETLIB_LOGGING_H_
#define NETLIB_NETLIB_LOGGING_H_

#include <stdint.h> // int64_t

#include <atomic> // atomic
#include <string> // string

#include <netlib/copyable.h> // Copyable

#define LOG(level, ...) do \
{ \
	if(level <= netlib::Logger::logger().log_level()) \
	{ \
		::snprintf(0, 0, __VA_ARGS__); \
		netlib::Logger::logger().Log(level, __FILE__, __LINE__, __VA_ARGS__); \
	} \
} \
while(false);

#define LOG_FATAL(...) LOG(netlib::Logger::FATAL, __VA_ARGS__)
#define LOG_ERROR(...) LOG(netlib::Logger::ERROR, __VA_ARGS__)
#define LOG_WARN(...) LOG(netlib::Logger::WARN, __VA_ARGS__)
#define LOG_INFO(...) LOG(netlib::Logger::INFO, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(netlib::Logger::DEBUG, __VA_ARGS__)
#define LOG_TRACE(...) LOG(netlib::Logger::TRACE, __VA_ARGS__)

#define SetLogLevel(level) netlib::Logger::logger().set_log_level(netlib::Logger::level)
#define SetLogFile(file) netlib::Logger::logger().set_file_name(file)

namespace netlib
{

class Logger: public netlib::Copyable
{
public:
	enum LogLevel
	{
		FATAL = 0, ERROR, WARN, INFO, DEBUG, TRACE, ALL,
	};

	Logger();
	~Logger();

	LogLevel log_level() const
	{
		return log_level_;
	}
	void set_log_level(LogLevel level)
	{
		log_level_ = (level <= FATAL ? FATAL : (level <= ALL ? level : ALL));
	}
	void set_file_name(const std::string &file_name);

	void Log(int level, const char *file, int line, const char *fmt ...);

	static Logger &logger(); // Get Logger object.

private:
	static const char *log_level_string_[ALL + 1];

	void Rotate();

	LogLevel log_level_;
	std::string file_name_;
	int fd_;
	std::atomic<int64_t> rotate_time_;
	int64_t rotate_interval_;
};

}

#endif // NETLIB_NETLIB_LOGGING_H_
