#include <netlib/time_stamp.h>

#include <stdio.h>

#include <vector>

using std::vector;
using netlib::TimeStamp;

const int kTestNumber = 1000 * 1000;

void PassByConstReference(const TimeStamp &x)
{
	printf("%s\n", x.ToFormattedTimeString().c_str());
}

void PassByValue(TimeStamp x)
{
	printf("%s\n", x.ToFormattedTimeString().c_str());
}

void Benchmark()
{
	vector<TimeStamp> stamp;
	stamp.reserve(kTestNumber);
	for(int index = 0; index < kTestNumber; ++index)
	{
		stamp.push_back(TimeStamp::Now());
	}
	printf("%s\n", stamp.front().ToFormattedTimeString().c_str());
	printf("%s\n", stamp.back().ToFormattedTimeString().c_str());
	printf("%f\n", TimeDifferenceInSecond(stamp.back(), stamp.front()));

	int difference[20] = {0};
	int64_t start = stamp.front().microsecond_since_epoch();
	for(int index = 1; index < kTestNumber; ++index)
	{
		int64_t next = stamp[index].microsecond_since_epoch();
		int64_t diff = next - start;
		start = next;
		if(diff < 0)
		{
			printf("Reverse!\n");
		}
		else if(diff < 20)
		{
			++difference[diff];
		}
		else
		{
			printf("Big gap %d\n", static_cast<int>(diff));
		}
	}

	for (int index = 0; index < 20; ++index)
	{
		printf("%2d: %d\n", index, difference[index]);
	}
}

int main()
{
	TimeStamp now(TimeStamp::Now());
	printf("%s\n", now.ToFormattedTimeString().c_str());
	PassByValue(now);
	PassByConstReference(now);
	Benchmark();
}
