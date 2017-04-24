#include <stdio.h>
#include <string.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Assume: data points to a valid array, first and last are valid indexes.
// Return: nonnegative if found, otherwise -1.
int BinarySearchIterate(int *data, int first, int last, int target) // Search target in [first, last)
{
	while(first < last)
	{
		int middle = first + ((last - first) >> 1);
		// [first, middle), [middle, middle + 1), [middle + 1, last)
		if(data[middle] > target) // Search left half: [first, middle)
		{
			last = middle;
		}
		else if(data[middle] == target) // Return result.
		{
			return middle;
		}
		else // Search right half: [middle + 1, last)
		{
			first = middle + 1;
		}
	}
	return -1;
}
int BinarySearchRecursive(int *data, int first, int last, int target)
{
	if(first >= last)
	{
		return -1;
	}
	int middle = first + ((last - first) >> 1);
	// [first, middle), [middle, middle + 1), [middle + 1, last)
	if(data[middle] > target)
	{
		return BinarySearchRecursive(data, first, middle, target);
	}
	else if(data[middle] == target)
	{
		return middle;
	}
	else
	{
		return BinarySearchRecursive(data, middle + 1, last, target);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int kNumberOfChar = 128;
// const int kMod = TODO;
int QuickPower(int base, int exponent)
{
	int power = 1;
	while(exponent > 0)
	{
		if((exponent & 0x1) == 1)
		{
			power *= base; // %
		}
		exponent >>= 1;
		base *= base;
	}
	return power;
}
int HashValue(const char *data, const int length) // O(length)
{
	int hash_value = 0;
	for(int index = 0; index < length; ++index)
	{
		hash_value = hash_value * kNumberOfChar + static_cast<int>(data[index]); // %
	}
	return hash_value;
}
void RKStringSearch(const char *long_string, const char *short_string)
{
	printf("----------RKStringSearch----------\n");
	int long_length = static_cast<int>(strlen(long_string));
	int short_length = static_cast<int>(strlen(short_string));
	// Pre-process
	int target_hash_value = HashValue(short_string, short_length);
	int substring_number = long_length - short_length + 1;
	int hash_value[substring_number];
	hash_value[0] = HashValue(long_string, short_length);
	int max_power = QuickPower(kNumberOfChar, short_length - 1);
	bool good_match = false;
	for(int index = 0; index < substring_number; ++index)
	{
		if(hash_value[index] == target_hash_value &&
		        strncmp(long_string + index, short_string, short_length) == 0)
		{
			printf("%d ", index);
			good_match = true;
		}
		if(index < substring_number - 1)
		{
			int to_subtract = static_cast<int>(long_string[index]) * max_power; // %
			int to_add = static_cast<int>(long_string[index + short_length]);
			hash_value[index + 1] =
			    (hash_value[index] - to_subtract) * kNumberOfChar + to_add; // %
		}
	}
	if(good_match == false)
	{
		printf("null");
	}
	printf("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComputePrefix(const char *string, int *prefix_length, int length)
{
	prefix_length[0] = 0; // string[0] has no prefix.
	int matched = 0; // Already matched prefix's length.
	for(int index = 1; index < length; ++index) // string[1, length - 1] has prefix.
	{
		// 1.	matched == 0: we should check string[0] next, since string[0] has no
		//		prefix, thus exit while loop.
		// 2.	matched != 0: we have matched string[0, matched - 1],
		//		check string[matched] next.
		// 3.	string[matched] != string[index]: decrease matched to current prefix
		//		string[0, matched - 1] 's matched prefix length.
		// 4.	string[matched] == string[index]: match success, increase matched by 1.
		while(matched > 0 && string[matched] != string[index])
		{
			matched = prefix_length[matched - 1];
		}
		if(string[matched] == string[index])
		{
			++matched;
		}
		prefix_length[index] = matched;
	}
}
void KMPStringSearch(const char *long_string, const char *short_string)
{
	printf("----------KMPStringSearch----------\n");
	//int long_length = static_cast<int>(strlen(long_string));
	int short_length = static_cast<int>(strlen(short_string));
	// Pre-process
	int prefix_length[short_length];
	ComputePrefix(short_string, prefix_length, short_length);
	for(int index = 0; index < short_length; ++index)
	{
		printf("%d ", prefix_length[index]);
	}
	/*
	int matched = 0;
	bool good_match = false;
	for(int index = 0; index < long_length; ++index)
	{
		while(matched > 0 && short_string[matched] != long_string[index])
		{
			matched = prefix_length[matched - 1];
		}
		if(short_string[matched] == long_string[index])
		{
			++matched;
		}
		if(matched == short_length)
		{
			good_match = true;
			printf("%d ", index - matched + 1);
			matched = prefix_length[matched - 1];
		}
	}
	if(good_match == false)
	{
		printf("null");
	}
	*/
	printf("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	const int kLongStringSize = 1024;
	const int kShortStringSize = 64;
	char long_string[kLongStringSize], short_string[kShortStringSize];
	while(scanf("%s %s", long_string, short_string) == 2)
	{
		//RKStringSearch(long_string, short_string);
		KMPStringSearch(long_string, short_string);
	}
}
/*
a ababaaa
a ababaca
0 0 1 2 3 1 1
0 0 1 2 3 0 1
a a
aaa a
aaa aa
abcdefac ac
abcdabcd bc
ababaca aba
a b
ab c
abc cd
----------------
0
0 1 2
0 1
6
1 5
0 2
null
null
null
*/
