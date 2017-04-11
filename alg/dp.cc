#include <stdio.h>
#include <assert.h>
#include <algorithm>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MaxSubArraySum(int *data, int first, int last)
{
	// Max sum of sub-array that ends with [index], initial first.
	int current_max_sum = data[first];
	int max_sum = current_max_sum; // Max among all current_max_sum.
	for(int index = first + 1; index < last; ++index)
	{
		if(current_max_sum <= 0)
		{
			current_max_sum = data[index];
		}
		else
		{
			current_max_sum += data[index];
		}
		max_sum = max(max_sum, current_max_sum);
	}
	return max_sum;
}
void TestMaxSubArraySum()
{
	printf("----------TestMaxSubArraySum----------\n");
	const int kCaseNumber = 8, kMaxSize = 6;
	int data[kCaseNumber][kMaxSize] =
	{
		{1, 1, 1, 1, 1, 1},
		{-1, 0, 1, 1, 1, 1},
		{-1, 0, -1, 0, -1, 0},
		{-1, 0, 2, -1, 0, 0},
		{-5, -4, -1, -2, -3, -6},
		{-5, 0, 10, -2, -3, 9},
		{-2, 11, -4, 13, -5, -2},
		{5, -8, 3, 2, 5, 0},
	};
	int answer[kCaseNumber] = {6,4,0,2,-1,14,20,10};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		assert(answer[index] == MaxSubArraySum(data[index], 0, kMaxSize));
	}
	printf("All case pass.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LongestCommonSubsequence() {}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestMaxSubArraySum();
}
