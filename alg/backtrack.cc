#include <stdio.h>
#include <utility>
using std::swap;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowContent(int *data, int first, int last)
{
	for(int index = first; index < last; ++index)
	{
		printf("%d ", data[index]);
	}
	printf("\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_permutation_number = 0;
void Permutation(int *data, int first, int last)  // [first, last)
{
	if(last - first == 1) // Length = 1, recursive ends, get one n-data permutation.
	{
		++g_permutation_number;
		ShowContent(data, 0, last);
		return;
	}
	for(int index = first; index < last; ++index)
	{
		if(index != first && data[first] == data[index]) // Repeated data, No need swap.
		{
			continue;
		}
		swap(data[first], data[index]); // Get permutation of the first position.
		Permutation(data, first + 1, last); // Get permutation of length n-1 data.
		swap(data[first], data[index]); // Backtrack.
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_subset_number = 0;
bool g_exist[128];
void GetSubset(int *data, bool *exist, int last)
{
	for(int index = 0; index < last; ++index)
	{
		if(exist[index] == true)
		{
			printf("%d ", data[index]);
		}
	}
	printf("\n");
}
void Subset(int *data, bool *exist, int first, int last) // [first, last)
{
	if(last - first == 1)
	{
		exist[first] = true;
		GetSubset(data, exist, last);
		exist[first] = false;
		GetSubset(data, exist, last);
		g_subset_number += 2;
		return;
	}
	exist[first] = true; // in subset
	Subset(data, exist, first + 1, last); // get subset of [first + 1, last)
	exist[first] = false;
	Subset(data, exist, first + 1, last);
}
void TestPermutationAndSubset()
{
	const int kCaseNumber = 4, kDataLength = 4;
	int data[kCaseNumber][kDataLength] = {{1,2,3,4}, {1,1,2,3}, {1,1,1,2},{1,1,1,1}};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		g_permutation_number = 0;
		Permutation(data[index], 0, kDataLength);
		printf("Case %d: total %d permutation.\n", index, g_permutation_number);
	}
	for(int index = 0; index < kCaseNumber; ++index)
	{
		g_subset_number = 0;
		Subset(data[index], g_exist, 0, kDataLength);
		printf("Case %d: total %d subset.\n", index, g_subset_number);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestPermutationAndSubset();
}

