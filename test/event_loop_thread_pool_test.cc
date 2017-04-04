#include <unistd.h> // getpid(), sleep()
#include <stdio.h> // printf()

#include <netlib/event_loop.h>
#include <netlib/event_loop_thread_pool.h>

using std::bind;
using netlib::EventLoop;
using netlib::EventLoopThreadPool;
using netlib::Thread;

void Print(EventLoop *loop = nullptr)
{
	printf("pid = %d, tid = %d, loop = %p\n", getpid(), Thread::ThreadId(), loop);
}

void InitialTask(EventLoop *loop)
{
	printf("InitialTask(): pid = %d, tid = %d, loop = %p\n", getpid(), Thread::ThreadId(), loop);
}

int main()
{
	printf("main enter.\n");
	Print();

	EventLoop loop;
	loop.RunAfter(bind(&EventLoop::Quit, &loop), 5);

	{
		printf("Single thread %p:\n", &loop);
		EventLoopThreadPool model(&loop, 0, InitialTask);
		model.Start();
		assert(model.GetNextLoop() == &loop);
		assert(model.GetNextLoop() == &loop);
		assert(model.GetNextLoop() == &loop);
	}

	{
		printf("Another thread:\n");
		EventLoopThreadPool model(&loop, 1, InitialTask);
		model.Start();
		EventLoop *next_loop = model.GetNextLoop();
		next_loop->RunAfter(bind(Print, next_loop), 2);
		assert(next_loop != &loop);
		assert(next_loop == model.GetNextLoop());
		assert(next_loop == model.GetNextLoop());
		sleep(3);
	}

	{
		printf("Three threads:\n");
		EventLoopThreadPool model(&loop, 3, InitialTask);
		model.Start();
		EventLoop *next_loop = model.GetNextLoop();
		next_loop->RunInLoop(bind(Print, next_loop));
		assert(next_loop != &loop);
		assert(next_loop != model.GetNextLoop());
		assert(next_loop != model.GetNextLoop());
		assert(next_loop == model.GetNextLoop());
	}
	printf("All passed.\n");
	loop.Loop();
	printf("main exit.\n");
}
