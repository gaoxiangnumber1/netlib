#include <sys/types.h>
#include <unistd.h> // getpid(), sleep()

#include <functional> // bind<>

#include <thread.h>
#include <time_stamp.h>
#include <event_loop.h>
#include <logging.h>

using std::bind;
using netlib::Thread;
using netlib::TimeStamp;
using netlib::EventLoop;

int g_counter = 0;
EventLoop *g_loop = nullptr;

void PrintString(const char *message)
{
	LOG_INFO("msg: %s; g_counter = %d\n", message, g_counter);
	if(++g_counter == 100)
	{
		LOG_INFO("set_quit(true)\n");
		g_loop->set_quit(true);
	}
}

void PrintId()
{
	LOG_INFO("pid = %d, tid = %d\n", getpid(), Thread::ThreadId());
	LOG_INFO("now %s\n", TimeStamp::Now().ToString());
}

int main()
{
	PrintId();
	EventLoop loop;
	g_loop = &loop;

	LOG_INFO("main");
	loop.RunAfter(bind(PrintString, "12"), 12);
	loop.RunAfter(bind(PrintString, "10"), 10);
	loop.RunAfter(bind(PrintString, "9"), 9);
	loop.RunAfter(bind(PrintString, "8"), 8);
	loop.RunAfter(bind(PrintString, "6"), 6);
	loop.RunAfter(bind(PrintString, "4"), 4);
	loop.RunAfter(bind(PrintString, "3"), 3);
	loop.RunEvery(bind(PrintString, "33333"), 3);
	loop.RunAfter(bind(PrintString, "2"), 2);
	loop.RunEvery(bind(PrintString, "22222"), 2);
	loop.RunEvery(bind(PrintString, "11111"), 1);

	loop.Loop();
	PrintString("main exist1.");
	sleep(3);
	PrintString("main exist2.");
}
