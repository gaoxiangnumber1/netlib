#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

bool g_invalid_input = false;
void ShowContent(const void *data, size_t length)
{
	const char *data_char = static_cast<const char*>(data);
	for(size_t index = 0; index < length; ++index)
	{
		printf("%c", data_char[index] ? data_char[index] : '*');
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
size_t StringLength(const char *string)
{
	if(string == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}

	size_t length = 0;
	for(; string[length] != 0; ++length)
		;
	return length;
}
size_t StringLengthRecursive(const char *string)
{
	if(string == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}
	return *string ? StringLengthRecursive(string + 1) + 1 : 0;
}
void TestStringLengthRecursive()
{
	printf("----------TestStringLengthRecursive----------\n");
	assert(StringLengthRecursive(nullptr) == 0 && g_invalid_input == true);
	assert(StringLengthRecursive("") == 0);
	assert(StringLengthRecursive("a") == 1);
	assert(StringLengthRecursive("aaa") == 3);
	assert(StringLengthRecursive("aaaaa") == 5);
	assert(StringLengthRecursive("aaaaaaaaaa") == 10);
	printf("All Case Pass.\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void *MemoryCopyOrMove(void *dest, const void *src, size_t length)
{
	if(dest == nullptr || src == nullptr)
	{
		g_invalid_input = true;
		return dest;
	}

	if(dest == src)
	{
		return dest;
	}
	char *dest_char = static_cast<char*>(dest);
	const char *src_char = static_cast<const char*>(src);
	if(dest_char < src_char || dest_char >= src_char + length)
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_char[index] = src_char[index];
		}
	}
	else // src_char < dest_char < src_char + length
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_char[length - 1 - index] = src_char[length - 1 - index];
		}
	}
	return dest;
}
void TestMemoryCopyOrMove()
{
	printf("----------TestMemoryCopyOrMove----------\n");
	const int kCaseNumber = 7, kStringSize = 12;
	int offset[kCaseNumber] = { 0, 1, 2, 4, 5, 7, 8 };
	const char answer[kCaseNumber][kStringSize] = {
		// .1. [2src - length] .3. [4src] .5. [6src + length] .7.
		"abc.abc....", ".abcabc....", "..abcbc....", "....abc....", "....aabc...", "....abcabc.",
		"....abc.abc", };
	bool pass = true;
	for(int cnt = 0; cnt < kCaseNumber; ++cnt)
	{
		char string[kStringSize] = "....abc....";
		MemoryCopyOrMove(string + offset[cnt], string + 4, 3);
		if(strcmp(string, answer[cnt]) != 0)
		{
			printf("Case %d Not Pass. your = `%s` right = `%s`\n", cnt, string, answer[cnt]);
			pass = false;
		}
	}
	if(pass == true)
	{
		printf("All Case Pass.\n");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
char *StringCopy(char *dest, const char *src)
{
	// Check to avoid function call.
	if(dest == nullptr || src == nullptr)
	{
		g_invalid_input = true;
		return dest;
	}
	if(dest == src)
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
	int offset[kCaseNumber] = { 0, 1, 2, 5, 7, 9, 10 };
	const char answer[kCaseNumber][kStringSize] = {
		// .1. [2src - length] .3. [4src] .5. [6src + length] .7.
		"abc\0.abc\0", ".abc\0abc\0", "..abc\0bc\0", ".....abc\0", ".....ababc\0",
		".....abc\0abc\0", ".....abc\0\0abc\0" };
	const int answer_length[kCaseNumber] = { 9, 9, 9, 9, 11, 13, 14 };
	bool pass = true;
	for(int cnt = 0; cnt < kCaseNumber; ++cnt)
	{
		char string[kStringSize] = ".....abc\0\0\0\0\0\0";
		StringCopy(string + offset[cnt], string + 5);
		if(memcmp(string, answer[cnt], answer_length[cnt]) != 0)
		{
			printf("Case %d Not Pass. your = ", cnt);
			ShowContent(string, answer_length[cnt]);
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
//////////////////////////////////////////////////////////////////////////////////////////////////////
int MemoryCompare(const void *data1, const void *data2, size_t length)
{
	if(data1 == nullptr || data2 == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}

	if(data1 == data2)
	{
		return 0;
	}
	const unsigned char *data1_unsigned = static_cast<const unsigned char*>(data1);
	const unsigned char *data2_unsigned = static_cast<const unsigned char*>(data2);
	size_t index = 0;
	for(; index < length && data1_unsigned[index] == data2_unsigned[index]; ++index)
		;
	return index == length ? 0 : data1_unsigned[index] - data2_unsigned[index];
}
void TestMemoryCompare()
{
	printf("----------TestMemoryCompare----------\n");
	int i1 = 12345, i2 = 67890;
	double d1 = 123.45, d2 = 678.90, d3 = 123.45;
	assert(MemoryCompare(&i1, &i2, sizeof i1) * memcmp(&i1, &i2, sizeof i1) > 0);
	assert(MemoryCompare(&d1, &d2, sizeof d1) * memcmp(&d1, &d2, sizeof d1) > 0);
	assert(MemoryCompare(&d1, &d3, sizeof d1) == 0 && memcmp(&d1, &d3, sizeof d1) == 0);
	printf("All Case Pass.\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int StringCompare(const char *string1, const char *string2)
{
	if(string1 == nullptr || string2 == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}

	if(string1 == string2)
	{
		return 0;
	}
	for(; *string1 != 0 && *string1 == *string2; ++string1, ++string2)
		;
	return *string1 - *string2;
}
void TestStringCompare()
{
	printf("----------TestMemoryCompare----------\n");
	char string[3][8] = { "abcd", "abcde", "abcd" };
	assert(StringCompare(string[0], string[1]) * strcmp(string[0], string[1]) > 0);
	assert(StringCompare(string[1], string[0]) * strcmp(string[1], string[0]) > 0);
	assert(StringCompare(string[0], string[2]) == 0 && strcmp(string[0], string[2]) == 0);
	printf("All Case Pass.\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void *MemorySet(void *data, int value, size_t length)
{
	if(data == nullptr)
	{
		g_invalid_input = true;
		return data;
	}

	char *data_char = static_cast<char*>(data);
	const char value_char = static_cast<const char>(value);
	for(size_t index = 0; index < length; ++index)
	{
		data_char[index] = value_char;
	}
	return data;
}
void TestMemorySet()
{
	printf("----------TestMemorySet----------\n");
	unsigned a, b, c;
	MemorySet(&a, -1, sizeof a);
	MemorySet(&b, 0, sizeof b);
	MemorySet(&c, 1, sizeof c);
	assert(a == 0xffffffff && b == 0 && c == 0x01010101);
	printf("All Case Pass.\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define ERROR() g_invalid_input = true; return 0;
int StringToInt(const char *string)
{
	if(string == nullptr)
	{
		ERROR()
		;
	}
	if(*string == 0)
	{
		return 0;
	}
	// Process sign.
	int sign = 1;
	if(*string == '+' || *string == '-')
	{
		if(*(string + 1) == 0)
		{
			ERROR()
			;
		}
		sign = (*string == '-' ? -1 : sign);
		++string;
	}
	// Process number.
	int64_t num = 0;
	const int kIntMin = 0x80000000, kIntMax = 0x7fffffff;
	for(; *string != 0; ++string)
	{
		if(*string < '0' || *string > '9')
		{
			ERROR()
			;
		}
		num = num * 10 + *string - '0';
		if((sign == -1 && -1 * num < kIntMin) || (sign == 1 && num > kIntMax))
		{
			ERROR()
			;
		}
	}
	int result = static_cast<int>(num) * sign;
	return result;
}
void TestStringToInt()
{
	// Function: -2147483648, -999, -0, 0, +0, 999, +999, 2147483647
	// Edge: ""
	// Negative: nullptr, "+" "-", "9a", -2147483649, 2147483648
	printf("----------TestStringToInt----------\n");
	const int kCaseNumber = 15;
	const char *string[kCaseNumber] = { "-2147483648", "-999", "-0", "0", "+0", "999", "+999",
		"2147483647", "", nullptr, "+", "-", "9a", "-2147483649", "2147483648" };
	const int answer[kCaseNumber] = { -2147483648, -999, 0, 0, 0, 999, 999, 2147483647, 0, 0, 0, 0,
		0, 0, 0 };
	bool pass = true;
	for(int index = 0; index < kCaseNumber; ++index)
	{
		int your = StringToInt(string[index]);
		if(your != answer[index])
		{
			printf("Case %d Not Pass. your = %d, right = %d\n", index, your, answer[index]);
			pass = false;
		}
	}
	if(pass == true)
	{
		printf("All Case Pass.\n");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
const char *StringSearch(const char *haystack, const char *needle)
{
	if(haystack == nullptr || needle == nullptr)
	{
		g_invalid_input = true;
		return nullptr;
	}

	if(haystack == needle)
	{
		return haystack;
	}
	size_t haystack_length = StringLength(haystack);
	size_t needle_length = StringLength(needle);
	size_t diff = haystack_length - needle_length;
	for(size_t offset = 0; offset <= diff; ++offset)
	{
		const char *substring = haystack + offset;
		size_t index = 0;
		for(; index < needle_length && substring[index] == needle[index]; ++index)
			;
		if(index == needle_length)
		{
			return substring;
		}
	}
	return nullptr;
}
// TODO: use kmp to implement StringSearch.
void TestStringSearch()
{
	printf("----------TestStringSearch----------\n");
	const int kCaseNumber = 4;
	const char *haystack[kCaseNumber] = { nullptr, "a", "abcd", "abcdefg" };
	const char *needle[kCaseNumber] = { "a", nullptr, "abc", "hi" };
	const char *answer[kCaseNumber] = { nullptr, nullptr, haystack[2], nullptr };
	bool pass = true;
	for(int index = 0; index < kCaseNumber; ++index)
	{
		if(StringSearch(haystack[index], needle[index]) != answer[index])
		{
			printf("Case %d Not Pass.\n", index);
			pass = false;
		}
	}
	if(pass == true)
	{
		printf("All Case Pass.\n");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestStringLengthRecursive();
	TestMemoryCopyOrMove();
	TestStringCopy();
	TestMemoryCompare();
	TestStringCompare();
	TestMemorySet();
	TestStringToInt();
	TestStringSearch();
}
