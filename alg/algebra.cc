#include <stdio.h>
#include <string.h>

const int kMax = 100000;
bool is_prime[kMax + 1];
int prime[kMax + 1];
int prime_count = -1;

void GetPrime()
{
	memset(is_prime, true, sizeof is_prime);
	for(int number = 2; number <= kMax; ++number)
	{
		if(is_prime[number] == true)
		{
			prime[++prime_count] = number;
		}
		for(int index = 0; index <= prime_count && (prime[index] * number <= kMax); ++index)
		{
			is_prime[prime[index] * number] = false;
			if(number % prime[index] == 0)
			{
				break;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool g_invalid_input = false;  // global variable to indicate whether the input is invalid
bool Equal(double num1, double num2)
{
	// since float number can't be represented exactly, if the difference is very small,
	// we think the two numbers are equal.
	if((num1 - num2 > -0.0000001) && (num1 - num2 < 0.0000001))
	{
		return true;
	}
	return false;
}
// recursive quick power
double RecursiveQuickPower(double base, int exponent)  // exponent must >= 0
{
	if(exponent == 0)
	{
		return 1;  // x^0 = 1
	}
	if(exponent == 1)
	{
		return base;  // x^1 = x
	}

	double result = RecursiveQuickPower(base, exponent >> 1);  // >> is more efficient than /
	result *= result;
	if((exponent & 0x1) == 1)  // if exponent is odd
	{
		result *= base;
	}
	return result;
}
double QuickPower(double base, int exponent)
{
	if(Equal(base, 0.0) && exponent < 0)  // invalid input: divided by 0
	{
		g_invalid_input = true;
		return 0.0;
	}

	int abs_exponent = exponent >= 0 ? exponent : -exponent;  // exponent's absolute value
	//double result = RecursiveQuickPower(base, abs_exponent);
	// RecursiveQuickPower is a kind of recursive quick power,
	// which is slower than the following loop quick power.
	double result = 1.0;
	while(abs_exponent != 0)
	{
		if((abs_exponent & 0x1) == 1)  // the lowest bit is 1
		{
			result *= base;
		}
		abs_exponent >>= 1;  // move to right 1 bit
		base *= base;
	}

	if(exponent < 0)
	{
		result = 1.0 / result;
	}
	return result;
}

int main()
{
	return 0;
}
