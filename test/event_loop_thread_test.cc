#include <unistd.h> // getpid(), sleep()
#include <stdio.h> // printf()

#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>

using std::bind;
using netlib::EventLoop;
using netlib::EventLoopThread;
using netlib::Thread;

void Print(EventLoop *loop = nullptr)
{
	printf("pid = %d, tid = %d, loop = %p\n", getpid(), Thread::ThreadId(), loop);
}

void Quit(EventLoop *loop)
{
	Print(loop);
	loop->Quit();
}

int main()
{
	Print();

	{
		EventLoopThread thread1;  // Never start
	}

	{
		// Dtor calls Quit()
		EventLoopThread thread2;
		EventLoop *loop = thread2.StartLoop();
		loop->RunInLoop(bind(Print, loop));
		sleep(1);
	}

	{
		// Quit() before Dtor
		EventLoopThread thread3;
		EventLoop *loop = thread3.StartLoop();
		loop->RunInLoop(bind(Quit, loop));
		sleep(1);
	}
}
