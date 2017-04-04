#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowContent(const char *str, int length)
{
	for(int index = 0; index < length; ++index)
	{
		printf("%c", str[index] ? str[index] : '*');
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void *MemoryCopyOrMove(void *dest, const void *src, size_t length)
{
	if(dest == nullptr || src == nullptr || dest == src)
	{
		return dest;
	}

	char *dest_ptr = static_cast<char*>(dest);
	const char *src_ptr = static_cast<const char*>(src);
	if(dest_ptr < src_ptr || dest_ptr >= src_ptr + length)
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_ptr[index] = src_ptr[index];
		}
	}
	else // src_ptr < dest_ptr < src_ptr + length
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_ptr[length - 1 - index] = src_ptr[length - 1 - index];
		}
	}
	return dest;
}
void TestMemoryCopyOrMove()
{
	printf("----------TestMemoryCopyOrMove----------\n");
	const int kCaseNumber = 7, kStringSize = 12;
	int offset[kCaseNumber] = {0, 1, 2, 4, 5, 7, 8};
	const char answer[kCaseNumber][kStringSize] =
	{
		// .1. [2src - length] .3. [4src] .5. [6src + length] .7.
		"abc.abc....", ".abcabc....", "..abcbc....", "....abc....",
		"....aabc...", "....abcabc.", "....abc.abc",
	};
	bool pass = true;
	for(int cnt = 0; cnt < kCaseNumber; ++cnt)
	{
		char str[kStringSize] = "....abc....";
		MemoryCopyOrMove(str + offset[cnt], str + 4, 3);
		if(strcmp(str, answer[cnt]) != 0)
		{
			printf("Case %d Not Pass. your = `%s` right = `%s`\n", cnt, str, answer[cnt]);
			pass = false;
		}
	}
	if(pass == true)
	{
		printf("All Case Pass.\n");
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t StringLength(const char *str)
{
	if(str == nullptr)
	{
		return 0;
	}
	size_t length = 0;
	for(int index = 0; str[index] != '\0'; ++index, ++length);
	return length;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *StringCopy(char *dest, const char *src)
{
	if(dest == nullptr || src == nullptr || dest == src)
	{
		return dest;
	}
	MemoryCopyOrMove(dest, src, StringLength(src) + 1);
	return dest;
}
void TestStringCopy()
{
	printf("----------TestStringCopy----------\n");
	const int kCaseNumber = 7, kStringSize = 16;
	int offset[kCaseNumber] = {0, 1, 2, 5, 7, 9, 10};
	const char answer[kCaseNumber][kStringSize] =
	{
		// .1. [2src - length] .3. [4src] .5. [6src + length] .7.
		"abc\0.abc\0", ".abc\0abc\0", "..abc\0bc\0", ".....abc\0",
		".....ababc\0", ".....abc\0abc\0", ".....abc\0\0abc\0"
	};
	const int answer_length[kCaseNumber] = {9, 9, 9, 9, 11, 13, 14};
	bool pass = true;
	for(int cnt = 0; cnt < kCaseNumber; ++cnt)
	{
		char str[kStringSize] = ".....abc\0\0\0\0\0\0";
		StringCopy(str + offset[cnt], str + 5);
		if(memcmp(str, answer[cnt], answer_length[cnt]) != 0)
		{
			printf("Case %d Not Pass. your = ", cnt);
			ShowContent(str, answer_length[cnt]);
			printf(",right = ");
			ShowContent(answer[cnt], answer_length[cnt]);
			printf("\n");
			pass = false;
		}
	}
	if(pass == true)
	{
		printf("All Case Pass.\n");
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
// Function Test: +1234, -566, 5625
// Edge Test: none
// Negative Test:	NULL pointer, "", "+" "-", character that is not '0123456789+-'
//								over/underflow
bool g_valid = true;  // Global variable to indicate whether input is valid?
#define Error() g_valid = false; return 0;  // Just for efficiency
int StringToInt(const char *str)
{
	// Negative data: null pointer
	if(str == nullptr)
	{
		Error();
	}
	int length = static_cast<int>(strlen(str));
	// Negative data: ""
	if(length <= 0)
	{
		Error();
	}
	int index = 0;
	// If str begin with '+' or '-', then number should begin with [1],
	// otherwise is invalid input.
	if(str[0] == '+' || str[0] == '-')
	{
		// str = "+" or "-"
		if(length == 1)
		{
			Error();
		}
		++index;
	}
	int64_t result = 0;  // Deal with over/underflow condition
	while(index < length)
	{
		// Valid input:
		if('0' <= str[index] && str[index] <= '9')
		{
			result = result * 10 + str[index] - '0';
			// Negative data: over/underflow
			if(0x7fffffff < result || result < (signed int)0x80000000)
			{
				Error();
			}
			++index;
		}
		// Negative data: character that is not number / '+' / '-'
		else
		{
			Error();
		}
	}
	int return_result = static_cast<int>(result);
	if(str[0] == '-')
	{
		return_result *= -1;
	}
	return return_result;
}
*/
/*
//Compare two strings:
//1.	Empty string's size is 0. All empty strings are equal. All empty strings are smaller
//	than nonempty strings.
//2.	If two strings' common length part are same, then they are equal if they have the same
//	length, otherwise, the shorter string is smaller.
//3.	Other conditions: Judge according to the first different letter.
// Return: -2 on input error; -1 if string1 < string2;
// 0 if string1 = string2; 1 if string1 >string2
int StringCompare(string string1, string string2)
{
	int length1 = string1.size(), length2 = string2.size();

	// Input error:
	if(length1 < 0 || length2 < 0)
	{
		return -2;
	}

	// If there exist at least one empty string
	if(length1 == 0 || length2 == 0)
	{
		if(length1 == 0 && length2 == 0)  // Two empty strings are equal
		{
			return 0;
		}
		else if(length1 != 0)  //string2 is empty, so string1 > string2
		{
			return 1;
		}
		else  // string1 is empty, so string1 < string2
		{
			return -1;
		}
	}

	// Up to now, both strings are not empty.
	int common_length = (length1 < length2 ? length1 : length2);
	for(int index = 0; index < common_length; ++index)
	{
		if(string1[index] < string2[index])
		{
			return -1;
		}
		else if(string1[index] > string2[index])
		{
			return 1;
		}
	}

	// Common part are the same.
	if(length1 == length2)
	{
		return 0;
	}
	else if(length1 > length2)  // string1 is longer, so string1 > string2
	{
		return 1;
	}
	else  // string2 is longer, so string1 < string2
	{
		return -1;
	}
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestMemoryCopyOrMove();
	TestStringCopy();
}
