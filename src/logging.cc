#include <logging.h>

#include <stdio.h> // *printf(), rename()
#include <time.h> // time(), tzset(), localtime_r()
#include <unistd.h> // close(), dup2(), write()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> // open()
#include <string.h> // strerror()
#include <stdlib.h> // exit()
#include <sys/time.h> // gettimeofday()
#include <stdarg.h> // va_*

using std::string;

namespace netlib
{
const int kSecondsPerDay = 24 * 60 * 60;
const int kBufferSize = 4096;
// O_CREAT			Create the file if it doesn't exist.
// O_WRONLY		Open for writing only.
// O_APPEND		Append to the end of file on each write.
// O_CLOEXEC	Set the close-on-exec flag.
#define OPEN_FLAG (O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC)

const char *Logger::log_level_string_[ALL + 1] =
{
	"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "ALL",
};

Logger::Logger():
	log_level_(INFO),
	fd_(-1),
	rotate_time_(::time(NULL)),
	rotate_interval_(kSecondsPerDay) // Rotate every 24 hours.
{
	::tzset(); // Initialize time related variables, such as timezone.
}

Logger::~Logger()
{
	if(fd_ != -1) // If fd_ is opened, close it.
	{
		::close(fd_);
	}
}

void Logger::set_file_name(const string &file_name) // Set file name and fd_.
{
	// int open(const char *pathname, int flags, mode_t mode);
	// DEFFILEMODE	0666(use/group/other can all read/write)
	int fd = open(file_name.c_str(), OPEN_FLAG, DEFFILEMODE);
	if(fd == -1) // open fail.
	{
		fprintf(stderr, "Open log file %s failed. errno: %s\n", file_name.c_str(), strerror(errno));
		return;
	}
	file_name_ = file_name;
	if(fd_ == -1) // If fd_ is not open, i.e., if this object doesn't hold any file.
	{
		fd_ = fd; // Share the opened file descriptor.
		// Since only one value of file descriptor(fd == fd_) is opened,
		// we can't `close(fd)` that will also close our fd_.
	}
	else // If fd_ is already open.
	{
		// fd_ is first closed and the value of the new(duplicated) descriptor is fd_.
		if(dup2(fd, fd_) == -1) // Return -1 if dup2 failed.
		{
			fprintf(stderr, "dup2 failed.\n");
			exit(1);
		}
		// Since two different values of file descriptor(fd != fd_) are opened,
		// `close(fd)` only close this file descriptor in this process table entry,
		// don't close file-table in the kernel, and fd_ is still opened to this file.
		close(fd);
	}
}

//static thread_local int64_t tid;
void Logger::Log(int level, const char *file, int line, const char *format ...)
{
	if(level > log_level_)
	{
		return;
	}
	Rotate();
	char buffer[kBufferSize];
	char *ptr = buffer;
	char *buffer_end = buffer + sizeof(buffer);

	struct timeval now_time_val;
	// int gettimeofday(struct timeval *tv, struct timezone *tz);
	gettimeofday(&now_time_val, NULL);
	struct tm now_tm;
	// struct tm *localtime_r(const time_t *timep, struct tm *result);
	localtime_r(&(now_time_val.tv_sec), &now_tm);
	ptr += snprintf(ptr, buffer_end - ptr,
	                "%s\t%04d/%02d/%02d-%02d:%02d:%02d.%06d tid = ? %s:%d ",
	                log_level_string_[level],
	                now_tm.tm_year + 1900,
	                now_tm.tm_mon + 1,
	                now_tm.tm_mday,
	                now_tm.tm_hour,
	                now_tm.tm_min,
	                now_tm.tm_sec,
	                static_cast<int>(now_time_val.tv_usec),
	                //(long)tid,
	                file,
	                line);
	// #include <stdarg.h>
	va_list args;
	va_start(args, format);
	ptr += vsnprintf(ptr, buffer_end - ptr, format, args);
	va_end(args);
	ptr = (ptr < buffer_end - 2 ? ptr : buffer_end - 2);
	// Trim the ending \n.
	while(*(--ptr) == '\n');
	*(++ptr) = '\n';
	*(++ptr) = '\0';
	int fd = (fd_ == -1 ? 1 : fd_);
	int written = static_cast<int>(::write(fd, buffer, ptr - buffer));
	if(written != ptr - buffer)
	{
		fprintf(stderr, "Write log file %s failed. written %d bytes. errno: %s\n",
		        file_name_.c_str(), written, strerror(errno));
	}
}

void Logger::Rotate()
{
	time_t now = time(NULL); // Get now time.
	// If (1) don't have log file or (2) now-time and rotate-time are in the same day:
	// don't need rotate.
	if(file_name_.empty() ||
	        (now - timezone)/rotate_interval_ == (rotate_time_.load() - timezone)/rotate_interval_)
	{
		return;
	}
	rotate_time_.exchange(now); // Atomic exchange value.

	struct tm now_tm;
	// struct tm *localtime_r(const time_t *timep, struct tm *result);
	localtime_r(&now, &now_tm);
	const char *old_file = file_name_.c_str();
	char new_file[kBufferSize];
	snprintf(new_file, sizeof(new_file), "%s.%d%02d%02d%02d%02d",
	         old_file,
	         now_tm.tm_year + 1900,
	         now_tm.tm_mon + 1,
	         now_tm.tm_mday,
	         now_tm.tm_hour,
	         now_tm.tm_min);
	// int rename(const char *oldpath, const char *newpath);
	// 1. Rename the file that is already opened.
	if(rename(old_file, new_file) != 0)
	{
		fprintf(stderr, "Rename logging file %s TO %s failed errno: %s\n",
		        old_file, new_file, strerror(errno));
		return;
	}
	// 2. Create new logging file.
	int fd = open(file_name_.c_str(), OPEN_FLAG, DEFFILEMODE);
	if(fd < 0)
	{
		fprintf(stderr, "Open logging file %s failed. errno: %s\n", new_file, strerror(errno));
		return;
	}
	// 3. Duplicate the new file descriptor and set its value to the old file descriptor.
	dup2(fd, fd_);
	// 4. Close the first new file descriptor.
	close(fd);
}

Logger &Logger::logger()
{
	static Logger logger;
	return logger;
}

}
