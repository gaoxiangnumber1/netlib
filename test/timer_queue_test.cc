#include <stdio.h> // printf()
#include <unistd.h> // getpid(), sleep()

#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>
#include <netlib/thread.h>
#include <netlib/time_stamp.h>
#include <netlib/timer_id.h>

using std::bind;
using netlib::EventLoop;
using netlib::EventLoopThread;
using netlib::Thread;
using netlib::TimeStamp;
using netlib::TimerId;

int cnt = 0;
const int kMaxCnt = 15;
EventLoop *g_loop;

void PrintThreadId()
{
	printf("pid = %d, tid = %d now %s\n",
	       getpid(),
	       Thread::ThreadId(),
	       TimeStamp::Now().ToFormattedTimeString().c_str());
}

void PrintMessage(const char *message)
{
	printf("%s: %s\n", message, TimeStamp::Now().ToFormattedTimeString().c_str());
	if (++cnt == kMaxCnt)
	{
		g_loop->Quit();
	}
}

void CancelTimer(TimerId timer)
{
	g_loop->CancelTimer(timer);
	printf("Cancel : %s\n", TimeStamp::Now().ToFormattedTimeString().c_str());
}

int main()
{
	PrintThreadId();
	sleep(1);
	{
		EventLoop loop;
		g_loop = &loop;

		PrintMessage("main   ");
		loop.RunAfter(bind(PrintMessage, "once3.5"), 3.5);
		loop.RunEvery(bind(PrintMessage, "every2 "), 2.0);
		loop.RunAfter(bind(PrintMessage, "once1  "), 1);
		TimerId timer_id_45 = loop.RunAfter(bind(PrintMessage, "once4.5"), 4.5);
		loop.RunAfter(bind(CancelTimer, timer_id_45), 4.2);
		loop.RunAfter(bind(CancelTimer, timer_id_45), 4.8);
		TimerId timer_id_30 = loop.RunEvery(bind(PrintMessage, "every3 "), 3.0);
		loop.RunAfter(bind(CancelTimer, timer_id_30), 8.999);

		loop.Loop();
		PrintMessage("exit   ");
	}
	sleep(1);
	{
		EventLoopThread loop_thread;
		EventLoop *loop = loop_thread.StartLoop();
		loop->RunAfter(PrintThreadId, 2);
		sleep(3);
		PrintMessage("Thread loop exits");
	}
}
