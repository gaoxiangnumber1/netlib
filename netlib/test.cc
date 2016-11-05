/*#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>

#include <stdio.h>

using netlib::Thread;
using netlib::EventLoop;
using netlib::EventLoopThread;

void RunInThread()
{
	printf("RunInThread(): pid = %d, tid = %d\n", getpid(), Thread::ThreadId());
}

int main()
{
	printf("main(): pid = %d, tid = %d\n", getpid(), Thread::ThreadId());

	EventLoopThread loop_thread;
	EventLoop* loop = loop_thread.StartLoop();
	loop->RunInLoop(RunInThread);
	loop->RunAfter(RunInThread, 2);
	sleep(3);
	loop->Quit();

	printf("exit main().\n");
}
*/
