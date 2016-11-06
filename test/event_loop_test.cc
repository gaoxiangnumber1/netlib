#include <event_loop.h>
#include <thread.h>
#include <logging.h>

using netlib::EventLoop;
using netlib::Thread;

EventLoop *g_loop = nullptr;

void Fun1()
{
	LOG_INFO("Fun: pid = %d, tid = %d", getpid(), Thread::ThreadId());
	EventLoop loop;
	loop.Loop();
}

void Fun2()
{
	g_loop->Loop();
}

int main()
{
	// Test1.
	/*
	LOG_INFO("main: pid = %d, tid = %d", getpid(), Thread::ThreadId());
	EventLoop loop; // Create in main thread.
	Thread thread(Fun1); // Create in child thread.
	thread.Start();
	loop.Loop();
	pthread_exit(NULL);
	*/
	// Test2
	/*
	LOG_INFO("main: pid = %d, tid = %d", getpid(), Thread::ThreadId());
	EventLoop loop; // Create in main thread.
	g_loop = &loop;
	Thread thread(Fun2); // Create in child thread.
	thread.Start();
	thread.Join();
	*/
	LOG_INFO("main: pid = %d, tid = %d", getpid(), Thread::ThreadId());
	EventLoop loop1;
	EventLoop loop2;
}
