// Function Test: +1234, -566, 5625
// Edge Test: none
// Negative Test:	NULL pointer, "", "+" "-", character that is not '0123456789+-'
//								over/underflow

#include <stdio.h>
#include <string.h>
#include <stdint.h>

bool g_valid = true;  // Global variable to indicate whether input is valid?
#define Error() g_valid = false; return 0;  // Just for efficiency

int StrToInt(const char *str)
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

int main()
{
	char str[1024];
	while(scanf("%s", str) == 1)
	{
		printf("%d\n", StrToInt(str));
	}

	return 0;
}
