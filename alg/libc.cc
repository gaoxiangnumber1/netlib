#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

bool g_invalid_input = false;
void ShowContent(const void *data, int length)
{
	const char *char_data = static_cast<const char*>(data);
	for(int index = 0; index < length; ++index)
	{
		printf("%c", char_data[index] ? char_data[index] : '*');
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t StringLength(const char *string)
{
	if(string == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}

	size_t length = 0;
	for(; *string != 0; ++string, ++length);
	return length;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	char *char_dest = static_cast<char*>(dest);
	const char *char_src = static_cast<const char*>(src);
	if(char_dest < char_src || char_dest >= char_src + length)
	{
		for(size_t index = 0; index < length; ++index)
		{
			char_dest[index] = char_src[index];
		}
	}
	else // char_src < char_dest < char_src + length
	{
		for(size_t index = 0; index < length; ++index)
		{
			char_dest[length - 1 - index] = char_src[length - 1 - index];
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *StringCopy(char *dest, const char *src)
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	const unsigned char *char_data1 = static_cast<const unsigned char*>(data1);
	const unsigned char *char_data2 = static_cast<const unsigned char*>(data2);
	size_t index = 0;
	for(; index < length && char_data1[index] == char_data2[index]; ++index);
	return (index == length ? 0 : char_data1[index] - char_data2[index]);
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	for(; *string1 != 0 && *string1 == *string2; ++string1, ++string2);
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void *MemorySet(void *data, int value, size_t length)
{
	if(data == nullptr)
	{
		g_invalid_input = true;
		return data;
	}

	char *char_data = static_cast<char*>(data);
	char char_value = static_cast<char>(value);
	for(size_t index = 0; index < length; ++index)
	{
		char_data[index] = char_value;
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StringToInt(const char *string)
{
	if(string == nullptr)
	{
		g_invalid_input = true;
		return 0;
	}

	size_t length = StringLength(string);
	if(length == 0)
	{
		return 0;
	}
	size_t index = 0;
	if(string[0] == '+' || string[0] == '-')
	{
		if(length == 1)
		{
			g_invalid_input = true;
			return 0;
		}
		++index;
	}
	int64_t int64_value = 0;
	const int kIntMin = 0x80000000, kIntMax = 0x7fffffff;
	while(index < length)
	{
		if('0' <= string[index] && string[index] <= '9')
		{
			int64_value = int64_value * 10 + string[index] - '0';
			if((string[0] == '-' && -1 * int64_value < kIntMin) ||
			        (string[0] != '-' && int64_value > kIntMax))
			{
				g_invalid_input = true;
				return 0;
			}
			++index;
		}
		else
		{
			g_invalid_input = true;
			return 0;
		}
	}
	int result = static_cast<int>(int64_value);
	if(string[0] == '-')
	{
		result *= -1;
	}
	return result;
}
void TestStringToInt()
{
	// Function: -2147483648, -999, -0, 0, +0, 999, +999, 2147483647, +2147483647
	// Edge: ""
	// Negative: nullptr, "+" "-", "999*9"(char not in '0123456789'),
	//					 -9999999999999, -2147483649, 2147483648, +2147483648, 9999999999999
	printf("----------TestStringToInt----------\n");
	const int kCaseNumber = 19;
	const char *string[kCaseNumber] =
	{
		"-2147483648", "-999", "-0", "0", "+0", "999", "+999", "2147483647", "+2147483647",
		"",
		nullptr, "-9999999999999","-2147483649","2147483648","+2147483648","9999999999999",
	};
	const int answer[kCaseNumber] =
	{
		-2147483648, -999, 0, 0, 0, 999, 999, 2147483647, 2147483647,
		0,
		0, 0, 0, 0, 0, 0,
	};
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	for(char ch = -128; ch != 127; ++ch)
	{
		printf("%d: (%c)\n", static_cast<int>(ch), ch);
	}
	printf("\n\n\n\n\n\n");
	for(unsigned char ch = 0; ch != 255; ++ch)
	{
		printf("%d: (%c)\n", static_cast<int>(ch), ch);
	}
	TestMemoryCopyOrMove();
	TestStringCopy();
	TestMemoryCompare();
	TestStringCompare();
	TestMemorySet();
	TestStringToInt();
}
