#include <netlib/thread.h>

#include <unistd.h> // sleep()

#include <netlib/logging.h>

using std::bind;
using netlib::Thread;

void Fun1()
{
	printf("Thread 1-Fun1: thread_id=%d\n", Thread::ThreadId());
}

void Fun2(int num)
{
	printf("Thread 2-Fun2: thread_id=%d, num=%d\n", Thread::ThreadId(), num);
}

class Test
{
public:
	void Fun3(double dou)
	{
		printf("Thread 3-Fun3: thread_id=%d, dou=%f\n", Thread::ThreadId(), dou);
	}
};

int main()
{
	LOG_INFO("Enter main()");
	printf("main: pid=%d, thread_id=%d\n", getpid(), Thread::ThreadId());

	Thread thread1(Fun1);
	thread1.Start();
	thread1.Join();

	{
		Thread thread2(bind(Fun2, 7188));
		thread2.Start();
		//thread2.Join();
		// May destruct earlier than thread creation.
	}

	{
		Test test;
		Thread thread3(bind(&Test::Fun3, &test, 71.88));
		thread3.Start();
		sleep(1);
		// Destruct later than thread creation.
	}

	printf("Create %d threads\n", Thread::created_number());

	pid_t pid = fork();
	if(pid == 0)
	{
		printf("child process: pid=%d, thread_id=%d\n", getpid(), Thread::ThreadId());
	}
	LOG_INFO("Leave main()");
}
