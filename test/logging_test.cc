#include <sys/types.h>
#include <unistd.h> // getpid(), sleep()

#include <netlib/logging.h>
#include <netlib/thread.h>

using netlib::Thread;

void Function()
{
	LOG_TRACE("pid = %d", getpid());
	LOG_DEBUG("pid = %d", getpid());
	LOG_INFO("pid = %d", getpid());
	LOG_WARN("pid = %d", getpid());
	LOG_ERROR("pid = %d", getpid());
}

int main()
{
	SetLogLevel(ALL);
	for(int index = 0; index < 3; ++index)
	{
		Thread thread(Function);
		thread.Start();
		thread.Join();
	}
	SetLogLevel(INFO);
	for(int index = 0; index < 3; ++index)
	{
		Thread thread(Function);
		thread.Start();
		thread.Join();
	}
}
