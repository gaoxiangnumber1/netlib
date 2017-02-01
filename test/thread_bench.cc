#include <unistd.h> // sleep(), fork()
#include <stdlib.h> // exit()
#include <sys/types.h> // waitpid()
#include <sys/wait.h> // waitpid()
#include <stdio.h> // printf()

#include <map>
#include <functional>

#include <netlib/mutex.h>
#include <netlib/time_stamp.h>
#include <netlib/thread.h>

using std::map;
using std::bind;
using netlib::MutexLock;
using netlib::MutexLockGuard;
using netlib::TimeStamp;
using netlib::Thread;

const int kProcessNumber = 100 * 1000;
const int kThreadNumber = 100 * 1000;
const int kMicrosecondPerSecond = 1000 * 1000;

MutexLock g_mutex;
map<int, int> g_delay;

void Fun1() {}

void Fun2(TimeStamp start)
{
	TimeStamp now(TimeStamp::Now());
	int delay = static_cast<int>(TimeDifferenceInSecond(now, start) * kMicrosecondPerSecond);
	MutexLockGuard lock(g_mutex);
	++g_delay[delay];
}

void ForkBench()
{
	sleep(3);
	TimeStamp start(TimeStamp::Now());

	for(int index = 0; index < kProcessNumber; ++index)
	{
		pid_t child = fork();
		if(child == 0)
		{
			exit(0);
		}
		else
		{
			waitpid(child, NULL, 0);
		}
	}

	double used_time = TimeDifferenceInSecond(TimeStamp::Now(), start);
	printf("Average process creation time is %f microseconds.\n",
	       used_time * kMicrosecondPerSecond / kProcessNumber);
	printf("Total create %d processes.\n", kProcessNumber);
}

int main()
{
	printf("pid=%d, tid=%d\n", getpid(), Thread::ThreadId());

	TimeStamp start(TimeStamp::Now());
	for(int index = 0; index < kThreadNumber; ++index)
	{
		Thread thread1(Fun1);
		thread1.Start();
		thread1.Join();
	}
	double used_time = TimeDifferenceInSecond(TimeStamp::Now(), start);
	printf("Average thread creation time is %f microseconds\n",
	       used_time * kMicrosecondPerSecond / kThreadNumber);
	printf("Total create %d threads.\n", Thread::created_number());

	for(int index = 0; index < kThreadNumber; ++index)
	{
		TimeStamp now(TimeStamp::Now());
		Thread thread2(bind(Fun2, now));
		thread2.Start();
		thread2.Join();
	}
	{
		MutexLockGuard lock(g_mutex);
		for(map<int, int>::iterator it = g_delay.begin(); it != g_delay.end(); ++it)
		{
			printf("Delay = %d microseconds appears %d counts.\n", it->first, it->second);
		}
	}

	ForkBench();
}
/*
$ ./thread_bench
pid=15609, tid=15609
Average thread creation time is 9.521340 microseconds
Total create 100000 threads.
Delay = 4 microseconds appears 36 counts.
Delay = 5 microseconds appears 36867 counts.
Delay = 6 microseconds appears 54228 counts.
Delay = 7 microseconds appears 6824 counts.
Delay = 8 microseconds appears 928 counts.
Delay = 9 microseconds appears 448 counts.
Delay = 10 microseconds appears 207 counts.
Delay = 11 microseconds appears 110 counts.
Delay = 12 microseconds appears 76 counts.
Delay = 13 microseconds appears 34 counts.
Delay = 14 microseconds appears 35 counts.
Delay = 15 microseconds appears 49 counts.
Delay = 16 microseconds appears 26 counts.
Delay = 17 microseconds appears 15 counts.
Delay = 18 microseconds appears 8 counts.
Delay = 19 microseconds appears 5 counts.
Delay = 20 microseconds appears 21 counts.
Delay = 21 microseconds appears 29 counts.
Delay = 22 microseconds appears 7 counts.
Delay = 23 microseconds appears 9 counts.
Delay = 24 microseconds appears 2 counts.
Delay = 25 microseconds appears 2 counts.
Delay = 26 microseconds appears 10 counts.
Delay = 27 microseconds appears 2 counts.
Delay = 28 microseconds appears 6 counts.
Delay = 29 microseconds appears 2 counts.
Delay = 30 microseconds appears 2 counts.
Delay = 33 microseconds appears 2 counts.
Delay = 34 microseconds appears 3 counts.
Delay = 35 microseconds appears 1 counts.
Delay = 36 microseconds appears 2 counts.
Delay = 37 microseconds appears 3 counts.
Delay = 57 microseconds appears 1 counts.
Average process creation time is 138.160460 microseconds.
Total create 100000 processes.
*/
