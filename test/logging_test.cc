#include <logging.h>

int main()
{
	SetLogFile("test.log");
	SetLogLevel(ALL);
	LOG_FATAL("hello world");
	LOG_ERROR("hello world");
	LOG_WARN("hello world");
	LOG_INFO("hello world");
	LOG_DEBUG("hello world");
	LOG_TRACE("hello world");
}
