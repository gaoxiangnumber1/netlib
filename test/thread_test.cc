#include <thread.h>

#include <stdio.h>
#include <unistd.h>

using std::bind;
using netlib::ThreadId;
using netlib::Thread;

void mysleep(int seconds)
{
	timespec t = { seconds, 0 };
	nanosleep(&t, NULL);
}

void threadFunc()
{
	printf("thread_id=%d\n", ThreadId());
}

void threadFunc2(int x)
{
	printf("thread_id=%d, x=%d\n", ThreadId(), x);
}

void threadFunc3()
{
	printf("thread_id=%d\n", ThreadId());
	mysleep(1);
}

class Foo
{
public:
	explicit Foo(double x): x_(x) {}

	void memberFunc()
	{
		printf("thread_id=%d, Foo::x_=%f\n", ThreadId(), x_);
	}

private:
	double x_;
};

int main()
{
	printf("pid=%d, thread_id=%d\n", ::getpid(), ThreadId());

	Thread t1(threadFunc);
	t1.Start();
	t1.Join();

	Thread t2(bind(threadFunc2, 42));
	t2.Start();
	t2.Join();

	Foo foo(87.53);
	Thread t3(bind(&Foo::memberFunc, &foo));
	t3.Start();
	t3.Join();

	{
		Thread t4(threadFunc3);
		t4.Start();
		// t4 may destruct earlier than thread creation.
	}
	mysleep(2);
	{
		Thread t5(threadFunc3);
		t5.Start();
		mysleep(2);
		// t6 destruct later than thread creation.
	}
	::sleep(2);
	printf("number of created threads %d\n", Thread::created_number());
}
