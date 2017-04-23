#include <stdio.h>
#include <string.h>
#include <utility>
using std::swap;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EuclidGreatestCommonDivisor(int big, int small)
{
	if(big < small)
	{
		swap(big, small);
	}
	while(small != 0)
	{
		int mod = big % small;
		big = small;
		small = mod;
	}
	return big;
}
void TestEuclidGreatestCommonDivisor()
{
	printf("----------TestEuclidGreatestCommonDivisor----------\n");
	const int kCaseNumber = 5;
	int num[kCaseNumber][2] = {{100,9},{1,55},{11,1254},{55,66},{8,9}};
	int answer[kCaseNumber] = {1,1,11,11,1};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		if(EuclidGreatestCommonDivisor(num[index][0], num[index][1]) != answer[index])
		{
			printf("Case %d Not pass.\n", index);
		}
	}
	printf("All case pass\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int kMax = 255;
bool is_prime[kMax + 1];
int prime[kMax + 1];
int prime_count;
void Prime()
{
	memset(is_prime, true, sizeof is_prime);
	prime_count = 0;
	for(int number = 2; number <= kMax; ++number)
	{
		if(is_prime[number] == true)
		{
			prime[++prime_count] = number;
		}
		for(int index = 1;
		        index <= prime_count && prime[index] * number <= kMax;
		        ++index)
		{
			is_prime[prime[index] * number] = false;
			if(number % prime[index] == 0)
			{
				break;
			}
		}
	}
}
void TestPrime()
{
	printf("----------TestPrime----------\n");
	const int answer[] =
	{
		0,2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,
		101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,
		191,193,197,199,211,223,227,229,233,239,241,251
	};
	Prime();
	for(int index = 1; index <= prime_count; ++index)
	{
		if(prime[index] != answer[index])
		{
			printf("Case %d Not pass.\n", index);
		}
	}
	printf("All case Pass.\n");
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
	TestEuclidGreatestCommonDivisor();
	TestPrime();
}
