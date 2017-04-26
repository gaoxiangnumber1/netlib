#include <stdio.h>
#include <string.h>
#include <utility>
#include <math.h>
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
int prime[kMax];
int prime_count;
void Prime()
{
	bool is_prime[kMax + 1];
	memset(is_prime, true, sizeof is_prime);
	prime_count = 0;
	for(int num = 2; num <= kMax; ++num)
	{
		if(is_prime[num] == true)
		{
			prime[++prime_count] = num;
		}
		for(int index = 1;
		        index <= prime_count && prime[index] * num <= kMax;
		        ++index)
		{
			is_prime[prime[index] * num] = false;
			if(num % prime[index] == 0)
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
bool g_invalid_input = false;
bool Equal(double num1, double num2)
{
	double diff = num1 - num2;
	if(-0.0000001 < diff && diff < 0.0000001)
	{
		return true;
	}
	return false;
}
double QuickPower(double base, int exp)
{
	if(Equal(base, 0.0) == true && exp < 0) // Negative: divided by 0.
	{
		g_invalid_input = true;
		return 0.0;
	}

	int abs_exp = exp >= 0 ? exp : -exp;
	double result = 1.0;
	while(abs_exp > 0)
	{
		if((abs_exp & 0x1) == 1)
		{
			result *= base;
		}
		abs_exp >>= 1;
		base *= base;
	}

	if(exp < 0)
	{
		result = 1.0 / result;
	}
	return result;
}
void TestQuickPower()
{
	printf("----------TestQuickPower----------\n");
	const int kCaseNumber = 3;
	double base[kCaseNumber] = {-12.34, 12.34, 34.12};
	int exp[kCaseNumber] = {-5, 0, 5};
	for(int index1 = 0; index1 < kCaseNumber; ++index1)
	{
		for(int index2 = 0; index2 < kCaseNumber; ++index2)
		{
			if(Equal(pow(base[index1], exp[index2]),
			         QuickPower(base[index1], exp[index2])) == false)
			{
				printf("Case %f^%d Not Pass.\n", base[index1], exp[index2]);
			}
		}
	}
	printf("All case Pass.\n");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestEuclidGreatestCommonDivisor();
	TestPrime();
	TestQuickPower();
}
