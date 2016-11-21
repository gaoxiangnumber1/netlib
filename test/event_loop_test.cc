#include <sys/types.h>
#include <unistd.h> // getpid()

#include <netlib/logging.h>
#include <netlib/event_loop.h>
#include <netlib/thread.h>

using netlib::EventLoop;
using netlib::Thread;

void Callback()
{
	LOG_INFO("Callback(): pid = %d", getpid());
	EventLoop another_loop;
}

void ThreadFunction()
{
	LOG_INFO("ThreadFunction(): pid = %d", getpid());

	EventLoop loop;
	loop.RunAfter(Callback, 1.0);
	loop.Loop();
}

int main()
{
	LOG_INFO("main(): pid = %d", getpid());

	EventLoop loop;
	Thread thread(ThreadFunction);
	thread.Start();

	loop.Loop();
}
