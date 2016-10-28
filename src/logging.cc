namespace netlib
{
const kSecondsPerDay = 24 * 60 * 60;

Logger::Logger():
	log_level(INFO),
	fd_(-1),
	last_rotate_(::time(NULL)),
	real_rotate(last_rotate_),
	rotate_interval_(kSecondsPerDay)
{
	::tzset(); // TODO: what use?
}
Logger::~Logger()
{
	if(fd_ != -1)
	{
		::close(fd_);
	}
}

const char *Logger::log_level_string_[ALL + 1] =
{
	"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "ALL",
};

void set_log_level(const string &level)
{
	LogLevel log_level = INFO;
	for(int index = 0; index < ALL; ++index)
	{
		if(strcasecmp(level_string_[index], level.c_str()) == 0)
		{
			log_level = static_cast<LogLevel>(index);
			break;
		}
	}
	set_log_level(log_level);
}

}
