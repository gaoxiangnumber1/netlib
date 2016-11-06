#include <thread.h>

#include <stdio.h>
#include <unistd.h> // sleep()

using std::bind;
using netlib::Thread;

void ThreadFunction()
{
	printf("ThreadFunction: thread_id=%d\n", Thread::ThreadId());
}

void ThreadFunction2(int x)
{
	printf("ThreadFunction2: thread_id=%d, x=%d\n", Thread::ThreadId(), x);
}

void ThreadFunction3()
{
	printf("ThreadFunction3: thread_id=%d\n", Thread::ThreadId());
	sleep(1);
}

class Test
{
public:
	explicit Test(double x): x_(x) {}

	void MemberFunction()
	{
		printf("MemberFunction: thread_id=%d, Test::x_=%f\n", Thread::ThreadId(), x_);
	}

private:
	double x_;
};

int main()
{
	printf("main: pid=%d, thread_id=%d\n", getpid(), Thread::ThreadId());

	Thread thread1(ThreadFunction);
	thread1.Start();
	thread1.Join();

	Thread thread2(bind(ThreadFunction2, 100));
	thread2.Start();
	thread2.Join();

	Test test(71.88);
	Thread thread3(bind(&Test::MemberFunction, &test));
	thread3.Start();
	thread3.Join();

	{
		Thread thread4(ThreadFunction3);
		thread4.Start(); // May destruct earlier than thread creation.
	}

	{
		Thread thread5(ThreadFunction3);
		thread5.Start();
		sleep(2);
		// Destruct later than thread creation.
	}

	printf("number of created threads %d\n", Thread::created_number());
}
