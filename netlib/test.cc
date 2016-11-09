#include <stdio.h>

#include <netlib/event_loop.h>
#include <netlib/thread.h>
#include <netlib/time_stamp.h>

using netlib::TimerId;
using netlib::EventLoop;

int cnt = 0;
EventLoop* g_loop;

TimerId toCancel;
void cancelSelf()
{
	printf("cancelSelf()\n");
	g_loop->Cancel(toCancel);
}

int main()
{
	EventLoop loop;
	g_loop = &loop;

	printf("main\n");
	toCancel = loop.RunEvery(cancelSelf, 1);

	loop.Loop();
	printf("main loop exits\n");
}
