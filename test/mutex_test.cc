#include <netlib/mutex.h>

#include <stdio.h>

#include <vector>

#include <netlib/time_stamp.h>
#include <netlib/thread.h>

using std::vector;
using netlib::TimeStamp;
using netlib::MutexLock;
using netlib::MutexLockGuard;
using netlib::Thread;

const int kMaxThread = 8;
const int kCount = 10 * 1000 * 1000;

MutexLock g_mutex;
vector<int> g_vector;

void PushWithoutLock()
{
	for(int index = 0; index < kCount; ++index)
	{
		g_vector.push_back(index);
	}
}

void PushWithLock()
{
	for(int index = 0; index < kCount; ++index)
	{
		MutexLockGuard lock(g_mutex);
		g_vector.push_back(index);
	}
}

int TestLocked()
{
	MutexLockGuard lock(g_mutex);
	if(g_mutex.IsLockedByThisThread() == false)
	{
		return -1;
	}
	return 0;
}

int main()
{
	printf("Test IsLockedByThisThread(): ");
	assert(TestLocked() == 0);
	printf("passed.\n");

	g_vector.reserve(kMaxThread * kCount);
	for(int thread_number = 1; thread_number <= kMaxThread; ++thread_number)
	{
		vector<Thread*> thread_vector;
		g_vector.clear();
		TimeStamp start = TimeStamp::Now();
		for(int index = 0; index < thread_number; ++index)
		{
			thread_vector.push_back(new Thread(&PushWithoutLock));
			thread_vector.back()->Start();
		}
		for(int index = 0; index < thread_number; ++index)
		{
			thread_vector[index]->Join();
		}
		printf("%d thread without lock %f\n",
		       thread_number, TimeDifferenceInSecond(TimeStamp::Now(), start));

		thread_vector.clear();
		g_vector.clear();
		for(int index = 0; index < thread_number; ++index)
		{
			thread_vector.push_back(new Thread(&PushWithLock));
			thread_vector.back()->Start();
		}
		for(int index = 0; index < thread_number; ++index)
		{
			thread_vector[index]->Join();
		}
		printf("%d thread with lock %f\n",
		       thread_number, TimeDifferenceInSecond(TimeStamp::Now(), start));
	}
}
/*
$ make
g++ -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -rdynamic -I../ -o mutex_test.o -c mutex_test.cc
g++ -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -rdynamic -I../ -o ../netlib/thread.o -c ../netlib/thread.cc
g++ -std=c++11 -Wall -Wconversion -Werror -Wextra -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -rdynamic -I../ -o ../netlib/time_stamp.o -c ../netlib/time_stamp.cc
g++ -o mutex_test mutex_test.o ../netlib/thread.o ../netlib/time_stamp.o -lpthread
$ ./mutex_test
Test IsLockedByThisThread():
passed.
1 thread without lock 0.273272
1 thread with lock 0.709470
2 thread without lock 0.518821
2 thread with lock 6.073407
3 thread without lock 0.700522
3 thread with lock 8.068397
4 thread without lock 0.911926
4 thread with lock 7.657856
5 thread without lock 1.203762
5 thread with lock 14.849985
6 thread without lock 1.469592
6 thread with lock 25.265914
7 thread without lock 1.526363
7 thread with lock 34.263861
8 thread without lock 1.618995
8 thread with lock 39.859729
*/
