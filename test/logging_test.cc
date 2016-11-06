#include <sys/types.h>
#include <unistd.h> // getpid(), sleep()

#include <logging.h>
#include <thread.h>

using netlib::Thread;

void Function()
{
	LOG_FATAL("pid = %d, tid = %d", getpid(), Thread::ThreadId());
	LOG_ERROR("pid = %d, tid = %d", getpid(), Thread::ThreadId());
	LOG_WARN("pid = %d, tid = %d", getpid(), Thread::ThreadId());
	sleep(1);
	LOG_INFO("pid = %d, tid = %d", getpid(), Thread::ThreadId());
	LOG_DEBUG("pid = %d, tid = %d", getpid(), Thread::ThreadId());
	LOG_TRACE("pid = %d, tid = %d", getpid(), Thread::ThreadId());
}

int main()
{
	SetLogFile("test.log");
	SetLogLevel(ALL);

	for(int index = 0; index < 100; ++index)
	{
		Thread thread1(Function);
		thread1.Start();
		Thread thread2(Function);
		thread2.Start();
		Thread thread3(Function);
		thread3.Start();
		Thread thread4(Function);
		thread4.Start();
		Thread thread5(Function);
		thread5.Start();
		thread1.Join();
		thread2.Join();
		thread3.Join();
		thread4.Join();
		thread5.Join();
	}
	LOG_INFO("Created %d threads.", Thread::created_number());
}
