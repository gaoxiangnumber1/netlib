#ifndef NETLIB_SRC_BASE_LOGGING_H_;
#define NETLIB_SRC_BASE_LOGGING_H_

#include <types.h>

namespace netlib
{

class Logger: public Copyable
{
public:
	enum LogLevel
	{
		FATAL = 0, ERROR, WARN, INFO, DEBUG, TRACE, ALL,
	};

	Logger();
	~Logger();

	void Log(int level, const char *file, int line, const char *func, const char *fmt ...);

	void set_file_name(const string &file_name);
	void set_log_level(const string &level);
	void set_log_level(LogLevel level)
	{
		level_ = (level <= FATAL ? FATAL : (level <= ALL ? level : ALL));
	}

	LogLevel log_level()
	{
		return level_;
	}
	const char *log_level_string()
	{
		return log_level_string[level_];
	}
	int fd()
	{
		return fd_;
	}

	void AdjustLogLevel(int adjust)
	{
		set_log_level(LogLevel(level_ + adjust));
	}
	void set_rotate_interval(long rotate_interval)
	{
		rotate_interval_ = rotate_interval;
	}
	//static Logger &logger(); // TODO: When use???

private:
	static const char *log_level_string_[ALL + 1];

	void Rotate();

	LogLevel log_level_;
	string file_name_;
	int fd_;
	uint64_t last_rotate_;
	std::atomic<uint64_t> real_rotate_;
	long rotate_interval_;
};

}

#endif // NETLIB_SRC_BASE_LOGGING_H_
